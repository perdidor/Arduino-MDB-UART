/*
 * USART.h
 *
 * Created: 11.05.2019 10:29:29
 *  Author: root
 */ 

#include "MDB.h"

#ifndef USART_H_
#define USART_H_

volatile uint8_t EXT_UART_BUFFER[32];
volatile uint8_t EXT_UART_BUFFER_COUNT;
volatile uint8_t EXTCMDCOMPLETE;

//1=9600; 1=19200; 3=38400
uint8_t EXT_UART_BAUDRATE_ID;
uint16_t EXT_UART_BAUDRATE;
uint16_t MYUBRR1;

int mdbchecksum;

void DebugMessage(unsigned char data[]);
void EXT_UART_Setup(void);
void USART_Setup(void);
void MDB_Setup(void);
void MDB_ACK(void);
int MDB_Receive(void);
void MDB_getByte(MDB_Byte* mdbb);
void EXT_UART_Transmit(unsigned char data[]);
unsigned char MDB_ChecksumValidate(void);
void MDB_read(void);
unsigned char MDB_readBlock(unsigned int Count);
void MDB_SendCHK(unsigned char CHKByte);
void MDB_SendByte(unsigned char Byte);
void MDB_Send(uint8_t data[], uint8_t len);
void EXT_CRLF(void);
void delay_1ms(uint16_t ms);

unsigned char MDB_SendReaderJustReset();
unsigned char MDB_SendReaderSetupData(unsigned char index);
unsigned char MDB_SendReaderDisplayRequest(unsigned char index);
unsigned char MDB_SendReaderBeginSession(unsigned char index);
unsigned char MDB_SendReaderCancelSessionRequest(unsigned char index);
unsigned char MDB_SendReaderVendApproved(unsigned char index);
unsigned char MDB_SendReaderVendDenied(unsigned char index);
unsigned char MDB_SendReaderEndSession(unsigned char index);
unsigned char MDB_SendReaderCancelled(unsigned char index);
unsigned char MDB_SendReaderPeriphIDData(unsigned char index);
unsigned char MDB_SendReaderError(unsigned char index);
unsigned char MDB_SendReaderCMDOutOfSequence(unsigned char index);
unsigned char MDB_SendReaderRevalueApproved(unsigned char index);
unsigned char MDB_SendReaderRevalueDenied(unsigned char index);
unsigned char MDB_SendReaderRevalueAmountLimit(unsigned char index);
unsigned char MDB_SendReaderDateTimeRequest(unsigned char index);
unsigned char MDB_SendReaderDataEntryRequest(unsigned char index);
unsigned char MDB_SendReaderDataEntryCancel(unsigned char index);

#endif /* USART_H_ */