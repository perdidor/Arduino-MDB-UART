/*
 * CFile1.c
 *
 * Created: 04.08.2020 19:31:10
 *  Author: root
 */ 
#ifndef F_CPU
#define F_CPU       16000000UL
#endif

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
#include "BillValidator.h"
#include "MDB.h"
#include "Settings.h"
#include "VMC.h"
#include "USART.h"

uint8_t IsEnabled = 0;

void HandleBillValidatorCommand(int command)
{
	switch (command)
	{
		case 0x30:
		BillValidator_Reset();
		break;
		case 0x31:
		BillValidator_SetupData();
		break;
		case 0x32:
		BillValidator_Security();
		break;
		case 0x33:
		BillValidator_Poll();
		break;
		case 0x34:
		BillValidator_BillType();
		break;
		case 0x35:
		BillValidator_Escrow();
		break;
		case 0x36:
		BillValidator_Stacker();
		break;
		case 0x37:
		BillValidator_IDData();
		break;
		default:
		break;
	}
	//char buff[32];
	//sprintf(&buff, "SLAVE#%02x\r\n", command);
	//EXT_UART_Transmit(buff);
}

void BillValidator_Escrow()
{
	if (MDB_readBlock(2))
	{
		MDB_ACK();
		if (((BillValidatorInsertedByte >> 4) & 0x07) == 1)
		{
			if (MDB_BUFFER[1].data == 0x01)
			{
				BillValidatorInsertedByte &= 0x8f;
				BillValidatorStackerCount = (BillValidatorStackerCount + 1) % 0xffff;
				EXT_UART_Transmit((char *)"SLAVE#BILLSTACKED\r\n");
			}
			if (MDB_BUFFER[1].data == 0x00)
			{
				BillValidatorInsertedByte &= 0x8f;
				BillValidatorInsertedByte |= (1 << 5);
				EXT_UART_Transmit((char *)"SLAVE#BILLRETURNED\r\n");
			}
			BillValidatorDevice.Status = 2;
		} else
		{
			EXT_UART_Transmit((char *)"SLAVE#BILLNOTPRESENT\r\n");
			BillValidatorDevice.Status = 3;
		}
	}
}

void BillValidator_BillType()
{
	if (MDB_readBlock(5))
	{
		MDB_ACK();
		BillValidatorVMCEnabledBillTypes = (MDB_BUFFER[1].data << 8) | MDB_BUFFER[2].data;
		BillValidatorVMCEscrowBillTypes = (MDB_BUFFER[3].data << 8) | MDB_BUFFER[4].data;
		IsEnabled = (BillValidatorVMCEnabledBillTypes > 0 || BillValidatorVMCEscrowBillTypes > 0);
		char buff[64];
		sprintf(buff, "SLAVE#VMCBT#%d#%d#%d#%d\r\n", MDB_BUFFER[1].data, MDB_BUFFER[2].data, MDB_BUFFER[3].data, MDB_BUFFER[4].data);
		EXT_UART_Transmit(buff);
	}
}

void BillValidator_Security()
{
	MDB_Receive();
	MDB_Receive();
	MDB_ACK();
}

void BillValidator_IDData()
{
	MDB_Receive();
	int checksum = 0;
	for (uint8_t i = 0; i < 29; i++)
	{
		checksum += BillValidatorSlaveIDData[i];
		MDB_SendByte(BillValidatorSlaveIDData[i]);
	}
	uint8_t checksumbyte = checksum & 0xff;
	MDB_SendCHK(checksumbyte);
	checksum = 0;
	int VMCResponse = MDB_Receive();
}

void BillValidator_Stacker()
{
	if(!MDB_readBlock(1))
	{
		MDB_BUFFER_COUNT = 0;
		DebugMessage("Error: invalid checksum for [Stacker]\r\n");
		return;
	}
	uint8_t b0 = (BillValidatorStackerCount >> 8) & 0xff;
	uint8_t b1 = BillValidatorStackerCount & 0xff;
	uint8_t b2 = (b0 + b1) & 0xff;
	MDB_SendByte(0);
	MDB_SendByte(b1);
	MDB_SendCHK(b2);
	int VMCResponse = MDB_Receive();
}

void BillValidator_SetupData()
{
	if(!MDB_readBlock(1))
	{
		MDB_BUFFER_COUNT = 0;
		DebugMessage("Error: invalid checksum for [Setup]\r\n");
		return;
	}
	int checksum = 0;
	for (uint8_t i = 0; i < 27; i++)
	{
		checksum += BillValidatorSlaveSetupData[i];
		MDB_SendByte(BillValidatorSlaveSetupData[i]);
	}
	uint8_t checksumbyte = checksum & 0xff;
	MDB_SendCHK(checksumbyte);
	int VMCResponse = MDB_Receive();		
	//unsigned char * buff[6];
	//for (int a = 0; a < 27 - 1; a++){
	//sprintf(&buff, "%02x ", BillValidatorSlaveSetupData[a]);
	//EXT_UART_Transmit(buff);
	//}
	//sprintf(&buff, "%02x\r\n", BillValidatorSlaveSetupData[26]);
	//EXT_UART_Transmit(buff);
}

void BillValidator_Poll()
{
	if(!MDB_readBlock(1))
	{
		MDB_BUFFER_COUNT = 0;
		DebugMessage("Error: invalid checksum for [Poll]\r\n");
		return;
	}
	int VMCResponse = 0;
	switch (BillValidatorDevice.Status)
	{
		case 0://JUST RESET
		MDB_SendByte(0x06);
		MDB_SendCHK(0x06);
		VMCResponse = MDB_Receive();
		BillValidatorStackerCount = 0;
		BillValidatorDevice.Status = 1;
		break;
		case 1://NO EVENTS
		if (IsEnabled == 1)
		{
			MDB_ACK();
		} else
		{
			MDB_SendByte(0x09);
			MDB_SendCHK(0x09);
			VMCResponse = MDB_Receive();
		}
		break;
		case 2://BILL INSERTED / ESCROW / RETURN
		MDB_SendByte(BillValidatorInsertedByte);
		MDB_SendCHK(BillValidatorInsertedByte);
		VMCResponse = MDB_Receive();
		BillValidatorDevice.Status = 1;
		break;
		case 3://INVALID ESCROW Response
		MDB_SendByte(0x0a);
		MDB_SendCHK(0x0a);
		VMCResponse = MDB_Receive();
		BillValidatorDevice.Status = 1;
		break;
		default:
		MDB_ACK();
		break;
	}
}

void BillValidator_Reset()
{
	if(!MDB_readBlock(1))
	{
		MDB_BUFFER_COUNT = 0;
		DebugMessage("Error: invalid checksum for [Reset]\r\n");
		return;
	}
	MDB_ACK();
	BillValidatorDevice.Status = 0;
}