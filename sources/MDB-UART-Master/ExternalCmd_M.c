/*
 * ExternalCmd_M.c
 *
 * Created: 26.08.2019 09:47:57
 *  Author: root
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "USART_M.h"
#include "CoinChanger_M.h"
#include "BillValidator_M.h"
#include "CoinHopper_M.h"

void EXTCMD_PROCESS() {//receive commands from VMC
	int cnt = 0;
	uint8_t tmplen = EXT_UART_BUFFER_COUNT;
	uint8_t TMP[tmplen];
	memcpy(&TMP, &EXT_UART_BUFFER, tmplen);
	EXT_UART_BUFFER_COUNT = 0;
	EXTCMDCOMPLETE = 0;
	for (int i = 0; i < tmplen; i++)
	{
		if (TMP[i] == 0x2a) cnt++;
	}
	uint8_t (tmp[cnt])[6];
	int tmpcnt = 0;
	char * p = strtok(TMP, "*");
	while (p) {
		if ((tmpcnt < cnt) && (sizeof(p) <= 6)) strcpy(&tmp[tmpcnt++], p);
		p = strtok(NULL, "*");
	}
	if (tmpcnt > 0)
	{
		uint16_t toplevelcmd = atoi(&tmp[0]);
		uint16_t secondlevelcmd = atoi(&tmp[1]);
		switch (toplevelcmd)
		{
			case 0:
			{
				switch (secondlevelcmd)
				{
					case 0:
					ResetAll();
					break;
					case 1:
					ResetCoinChangerOptions();
					break;
					case 2:
					ResetBVOptions();
					break;
					case 3:
					ResetCoinHoppersOptions();
					break;
				}
			}
			break;
			case 1:
			switch (secondlevelcmd)
			{
				case 1:
				MDBDeviceReset(0x08);
				break;
				case 2:
				GetCoinChangerSetupData();
				GetCoinChangerTubeStatus();
				GetCoinChangerIdentification();
				break;
				case 3:
				CoinChangerEnableAcceptCoins();
				break;
				case 4:
				CoinChangerDisableAcceptCoins();
				break;
				case 5:
				{
					if (cnt == 4)
					{
						uint8_t DispenseParams = (atoi(&tmp[3]) << 4) & 0xff;//high 4 bits - quantity, max value = 15
						DispenseParams = DispenseParams | ((atoi(&tmp[2]) & 0x0f) - 1);//lower 4 bits - coin type, max value = 15
						CoinChangerDispense(DispenseParams);
					}
				}
				break;
				case 6:
				if (cnt == 3)
				{
					CoinChangerAlternativePayout(atoi(&tmp[2]) & 0xff);
				}
				break;
				case 7:
				//CoinChangerControlledManualFillReport();
				break;
				case 8:
				if (cnt == 5)
				{
					CoinChangerEnableCoinType(atoi(&tmp[2]),atoi(&tmp[3]),atoi(&tmp[4]));
				}
				break;
				case 9:
				if (cnt == 5)
				{
					CoinChangerConfigFeatures(atoi(&tmp[2]),atoi(&tmp[3]),atoi(&tmp[4]));
				}
				break;
			}
			break;
			case 2:
			switch (secondlevelcmd)
			{
				case 1:
				MDBDeviceReset(0x30);
				break;
				case 2:
				GetBillValidatorSetupData();
				break;
				case 3:
				BillValidatorEnableAcceptBills();
				break;
				case 4:
				BillValidatorDisableAcceptBills();
				break;
				case 5:
				if (cnt == 3)
				{
					BillValidatorEscrow(atoi(&tmp[2]) & 0x01);
				}
				break;
				case 6:
				if (cnt == 4)
				{
					BVDispenseBills(atoi(&tmp[2]) & 0xff, atoi(&tmp[3]) & 0xffff);
				}
				break;
				case 7:
				if (cnt == 3)
				{
					BVDispenseValue(atoi(&tmp[2]) & 0xffff);
				}
				break;
				case 8:
				if (cnt == 8)
				{
					BillValidatorEnableBillType(atoi(&tmp[2]),atoi(&tmp[3]),atoi(&tmp[4]),atoi(&tmp[5]),atoi(&tmp[6]),atoi(&tmp[7]));
				}
				break;
				case 9:
				if (cnt == 3)
				{
					BillValidatorConfigFeatures(atoi(&tmp[2]));
				}
				break;
				case 10:
				BillValidatorCancelPayout();
				break;
			}
			break;
			case 3:
			if (cnt >= 3)
			{
				uint8_t index = (atoi(&tmp[1]) == 1) ? 0 : 1;
				uint16_t thirdlevelcmd = atoi(&tmp[2]);
				switch (thirdlevelcmd)
				{
					case 1:
					MDBDeviceReset((index) ? 0x73 : 0x58);
					break;
					case 2:
					GetCoinHopperSetupData(index);
					GetCoinHopperIdentification(index);
					break;
					case 6:
					if (cnt == 5)
					{
						CoinHopperDispenseCoins(index, atoi(&tmp[3]) & 0xff, atoi(&tmp[4]) & 0xffff);
					}
					break;
					case 7:
					if (cnt == 4)
					{
						CoinHopperDispenseValue(index, atoi(&tmp[3]) & 0xffff);
					}
					break;
					case 8:
					if (cnt == 5)
					{
						CoinHopperEnableManualDispenseCoinType(index, atoi(&tmp[3]) & 0xff, atoi(&tmp[4]) & 0xff);
					}
					break;
				}
			}
			break;
		}
	}
}