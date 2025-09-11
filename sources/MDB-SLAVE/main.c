/*
 * MDB-SLAVE-CASHLESS.c
 *
 * Created: 02.05.2019 14:28:46
 * Author : root
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
#include "MDB.h"
#include "USART.h"
#include "Cashless.h"
#include "USD.h"
#include "VMC.h"
#include "Settings.h"
#include "LEDControl.h"
#include "BillValidator.h"

char * Version = "SYS#MDBSTART#1.0.1727\t\n";

void InitAll()
{
	//SetupPins();
	USART_Setup();
	//ReadMDBDeviceState();
	//ReadCashlessSetupData();
	//ReadCashlessExpIDData();
	//ReadCashlessRevalueAmountLimits();
	//ResetReaderVMCData(0);
	//ResetReaderVMCData(1);
	EXT_UART_Transmit(Version);
	//EXT_CRLF();
	delay_1ms(1000);
	
}

void Int32ToBCD(uint32_t InputInt, uint8_t *BCDArray[4])
{
	char * str = "";
	itoa(InputInt, str, 10);
	uint8_t strLen = strlen(str) - 1;
	if (strLen > 8) return;
	BCDArray[0] = BCDArray[1] = BCDArray[2] = BCDArray[3] = 0x00;
	uint8_t counter = 1;
	while (InputInt > 0)
	{
		uint32_t foo = InputInt % 100;
		BCDArray[4 - counter] = (((foo/10) << 4) | (foo % 10));
		InputInt -= foo;
		counter++;
	}
}

uint16_t BCDByteToInt(uint8_t BCDBytes[])
{
	int res = 0;
	for (int i = 0; i < sizeof(BCDBytes)/sizeof(uint8_t); i++)
	{
		res *= 100;
		res += (10 * (BCDBytes[i] >> 4));
		res += (BCDBytes[i] & 0xf);
	}
	return res;
}

int main(void)
{
	InitAll();
	//EXT_UART_Setup();
	//Reader_Poll_Reply[0] = MDB_REPLY_JUST_RESET;
	//Reader_Poll_Reply[1] = MDB_REPLY_JUST_RESET;
    while (1) 
    {
		MDBCommandHandler();
		//HandleBillValidatorCommand();
		//delay_1ms(1);
		EXTCMD_PROCESS();
    }
}

