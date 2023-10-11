/*
 * USART.c
 *
 * Created: 11.05.2019 10:30:00
 *  Author: root
 */ 
#ifndef F_CPU
#define F_CPU       16000000UL
#endif
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include "MDB.h"
#include "Cashless.h"
#include "VMC.h"
#include "USART.h"

void delay_1ms(uint16_t ms) {
	volatile uint16_t i,foo = 0;
	for(i=0;i<ms;i++)
	{
		_delay_ms(1);
		//make some "work" to avoid optimization
		foo++;
	}
	foo = 0;
}

void DebugMessage(unsigned char data[])
{
	#if DEBUG == 1
	EXT_UART_Transmit(data);
	#endif
}

void MDB_ACK() {
	while ( !( UCSR0A & (1<<UDRE0)));
	UCSR0B |= (1<<TXB80);
	UDR0 = 0x00;// send ACK to VMC
}

void MDB_Setup() {
	// Set baud rate with setbaud.h
	UBRR0H = (MYUBRR>>8);
	UBRR0L = MYUBRR;
	// Disable USART rate doubler
	UCSR0A &= ~(1<<U2X0);
	//UCSR0A |= (1<<MPCM0);
	// Set 9600-9-N-1 UART mode
	UCSR0C = (0<<UMSEL01)|(0<<UMSEL00)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B |= (1<<UCSZ02)|(1<<RXEN0)|(1<<TXEN0);//|(1<<RXCIE0); // 9bit, enable Rx interrupt
}

int MDB_Receive() {
	unsigned char resh, resl;
	int rtr = 0;
	// Wait for data to be received
	while ((!(UCSR0A & (1<<RXC0))) && rtr <20) {
		_delay_ms(1);
		rtr++;
	}
	if (rtr == 20){
		MDBReceiveErrorFlag = 1;
		MDBReceiveComplete = 1;
	}
	// Get 9th bit, then data from buffer
	resh = UCSR0B;
	resl = UDR0;
	// Filter the 9th bit, then return only data w\o mode bit
	resh = (resh >> 1) & 0x01;
	return ((resh << 8) | resl);
}

void MDB_getByte(MDB_Byte* mdbb) {
	int b;
	b = 0;
	b = MDB_Receive();
	memcpy (mdbb, &b, 2);
}

unsigned char MDB_ChecksumValidate() {
	uint32_t sum = 0;
	for (int i=0; i < (MDB_BUFFER_COUNT-1); i++)
	sum += MDB_BUFFER[i].data;
	if (MDB_BUFFER[MDB_BUFFER_COUNT-1].data == (sum & 0xFF))
	return 1;
	else
	return 0;
}

void MDB_read() {
	MDB_getByte(&MDB_BUFFER[MDB_BUFFER_COUNT++]);
	if (MDB_BUFFER_COUNT == 37){
		MDBReceiveComplete = 1;
		MDBReceiveErrorFlag = 1;
	}
	if (MDB_BUFFER_COUNT > 1 && MDB_ChecksumValidate()){
		MDBReceiveComplete = 1;
	}
}

unsigned char MDB_readBlock(unsigned int Count)
{
	for (int i = 0; i < Count; i++)
	{
		MDB_read();
	}
	return (MDBReceiveComplete & !MDBReceiveErrorFlag);
}

void EXT_UART_Transmit(unsigned char data[])
{
	for (int i = 0; i < strlen(data); i++){
		/* Wait for empty transmit buffer */
		while (( UCSR1A & (1<<UDRE1))  == 0){};
		if ((data[i] >= 30 && data[i] != 127) || (data[i] == 0x0d || data[i] == 0x0a)) UDR1 = data[i];
	}
}

void EXT_CRLF()
{
	EXT_UART_Transmit("\r\n");
}

void EXT_UART_Setup()
{
	MYUBRR1 = F_CPU/16/57600-1;
	UBRR1H = (MYUBRR1>>8);
	UBRR1L = MYUBRR1;
	/* Set frame format: 8data, 2stop bit */
	UCSR1C |= (1<< UCSZ10)|(1<< UCSZ11);
	UCSR1B = (1 << TXEN1)|(1 << RXEN1)|(1<<RXCIE1);
	while (( UCSR1A & (1<<UDRE1))  == 0){};
	sei();
}

