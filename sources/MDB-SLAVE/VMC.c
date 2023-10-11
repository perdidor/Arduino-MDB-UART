/*
 * VMC.c
 *
 * Created: 11.05.2019 18:58:51
 *  Author: root
 */ 
#ifndef F_CPU
#define F_CPU       16000000UL
#endif

#include <avr/io.h>
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
#include "USART.h"
#include "VMC.h"


void ResetReaderVMCData(unsigned char index)
{
	ReaderVMCSetupData[index].dispaly_cols = 0;
	ReaderVMCSetupData[index].dispaly_rows = 0;
	ReaderVMCSetupData[index].dispaly_info = 0;
	ReaderVMCSetupData[index].feature_level = 0;
	ReaderVMCMaxPrice[index].Value = 0;
	ReaderVMCMinPrice[index].Value = 0;
}


void VMCCommandHandler()
{
	unsigned char buff[16];
	int cmditemcnt = 0;
	unsigned char tmplen = EXT_UART_BUFFER_COUNT;
	unsigned char TMP[tmplen];
	if (EXTCMDCOMPLETE == 1)
	{
		memcpy(&TMP, &EXT_UART_BUFFER, tmplen);
		EXT_UART_BUFFER_COUNT = 0;
		EXTCMDCOMPLETE = 0;
		for (int i = 0; i < tmplen; i++)
		{
			if (TMP[i] == 0x2a) cmditemcnt++;
		}
		int tmpcnt = 0;
		int startpos = 0;
		char * p = strtok(TMP, "*");
		while (p)
		{
			if ((tmpcnt < cmditemcnt) && (sizeof(p) <= 6)) strcpy(CommandsArray[tmpcnt++], p);
			p = strtok(NULL, "*");
		}
		if (tmpcnt > 0)
		{
			unsigned int toplevelcmd = atoi(&CommandsArray[0]);
			//unsigned int secondlevelcmd = atoi(&tmp[1]);
			switch (toplevelcmd)
			{
				case 1:
				CashlessExtCommandHandler(0);
				break;
				case 2:
				CashlessExtCommandHandler(1);
				break;
			}
		}
	}
}

void CashlessExtCommandHandler(unsigned char index)
{
	unsigned int secondlevelcmd = atoi(&CommandsArray[1]);
	switch (secondlevelcmd)
	{
		case 1:
		DebugMessage("[RESET]\r\n");
		ResetReaderVMCData(index);
		Reader_State[index] = MDB_INACTIVE;
		Reader_Active_Command[index] = MDB_IDLE;
		Reader_Poll_Reply[index] = MDB_REPLY_JUST_RESET;
		break;
	}
}
