/*
 * MDB-UART-MASTER firmware
 * main.c
 * Created: 31.03.2019 12:47:10
 * Author : root
 */ 
#define F_CPU 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "USART_M.h"
#include "LEDControl_M.h"
#include "Cashless_M.h"
#include "BillValidator_M.h"
#include "CoinChanger_M.h"
#include "CoinHopper_M.h"
#include "Settings_M.h"
#include "MDB_M.h"
#include "ExternalCmd_M.h"

uint16_t IntCycles = 0;

uint8_t Version[] = "1.1.1730";

void Setup() {
	MDB_Setup();
	EXT_UART_Setup();
	EXT_UART_Transmit("SYS*MDBSTART*");
	EXT_UART_Transmit(Version);
	EXT_CRLF();
	//wait a bit for slaves initialization
	delay_1ms(1000);
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

void DispatchCommandOrPoll()
{
	//Define coin changer command
	switch (CoinChangerDevice.Status)
	{
		case 3:// alternative payout in progress
		CoinChangerAlternativePayoutValue();
		break;
		case 4:// its time to get diagnostic data
		GetCoinChangerTubeStatus();
		CoinChangerDevice.Status = 1;
		break;
		case 5:// its time to get diagnostic data
		if (CoinChangerSetupData.CoinChangerFeatureLevel >= 2 && CoinChangerIDData.ExtendedDiagnostic && ((CoinChangerOptions.EnableExtOptionsBits & 0x01) == 1)) GetCoinChangerDiagnosticStatus();
		CoinChangerDevice.Status = 1;
		break;
		default:// otherwise, just poll
		PollDevice(0x0b);
		break;
	}
	//Define coin hopper command
	for (int i = 0; i < 2; i++)
	{
		switch (CoinHopperDevice[i].Status)
		{
			case 3:// alternative payout in progress
			GetCoinHopperPayoutValue(i);
			break;
			case 4:
			GetCoinHopperDispenserStatus(i);
			CoinHopperDevice[i].Status = 1;
			break;
			default:// otherwise, just poll
			{
				uint8_t addr = i ? 0x73 : 0x5B;
				PollDevice(addr);
				break;
			}
		}
	}
	//Define cashless device command
	for (int i = 0; i < 2; i++)
	{
		//switch (CashlessDevice[i].Status)
		//{
			//default:// just poll
			//ReaderVendCancel(i);
			////CashlessDeviceSetup(i);
			////PollReader(i);
			//ReaderVendRequest(i,19.95,0);
			//ReaderVendSuccess(i, 25);
			//ReaderVendFailure(i);
			//ReaderSessionComplete(i);
			////PollReader(i);
			////CashlessDeviceSetup(i);
			//break;
		//}
		delay_1ms(1);
	}
	//Define bill validation device command
	switch (BillValidatorDevice.Status)
	{
		case 3:// alternative payout in progress
		BillValidatorPayoutValue();
		break;
		case 4:
		GetBillValidatorStackerStatus();
		if (BillValidatorIDData.BillRecyclingSupported) GetBVDispenserStatus();
		BillValidatorDevice.Status = 1;
		break;
		default:// otherwise, just poll
		PollDevice(0x33);
		break;
	}
}

void DispatchExternalCommand()
{
	if (EXTCMDCOMPLETE == 1)
	{
		EXTCMD_PROCESS();
	}
}

void CountCycles()
{
	if ((IntCycles % 666) == 0)
	{
		if (CoinChangerDevice.Status != 0)
		{
			CoinChangerDevice.Status = 4;
		}
		for (int i = 0; i < 2; i++)
		{
			//Define coin hopper command
			if (CoinHopperDevice[i].Status != 0)
			{
				CoinHopperDevice[i].Status = 4;
			}
		}
		if (BillValidatorDevice.Status != 0)
		{
			BillValidatorDevice.Status = 4;
		}
	} else
	{
		if ((IntCycles % 37) == 0)
		{
			if (CoinChangerDevice.Status != 0)
			{
				CoinChangerDevice.Status = 5;
				//The VMC should periodically transmit the command
				//approximately every 1 to 10 seconds.
			}
		}
	}
	if (IntCycles == 65535)
	{
		IntCycles = 0;
	}
	IntCycles++;
}

void DispatchDeviceLED()
{
	CoinChangerDevice.Status = (CoinChangerDevice.OfflinePollsCount == 0) ? 0 : CoinChangerDevice.Status;
	BillValidatorDevice.Status = (BillValidatorDevice.OfflinePollsCount == 0) ? 0 : BillValidatorDevice.Status;
	if (CoinChangerDevice.Status > 0) CCLED_ON(); else CCLED_OFF();
	if (BillValidatorDevice.Status > 0) BVLED_ON(); else BVLED_OFF();
	for (int i = 0; i < 2; i++)
	{
		CoinHopperDevice[i].Status = (CoinHopperDevice[i].OfflinePollsCount == 0) ? 0 : CoinHopperDevice[i].Status;
		if (CoinHopperDevice[i].Status > 0) CHLED_ON(i); else CHLED_OFF(i);
	}
}

void ReadSettings()
{
	ReadVMCData();
	ReadCoinChangerOptions();
	ReadBVOptions();
	ReadCoinHoppersOptions();
}

int main(void)
{
	Setup();
	ReadSettings();
	ResetAll();
	//cashless features not completed yet
	//ReaderReset(0);
	//ReaderReset(1);
	//CashlessDeviceSetup(0);
	//CashlessDeviceSetup(1);
    while (1)
    {
		DispatchExternalCommand();
		CountCycles();
		DispatchCommandOrPoll();
		//Uncomment next line when using in rev2a board
		DispatchDeviceLED();
		//BillValidatorEnableAcceptBills();
		delay_1ms(100);
    }
}