ISR(USART1_RX_vect)
{
	unsigned char tmp = UDR1;
	if (EXTCMDCOMPLETE == 0)
	{
		if (tmp == '+')
		{
			EXTCMDCOMPLETE = 1;
		} else
		{
			EXT_UART_BUFFER[EXT_UART_BUFFER_COUNT++] = tmp;
		}
	}
}

void MDB_Send(uint8_t data[], uint8_t len) {
	while ( !( UCSR0A & (1<<UDRE0))) {};
	for (int i = 0; i < len - 1; i++)
	{
		while ( !( UCSR0A & (1<<UDRE0))) {};
		UCSR0B &= ~(1<<TXB80);
		UDR0 = data[i];
	}
	UCSR0B |= (1<<TXB80);
	UDR0 = data[len - 1];
}

void MDB_SendByte(unsigned char Byte)
{
	while (!( UCSR0A & (1<<UDRE0)));
	UCSR0B &= ~(1<<TXB80);
	UDR0 = Byte;
}

void MDB_SendCHK(unsigned char CHKByte)
{
	while (!( UCSR0A & (1<<UDRE0)));
	UCSR0B |= (1<<TXB80);
	UDR0 = CHKByte;
}

unsigned char MDB_SendReaderJustReset()
{
	MDB_SendByte(0x00);
	MDB_SendCHK(0x00);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderSetupData(unsigned char index)
{
	// calculate checksum for configuration
	uint8_t MiscOptsByte = 0;
	MiscOptsByte |= (ReaderSetupData[index].Misc_Options_RefundAvailable << 0) | (ReaderSetupData[index].Misc_Options_MultivendSupport << 1) |
	 (ReaderSetupData[index].Misc_Options_DisplayPresent << 2) | (ReaderSetupData[index].Misc_Options_VendCashSaleSupport << 2);
	uint8_t checksum = ((ReaderSetupData[index].CFG_Constant +
	ReaderSetupData[index].Feature_Level +
	(ReaderSetupData[index].Country_Code >> 8) +
	(ReaderSetupData[index].Country_Code & 0xFF) +
	ReaderSetupData[index].Scale_Factor +
	ReaderSetupData[index].Decimal_Places +
	ReaderSetupData[index].MaxResp_Time +
	MiscOptsByte) & 0xFF);
	MDB_SendByte(ReaderSetupData[index].CFG_Constant);
	MDB_SendByte(ReaderSetupData[index].Feature_Level);
	MDB_SendByte(ReaderSetupData[index].Country_Code >> 8);
	MDB_SendByte(ReaderSetupData[index].Country_Code & 0xFF);
	MDB_SendByte(ReaderSetupData[index].Scale_Factor);
	MDB_SendByte(ReaderSetupData[index].Decimal_Places);
	MDB_SendByte(ReaderSetupData[index].MaxResp_Time);
	MDB_SendByte(MiscOptsByte);
	MDB_SendCHK(checksum);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderBeginSession(unsigned char index)
{
	MDB_SendByte(0x04);
	MDB_SendCHK(0x04);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderCancelSessionRequest(unsigned char index)
{
	// calculate checksum for configuration
	uint8_t checksum = ((0x03 +
	(ReaderSessionData[index].StartData.Funds >> 8) +
	(ReaderSessionData[index].StartData.Funds & 0xff) +
	(ReaderSessionData[index].StartData.MediaID >> 24) +
	(ReaderSessionData[index].StartData.MediaID >> 16) +
	(ReaderSessionData[index].StartData.MediaID >> 8) +
	(ReaderSessionData[index].StartData.MediaID & 0xff) +
	ReaderSessionData[index].StartData.PaymentType +
	(ReaderSessionData[index].StartData.PaymentData >> 8) +
	(ReaderSessionData[index].StartData.PaymentData & 0xff)) & 0xFF);
	MDB_SendByte(0x03);
	MDB_SendByte(ReaderSessionData[index].StartData.Funds >> 8);
	MDB_SendByte(ReaderSessionData[index].StartData.Funds & 0xff);
	MDB_SendByte(ReaderSessionData[index].StartData.MediaID >> 24);
	MDB_SendByte(ReaderSessionData[index].StartData.MediaID >> 16);
	MDB_SendByte(ReaderSessionData[index].StartData.MediaID >> 8);
	MDB_SendByte(ReaderSessionData[index].StartData.MediaID & 0xff);
	MDB_SendByte(ReaderSessionData[index].StartData.PaymentType);
	MDB_SendByte(ReaderSessionData[index].StartData.PaymentData >> 8);
	MDB_SendByte(ReaderSessionData[index].StartData.PaymentData & 0xff);
	MDB_SendCHK(checksum);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderVendApproved(unsigned char index)
{
	// calculate checksum for configuration
	uint8_t checksum = ((0x05 +
	(ReaderSessionData[index].ResultData.Amount >> 8) +
	(ReaderSessionData[index].ResultData.Amount & 0xff)) & 0xFF);
	MDB_SendByte(0x05);
	MDB_SendByte(ReaderSessionData[index].ResultData.Amount >> 8);
	MDB_SendByte(ReaderSessionData[index].ResultData.Amount & 0xff);
	MDB_SendCHK(checksum);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderVendDenied(unsigned char index)
{
	MDB_SendByte(0x06);
	MDB_SendCHK(0x06);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderEndSession(unsigned char index)
{
	MDB_SendByte(0x07);
	MDB_SendCHK(0x07);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderCancelled(unsigned char index)
{
	MDB_SendByte(0x08);
	MDB_SendCHK(0x08);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderPeriphIDData(unsigned char index)
{
	// calculate checksum for configuration
	uint16_t checksum = ReaderPeriphID_Data[index].ID_Constant +
	ReaderPeriphID_Data[index].ManufacturerCode[0] +
	ReaderPeriphID_Data[index].ManufacturerCode[1] +
	ReaderPeriphID_Data[index].ManufacturerCode[2] +
	ReaderPeriphID_Data[index].SerialNumber[0] +
	ReaderPeriphID_Data[index].SerialNumber[1] +
	ReaderPeriphID_Data[index].SerialNumber[2] +
	ReaderPeriphID_Data[index].SerialNumber[3] +
	ReaderPeriphID_Data[index].SerialNumber[4] +
	ReaderPeriphID_Data[index].SerialNumber[5] +
	ReaderPeriphID_Data[index].SerialNumber[6] +
	ReaderPeriphID_Data[index].SerialNumber[7] +
	ReaderPeriphID_Data[index].SerialNumber[8] +
	ReaderPeriphID_Data[index].SerialNumber[9] +
	ReaderPeriphID_Data[index].SerialNumber[10] +
	ReaderPeriphID_Data[index].SerialNumber[11] +
	ReaderPeriphID_Data[index].ModelRevision[0] +
	ReaderPeriphID_Data[index].ModelRevision[1] +
	ReaderPeriphID_Data[index].ModelRevision[2] +
	ReaderPeriphID_Data[index].ModelRevision[3] +
	ReaderPeriphID_Data[index].ModelRevision[4] +
	ReaderPeriphID_Data[index].ModelRevision[5] +
	ReaderPeriphID_Data[index].ModelRevision[6] +
	ReaderPeriphID_Data[index].ModelRevision[7] +
	ReaderPeriphID_Data[index].ModelRevision[8] +
	ReaderPeriphID_Data[index].ModelRevision[9] +
	ReaderPeriphID_Data[index].ModelRevision[10] +
	ReaderPeriphID_Data[index].ModelRevision[11] +
	((ReaderPeriphID_Data[index].SoftwareVersion >> 8) & 0xff) +
	(ReaderPeriphID_Data[index].SoftwareVersion & 0xff);
	MDB_SendByte(0x09);
	MDB_SendByte(ReaderPeriphID_Data[index].ManufacturerCode[0]);
	MDB_SendByte(ReaderPeriphID_Data[index].ManufacturerCode[1]);
	MDB_SendByte(ReaderPeriphID_Data[index].ManufacturerCode[2]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[0]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[1]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[2]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[3]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[4]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[5]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[6]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[7]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[8]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[9]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[10]);
	MDB_SendByte(ReaderPeriphID_Data[index].SerialNumber[11]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[0]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[1]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[2]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[3]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[4]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[5]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[6]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[7]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[8]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[9]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[10]);
	MDB_SendByte(ReaderPeriphID_Data[index].ModelRevision[11]);
	MDB_SendByte((ReaderPeriphID_Data[index].SoftwareVersion >> 8) & 0xff);
	MDB_SendByte(ReaderPeriphID_Data[index].SoftwareVersion & 0xff);
	if (ReaderVMCSetupData[index].feature_level == 3)
	{
		checksum += (ReaderPeriphID_Data[index].OptionalFeatureBits >> 24) +
		(ReaderPeriphID_Data[index].OptionalFeatureBits >> 16) +
		(ReaderPeriphID_Data[index].OptionalFeatureBits >> 8) +
		(ReaderPeriphID_Data[index].OptionalFeatureBits & 0xff);
		MDB_SendByte((ReaderPeriphID_Data[index].OptionalFeatureBits >> 24) & 0xff);
		MDB_SendByte((ReaderPeriphID_Data[index].OptionalFeatureBits >> 16) & 0xff);
		MDB_SendByte((ReaderPeriphID_Data[index].OptionalFeatureBits >> 8) & 0xff);
		MDB_SendByte(ReaderPeriphID_Data[index].OptionalFeatureBits & 0xff);
	}
	checksum &= 0xff;
	MDB_SendCHK(checksum);
	//MDB_SendByte(0x09);
	//for (int i = 0; i < 29; i++)
	//{
		//MDB_SendByte(0x33);
	//}
	//MDB_SendCHK(0xd0);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderError(unsigned char index)
{
	MDB_SendByte(0x0a);
	MDB_SendCHK(0x0a);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderCMDOutOfSequence(unsigned char index)
{
	MDB_SendByte(0x0b);
	MDB_SendCHK(0x0b);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderDisplayRequest(unsigned char index)
{
	uint8_t datalen = strlen(&ReaderDisplayRequestData[index].DisplayData);
	uint16_t checksum = 0x02 + ReaderDisplayRequestData[index].DisplayTime;
	MDB_SendByte(0x02);
	MDB_SendByte(ReaderDisplayRequestData[index].DisplayTime);
	for (int i = 0; i < datalen; i++)
	{
		MDB_SendByte(ReaderDisplayRequestData[index].DisplayData[i]);
		checksum += ReaderDisplayRequestData[index].DisplayData[i];
	}
	MDB_SendCHK(checksum & 0xff);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderRevalueApproved(unsigned char index)
{
	MDB_SendByte(0x0d);
	MDB_SendCHK(0x0d);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderRevalueDenied(unsigned char index)
{
	MDB_SendByte(0x0e);
	MDB_SendCHK(0x0e);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderRevalueAmountLimit(unsigned char index)
{
	MDB_SendByte(0x0f);
	MDB_SendByte(ReaderRevalueAmountLimit[index] >> 8);
	MDB_SendByte(ReaderRevalueAmountLimit[index] & 0xff);
	MDB_SendCHK((0x0f + (ReaderRevalueAmountLimit[index] >> 8) + (ReaderRevalueAmountLimit[index] & 0xff)) & 0xff);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderDateTimeRequest(unsigned char index)
{
	MDB_SendByte(0x11);
	MDB_SendCHK(0x11);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderDataEntryRequest(unsigned char index)
{
	unsigned char DataEntryByte = ((ReaderDataEntryRequest[index].Repeated << 7) | (ReaderDataEntryRequest[index].EntryLength & 0x7f)) & 0xff;
	if (ReaderSetupData[index].Misc_Options_DisplayPresent)
	{
		MDB_SendByte(0x12);
		MDB_SendByte(DataEntryByte);
		MDB_SendCHK((0x12 + DataEntryByte) & 0xff);
	} else
	{
		uint8_t datalen = strlen(&ReaderDisplayRequestData[index].DisplayData);
		MDB_SendByte(0x12);
		MDB_SendByte(DataEntryByte);
		MDB_SendByte(0x02);
		uint16_t checksum = 0x12 + DataEntryByte + 0x02;
		MDB_SendByte(ReaderDisplayRequestData[index].DisplayTime);
		for (int i = 0; i < datalen; i++)
		{
			MDB_SendByte(ReaderDisplayRequestData[index].DisplayData[i]);
			checksum += ReaderDisplayRequestData[index].DisplayData[i];
		}
		MDB_SendCHK(checksum & 0xff);
	}
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

unsigned char MDB_SendReaderDataEntryCancel(unsigned char index)
{
	MDB_SendByte(0x13);
	MDB_SendCHK(0x13);
	int VMCResponse = MDB_Receive();
	return (!VMCResponse && !MDBReceiveErrorFlag);
}

void USART_Setup()
{
	//ReadExtUARTBaudRateID();
	MDB_Setup();
	EXT_UART_Setup();
}