/*
 * CoinChanger_M.c
 *
 * Created: 18.05.2019 09:58:48
 *  Author: root
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
#include "MDB_M.h"
#include "USART_M.h"
#include "CoinChanger_M.h"
#include "LEDControl_M.h"
#include "Settings_M.h"

uint8_t CoinChangerInManualFillOrPaymentMode = 0;

void GetCoinChangerSetupData()
{
	uint8_t cmd[2] = {0x09, 0x09};
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete)
	{
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			CoinChangerSetupData.CoinChangerFeatureLevel = MDB_BUFFER[0].data;
			uint8_t cocd[2] = {MDB_BUFFER[1].data, MDB_BUFFER[2].data};
			CoinChangerSetupData.CountryOrCurrencyCode = BCDByteToInt(cocd);
			CoinChangerSetupData.CoinScalingFactor = MDB_BUFFER[3].data;
			CoinChangerSetupData.DecimalPlaces = MDB_BUFFER[4].data;
			uint16_t tmpcr  = MDB_BUFFER[5].data;
			tmpcr = (tmpcr << 8) | MDB_BUFFER[6].data;
			for (int i = 0; i < 16; i++)
			{
				CoinChangerSetupData.CoinsRouteable[i] = ((tmpcr & (1 << i)) != 0);
			}
			for (int i = 7; i < MDB_BUFFER_COUNT - 1; i++)
			{
				CoinChangerSetupData.CoinTypeCredit[i - 7] = MDB_BUFFER[i].data;
			}
			char tmpstr[80];
			uint8_t mcvbuff[5 + CoinChangerSetupData.DecimalPlaces];
			double mindispvalue = CoinChangerSetupData.CoinScalingFactor / pow(10, CoinChangerSetupData.DecimalPlaces);
			dtostrf(mindispvalue,0,CoinChangerSetupData.DecimalPlaces,mcvbuff);
			sprintf(tmpstr,"CC*CFG*%d*%d*%s\r\n", CoinChangerSetupData.CoinChangerFeatureLevel, CoinChangerSetupData.CountryOrCurrencyCode, mcvbuff);
			EXT_UART_Transmit(tmpstr);
			for (int i = 0; i < 16; i++)
			{
				if (CoinChangerSetupData.CoinsRouteable[i] == 1)
				{
					uint8_t cvbuff[10 + CoinChangerSetupData.DecimalPlaces];
					uint8_t buff[18 + sizeof(cvbuff)];
					double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i]) / pow(10, CoinChangerSetupData.DecimalPlaces);
					dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
					sprintf(buff,"CC*COINSUP*%d*%s*%d*%d\r\n",i + 1,cvbuff,(CoinChangerOptions.EnableAcceptCoinsBits >> i) & 0x01,(CoinChangerOptions.EnableDispenseCoinsBits >> i) & 0x01);
					EXT_UART_Transmit(buff);
				}
			}
		} else
		{
			if (MDB_BUFFER[0].data == 0x00)
			{
			
			}
		}
	} else 
	{
		EXT_UART_Transmit("CC*CFGERR");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void GetCoinChangerTubeStatus()
{
	uint8_t cmd[2] = {0x0a, 0x0a};
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1){
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			uint16_t fullflags  = MDB_BUFFER[0].data;
			fullflags = (fullflags << 8) | MDB_BUFFER[1].data;
			for (int i = 2; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if ((MDB_BUFFER[i].data != 0) || (fullflags & (1 << (i - 2)) == 1))
				{
					uint8_t tmpstr[32];
					uint8_t buff[5 + CoinChangerSetupData.DecimalPlaces];
					double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i - 2]) / pow(10, CoinChangerSetupData.DecimalPlaces);
					dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,buff);
					sprintf(tmpstr,"CC*TUBE*%d*%s*%d*%d", i - 1, buff, MDB_BUFFER[i].data, fullflags & (1 << (i - 2)));
					if ((MDB_BUFFER[i].data == 0x00) && (fullflags & (1 << (i - 2)) == 1)) EXT_UART_Transmit("*ERR");
					EXT_UART_Transmit(tmpstr);
					EXT_CRLF();
				}
			}
			} else{
			if (MDB_BUFFER[0].data == 0x00){
				
			}
		}
		} else {
		EXT_UART_Transmit("CC*TUBERR");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void CoinChangerPollResponse()
{
	CoinChangerDevice.OfflinePollsCount = 5;
	uint8_t tmpstr[64];
	uint8_t cvbuff[8];
	uint16_t tmplen = MDB_BUFFER_COUNT;
	MDB_Byte TMP[tmplen];
	memcpy(&TMP, &MDB_BUFFER, MDB_BUFFER_COUNT * 2);
	for (int i = 0; i < tmplen - 1; i++)
	{
		if ((TMP[i].data >> 5) == 1)
		{
			uint16_t slugs = (TMP[i].data & 0x1f);
			sprintf(tmpstr,"CC*SLUG*%d", slugs);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
		if ((TMP[i].data >> 4) == 0)
		{
			uint8_t statusbuff[20];
			switch (TMP[i].data & 0x0f)
			{
				case 1:
				sprintf(statusbuff,"%s", "ESCROWREQ");
				break;
				case 2:
				sprintf(statusbuff,"%s", "PAYOUTBUSY");
				break;
				case 3:
				sprintf(statusbuff,"%s", "NOCREDIT");
				break;
				case 4:
				sprintf(statusbuff,"%s", "BADTUBESENSOR");
				break;
				case 5:
				sprintf(statusbuff,"%s", "DOUBLECOIN");
				break;
				case 6:
				sprintf(statusbuff,"%s", "UNPLUGGED");
				break;
				case 7:
				sprintf(statusbuff,"%s", "TUBEJAM");
				break;
				case 8:
				sprintf(statusbuff,"%s", "ROMERROR");
				break;
				case 9:
				sprintf(statusbuff,"%s", "ROUTERROR");
				break;
				case 10:
				sprintf(statusbuff,"%s", "BUSY");
				break;
				case 11:
				sprintf(statusbuff,"%s", "JUSTRESET");
				//The following initialization sequence is recommended for all new VMCs
				//designed after July, 2000. It should be used after “power up”, after issuing
				//the RESET command, after issuing the Bus Reset (pulling the transmit line
				//“active” for a minimum of 100 mS), or anytime a POLL command results in a
				//“JUST RESET” response (i.e., peripheral self resets).
				CoinChangerDevice.Status = 1;
				CoinChangerDevice.OfflinePollsCount = 5;
				sprintf(tmpstr,"CC*STATUS*%s\r\n", &statusbuff);
				EXT_UART_Transmit(tmpstr);
				GetCoinChangerSetupData();
				if (CoinChangerSetupData.CoinChangerFeatureLevel >= 2)
				{
					GetCoinChangerIdentification();
					CoinChangerEnableFeatures();
					GetCoinChangerDiagnosticStatus();
				}
				CoinChangerControlledManualFillReport();
				GetCoinChangerTubeStatus();
				return;
				case 12:
				sprintf(statusbuff,"%s", "COINJAM");
				break;
				case 13:
				sprintf(statusbuff,"%s", "FISHING");
				break;
			}
			sprintf(tmpstr,"CC*STATUS*%s", &statusbuff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
		if ((TMP[i].data >> 7) == 1)
		{
			uint8_t cdmnumber = ((TMP[i].data & 0x70) >> 4);
			uint8_t coinsintube = (TMP[i + 1].data);
			uint8_t cointype = TMP[i].data & 0x0f;
			uint8_t cvbuff[5 + CoinChangerSetupData.DecimalPlaces];
			double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[cointype]) / pow(10, CoinChangerSetupData.DecimalPlaces);
			dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
			sprintf(tmpstr,"CC*MANUALDISP*%d*%s*%d*%d", cointype + 1, cvbuff, cdmnumber, coinsintube);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			i++;
		} else if ((TMP[i].data >> 6) == 1)
		{
			uint8_t coinrouting = ((TMP[i].data & 0x30) >> 4);
			uint8_t coinsintube = (TMP[i + 1].data);
			uint8_t cointype = TMP[i].data & 0x0f;
			uint8_t cvbuff[5 + CoinChangerSetupData.DecimalPlaces];
			double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[cointype]) / pow(10, CoinChangerSetupData.DecimalPlaces);
			dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
			uint8_t routbuff[10];
			switch (coinrouting)
			{
				case 0:
				sprintf(routbuff,"%s", "CASHBOX");
				break;
				case 1:
				sprintf(routbuff,"%s", "TUBE");
				break;
				case 2:
				sprintf(routbuff,"%s", "NA");
				break;
				case 3:
				sprintf(routbuff,"%s", "REJECT");
				break;
			}
			sprintf(tmpstr,"CC*DEPOSIT*%d*%s*%s*%d", cointype + 1, &cvbuff, &routbuff, coinsintube);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			i++;
		}
	}
}

void CoinChangerEnableCoinType(uint8_t CoinType, uint8_t EnableAccept, uint8_t EnableDispense)
{
	uint8_t buff[27 + CoinChangerSetupData.DecimalPlaces];
	CoinChangerOptions.EnableAcceptCoinsBits = (EnableAccept == 1) ? (CoinChangerOptions.EnableAcceptCoinsBits | (1 << (CoinType - 1))) : (CoinChangerOptions.EnableAcceptCoinsBits & ~(1 << (CoinType - 1)));
	CoinChangerOptions.EnableDispenseCoinsBits = (EnableDispense == 1) ? (CoinChangerOptions.EnableDispenseCoinsBits | (1 << (CoinType - 1))) : (CoinChangerOptions.EnableDispenseCoinsBits & ~(1 << (CoinType - 1)));
	WriteCoinChangerOptions();
	uint8_t cvbuff[5 + CoinChangerSetupData.DecimalPlaces];
	double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[CoinType - 1]) / pow(10, CoinChangerSetupData.DecimalPlaces);
	dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
	sprintf(buff,"CC*COINCFG*%d*%s*%d*%d*", CoinType, cvbuff, (EnableAccept == 1), (EnableDispense == 1));
	EXT_UART_Transmit(buff);
	EXT_UART_OK();
}

void CoinChangerEnableAcceptCoins()
{
	uint8_t cmd[6];
	cmd[0] = 0x0c;
	cmd[1] = (CoinChangerOptions.EnableAcceptCoinsBits >> 8) & 0xff;
	cmd[2] = CoinChangerOptions.EnableAcceptCoinsBits & 0xff;
	cmd[3] = (CoinChangerOptions.EnableDispenseCoinsBits >> 8) & 0xff;
	cmd[4] = CoinChangerOptions.EnableDispenseCoinsBits & 0xff;
	cmd[5] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff);
	MDB_Send(cmd, 6);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("CC*ENABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			CoinChangerDevice.OfflinePollsCount = 5;
			return;
		}
	}
	EXT_UART_FAIL();
	if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
}

void CoinChangerDisableAcceptCoins()
{
	uint8_t cmd[6];
	cmd[0] = 0x0c;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff);
	MDB_Send(cmd, 6);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("CC*DISABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			CoinChangerDevice.OfflinePollsCount = 5;
			return;
		}
	}
	EXT_UART_FAIL();
	if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
}

void CoinChangerDispense(uint8_t DispenseParams)
{
	uint8_t cmd[3];
	cmd[0] = 0x0d;
	cmd[1] = DispenseParams;
	cmd[2] = ((cmd[0] + cmd[1]) & 0xff);
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CoinChangerDevice.OfflinePollsCount = 5;
		EXT_UART_Transmit("CC*DISPENSE*");
		switch (MDB_BUFFER[0].data)
		{
			case 0x00:
			CoinChangerDevice.Status = 2;//awaiting dispense
			EXT_UART_OK();
			break;
			case 0xff:
			EXT_UART_NAK();
			break;
			default:
			EXT_UART_Transmit("UNKNOWN");
			EXT_CRLF();
			break;
		}
	} else
	{
		EXT_UART_Transmit("CC*DISPENSE*FAIL");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void CoinChangerAlternativePayout(uint8_t PayoutValue)
{
	uint8_t cmd[4];
	cmd[0] = 0x0f;
	cmd[1] = 0x02;
	cmd[2] = PayoutValue;
	cmd[3] = ((cmd[0] + cmd[1] + cmd[2]) & 0xff);
	MDB_Send(cmd, 4);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("CC*SUMPAYOUT*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CoinChangerDevice.OfflinePollsCount = 5;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			CoinChangerDevice.Status = 3;//Awaiting dispense complete
			EXT_UART_OK();
			return;
		}
	}
	EXT_UART_FAIL();
	if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
}

void CoinChangerAlternativePayoutStatus()
{
	uint8_t cmd[3];
	cmd[0] = 0x0f;
	cmd[1] = 0x03;
	cmd[2] = 0x12;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (!MDB_BUFFER[0].mode)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			for (int i = 0; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if (MDB_BUFFER[i].data > 0)
				{
					uint8_t tmpstr[10 + CoinChangerSetupData.DecimalPlaces];
					EXT_UART_Transmit("CC*PAYSTATUS");
					uint8_t cvbuff[5 + CoinChangerSetupData.DecimalPlaces];
					double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i]) / pow(10, CoinChangerSetupData.DecimalPlaces);
					dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
					sprintf(tmpstr,"*%s", cvbuff);
					EXT_UART_Transmit(tmpstr);
					sprintf(cvbuff,"*%d", MDB_BUFFER[i].data);
					EXT_UART_Transmit(cvbuff);
					EXT_CRLF();
				}
			}
			GetCoinChangerTubeStatus();
		} else
		{
			EXT_UART_Transmit("CC*PAYSTATUS*BUSY");
			EXT_CRLF();
		}
	} else
	{
		EXT_UART_Transmit("CC*PAYSTATUS*FAIL");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void CoinChangerAlternativePayoutValue()
{
	uint8_t cmd[3];
	cmd[0] = 0x0f;
	cmd[1] = 0x04;
	cmd[2] = 0x13;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (!MDB_BUFFER[0].mode)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			uint8_t cvbuff[5 + CoinChangerSetupData.DecimalPlaces];
			double coinvalue = (CoinChangerSetupData.CoinScalingFactor * MDB_BUFFER[0].data) / pow(10, CoinChangerSetupData.DecimalPlaces);
			dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,cvbuff);
			EXT_UART_Transmit("CC*PAID*");
			EXT_UART_Transmit(cvbuff);
			EXT_CRLF();
		} else
		{
			EXT_UART_Transmit("CC*PAYOUTEND");
			EXT_CRLF();
			CoinChangerDevice.Status = 1;
			CoinChangerAlternativePayoutStatus();
		}
	} else
	{
		EXT_UART_Transmit("CC*PAYSTATUSFAIL");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void CoinChangerConfigFeatures(uint8_t AlternativePayout, uint8_t ExtendedDiagnostic, uint8_t ControlledManualFillAndPayout)
{
	CoinChangerOptions.EnableExtOptionsBits = (AlternativePayout == 1) ? (CoinChangerOptions.EnableExtOptionsBits | 1) : (CoinChangerOptions.EnableExtOptionsBits & 0xfe);
	CoinChangerOptions.EnableExtOptionsBits = (ExtendedDiagnostic == 1) ? (CoinChangerOptions.EnableExtOptionsBits | 2) : (CoinChangerOptions.EnableExtOptionsBits & 0xfd);
	CoinChangerOptions.EnableExtOptionsBits = (ControlledManualFillAndPayout == 1) ? (CoinChangerOptions.EnableExtOptionsBits | 4) : (CoinChangerOptions.EnableExtOptionsBits & 0xfb);
	CoinChangerOptions.EnableExtOptionsBits |= (0 << 3);
	WriteCoinChangerOptions();
	EXT_UART_Transmit("CC*FEATCFG*");
	EXT_UART_OK();
	CoinChangerEnableFeatures();
}

void CoinChangerEnableFeatures()
{
	EXT_UART_Transmit("CC*FEATENABLE*");
	uint8_t cmd[7];
	cmd[0] = 0x0f;
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = CoinChangerOptions.EnableExtOptionsBits;
	cmd[6] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff);
	MDB_Send(cmd, 7);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CoinChangerDevice.OfflinePollsCount = 5;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			return;
		}
	}
	EXT_UART_FAIL();
	if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
}

void GetCoinChangerIdentification()
{
	uint8_t cmd[3] = {0x0f, 0x00, 0x0f};
	MDB_Send(cmd,3);
	while (!MDBReceiveComplete)
	{
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			uint8_t tmpstr[64];
			for (int i = 0; i < 3; i++)
			{
				CoinChangerIDData.ManufacturerCode[i] = 0x00;
			}
			for (int i = 0; i < 12; i++)
			{
				CoinChangerIDData.SerialNumber[i] = 0x00;
			}
			for (int i = 0; i < 12; i++)
			{
				CoinChangerIDData.ModelRevision[i] = 0x00;
			}
			uint8_t tmpmfg[3] = {MDB_BUFFER[0].data, MDB_BUFFER[1].data, MDB_BUFFER[2].data};
			memcpy(CoinChangerIDData.ManufacturerCode, tmpmfg, 3);
			EXT_UART_Transmit("CC*ID*");
			EXT_UART_Transmit(CoinChangerIDData.ManufacturerCode);
			uint8_t tmpsn[12] = {MDB_BUFFER[3].data, MDB_BUFFER[4].data, MDB_BUFFER[5].data, MDB_BUFFER[6].data, MDB_BUFFER[7].data, MDB_BUFFER[8].data, MDB_BUFFER[9].data, MDB_BUFFER[10].data, MDB_BUFFER[11].data, MDB_BUFFER[12].data, MDB_BUFFER[13].data, MDB_BUFFER[14].data};
			memcpy(CoinChangerIDData.SerialNumber,tmpsn, 12);
			EXT_UART_Transmit("*");
			EXT_UART_Transmit(CoinChangerIDData.SerialNumber);
			uint8_t tmpmr[12] = {MDB_BUFFER[15].data, MDB_BUFFER[16].data, MDB_BUFFER[17].data, MDB_BUFFER[18].data, MDB_BUFFER[19].data, MDB_BUFFER[20].data, MDB_BUFFER[21].data, MDB_BUFFER[22].data, MDB_BUFFER[23].data, MDB_BUFFER[24].data, MDB_BUFFER[25].data, MDB_BUFFER[26].data};
			memcpy(CoinChangerIDData.ModelRevision,tmpmr, 12);
			EXT_UART_Transmit("*");
			EXT_UART_Transmit(CoinChangerIDData.ModelRevision);
			uint8_t srd[2] = {MDB_BUFFER[27].data, MDB_BUFFER[28].data};
			CoinChangerIDData.SoftwareVersion = BCDByteToInt(srd);
			uint32_t flags  = MDB_BUFFER[29].data;
			flags = (flags << 8) | MDB_BUFFER[30].data;
			flags = (flags << 8) | MDB_BUFFER[31].data;
			flags = (flags << 8) | MDB_BUFFER[32].data;
			CoinChangerIDData.AlternativePayout = ((flags & (1 << 0)) != 0);
			CoinChangerIDData.ExtendedDiagnostic = ((flags & (1 << 1)) != 0);
			CoinChangerIDData.ControlledManualFillAndPayout = ((flags & (1 << 2)) != 0);
			CoinChangerIDData.FTLSupported = ((flags & (1 << 3)) != 0);
			sprintf(tmpstr,"*%d*%d*%d*%d*%d", CoinChangerIDData.SoftwareVersion, CoinChangerIDData.AlternativePayout, CoinChangerIDData.ExtendedDiagnostic, CoinChangerIDData.ControlledManualFillAndPayout, CoinChangerIDData.FTLSupported);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		} else
		{
			if (MDB_BUFFER[0].data == 0x00){
					
			}
		}
	} else
	{
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void GetCoinChangerDiagnosticStatus()
{
	uint8_t cmd[3] = {0x0f, 0x05, 0x14};
	uint8_t suppress = 0;
	MDB_Send(cmd,3);
	while (!MDBReceiveComplete)
	{
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 2 && (((MDB_BUFFER_COUNT - 1) % 2) == 0))
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			uint16_t tmplen = MDB_BUFFER_COUNT;
			MDB_Byte TMP[tmplen];
			memcpy(&TMP, &MDB_BUFFER, MDB_BUFFER_COUNT * 2);
			for (int i = 0; i < tmplen - 1; i++)
			{
				uint8_t statusvaluebytes[2] = {TMP[i].data, TMP[i + 1].data};
				uint16_t statusvalue = BCDByteToInt(statusvaluebytes);
				uint8_t tmpdmsg[16];
				sprintf(tmpdmsg,"%s*%02x%02x", "UNK", TMP[i].data, TMP[i + 1].data);
				if ((statusvalue != 510) && CoinChangerInManualFillOrPaymentMode)
				{
					CoinChangerInManualFillOrPaymentMode = 0;
					CoinChangerControlledManualFillReport();
				}
				switch (statusvalue)
				{
					case 100:
					sprintf(tmpdmsg,"%s", "POWERUP");
					break;
					case 200:
					sprintf(tmpdmsg,"%s", "POWERDOWN");
					break;
					case 300:
					sprintf(tmpdmsg,"%s", "OK");
					//we will suppress diagnostic output if all is OK with coin changer
					//suppress = 1;
					break;
					case 400:
					sprintf(tmpdmsg,"%s", "KEYPADSHIFTED");
					break;
					case 510:
					sprintf(tmpdmsg,"%s", "MANUALFILLPAY");
					if (CoinChangerInManualFillOrPaymentMode != 1) CoinChangerInManualFillOrPaymentMode = 1;
					break;
					case 520:
					sprintf(tmpdmsg,"%s", "NEWINVENTORY");
					break;
					case 600:
					sprintf(tmpdmsg,"%s", "INHIBITED");
					break;
				}
				switch (TMP[i].data)
				{
					case 0x10:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "ERROR");
								break;
							}
							case 0x01:
							{
								sprintf(tmpdmsg,"%s", "CSERR1");
								break;
							}
							case 0x02:
							{
								sprintf(tmpdmsg,"%s", "CSERR2");
								break;
							}
							case 0x03:
							{
								sprintf(tmpdmsg,"%s", "LOWVOLTAGE");
								break;
							}
						}
					}
					break;
					case 0x11:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "DISCERR");
								break;
							}
							case 0x10:
							{
								sprintf(tmpdmsg,"%s", "DISCDECK");
								break;
							}
							case 0x11:
							{
								sprintf(tmpdmsg,"%s", "DISCOPN");
								break;
							}
							case 0x30:
							{
								sprintf(tmpdmsg,"%s", "DISCJAM");
								break;
							}
							case 0x41:
							{
								sprintf(tmpdmsg,"%s", "DISCBLSTD");
								break;
							}
							case 0x50:
							{
								sprintf(tmpdmsg,"%s", "DISCASENS");
								break;
							}
							case 0x51:
							{
								sprintf(tmpdmsg,"%s", "DISCBSENS");
								break;
							}
							case 0x52:
							{
								sprintf(tmpdmsg,"%s", "DISCCSENS");
								break;
							}
							case 0x53:
							{
								sprintf(tmpdmsg,"%s", "DISCTMP");
								break;
							}
							case 0x54:
							{
								sprintf(tmpdmsg,"%s", "DISCOPT");
								break;
							}
						}
					}
					break;
					case 0x12:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "GATERR");
								break;
							}
							case 0x30:
							{
								sprintf(tmpdmsg,"%s", "GATNX");
								break;
							}
							case 0x31:
							{
								sprintf(tmpdmsg,"%s", "GATALM");
								break;
							}
							case 0x40:
							{
								sprintf(tmpdmsg,"%s", "GATND");
								break;
							}
							case 0x50:
							{
								sprintf(tmpdmsg,"%s", "GATSENS");
								break;
							}
						}
					}
					break;
					case 0x13:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "SEPERR");
								break;
							}
							case 0x10:
							{
								sprintf(tmpdmsg,"%s", "SEPSENS");
								break;
							}
						}
					}
					break;
					case 0x14:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "DISPERR");
								break;
							}
						}
					}
					break;
					case 0x15:
					{
						switch (TMP[i + 1].data)
						{
							case 0x00:
							{
								sprintf(tmpdmsg,"%s", "CASERR");
								break;
							}
							case 0x02:
							{
								sprintf(tmpdmsg,"%s", "CASRMD");
								break;
							}
							case 0x03:
							{
								sprintf(tmpdmsg,"%s", "CASSENS");
								break;
							}
							case 0x04:
							{
								sprintf(tmpdmsg,"%s", "CASLIT");
								break;
							}
						}
					}
					break;
				}
				i++;
				if (!suppress)
				{
					EXT_UART_Transmit("CC*DIAG*");
					EXT_UART_Transmit(tmpdmsg);
					EXT_CRLF();
				}
			}
		} else
		{
			EXT_UART_Transmit("CC*DIAG*NACK");
			EXT_CRLF();
		}
	} else
	{
		EXT_UART_Transmit("CC*DIAG*ND");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}

void CoinChangerControlledManualFillReport()
{
	uint8_t cmd[3];
	cmd[0] = 0x0f;
	cmd[1] = 0x06;
	cmd[2] = 0x15;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			for (int i = 0; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if (MDB_BUFFER[i].data > 0)
				{
					uint8_t tmpstr[10 + CoinChangerSetupData.DecimalPlaces];
					EXT_UART_Transmit("CC*MANUALFILL");
					uint8_t buff[5 + CoinChangerSetupData.DecimalPlaces];
					double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i]) / pow(10, CoinChangerSetupData.DecimalPlaces);
					dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,buff);
					sprintf(tmpstr,"*%s", buff);
					EXT_UART_Transmit(tmpstr);
					sprintf(buff,"*%d", MDB_BUFFER[i].data);
					EXT_UART_Transmit(buff);
					EXT_CRLF();
				}
			}
			//GetCoinChangerTubeStatus();
		} else
		{
			EXT_UART_Transmit("CC*MANUALFILL*UNKNOWN");
			EXT_CRLF();
		}
	} else
	{
		EXT_UART_Transmit("CC*MANUALFILL*FAIL");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
	CoinChangerControlledManualPayoutReport();
}

void CoinChangerControlledManualPayoutReport()
{
	uint8_t cmd[3];
	cmd[0] = 0x0f;
	cmd[1] = 0x07;
	cmd[2] = 0x16;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CoinChangerDevice.OfflinePollsCount = 5;
			for (int i = 0; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if (MDB_BUFFER[i].data > 0)
				{
					uint8_t tmpstr[10 + CoinChangerSetupData.DecimalPlaces];
					EXT_UART_Transmit("CC*MANUALPAYOUT");
					uint8_t buff[5 + CoinChangerSetupData.DecimalPlaces];
					double coinvalue = (CoinChangerSetupData.CoinScalingFactor * CoinChangerSetupData.CoinTypeCredit[i]) / pow(10, CoinChangerSetupData.DecimalPlaces);
					dtostrf(coinvalue,0,CoinChangerSetupData.DecimalPlaces,buff);
					sprintf(tmpstr,"*%s", buff);
					EXT_UART_Transmit(tmpstr);
					sprintf(buff,"*%d", MDB_BUFFER[i].data);
					EXT_UART_Transmit(buff);
					EXT_CRLF();
				}
			}
			//GetCoinChangerTubeStatus();
		} else
		{
			EXT_UART_Transmit("CC*MANUALPAYOUT*UNKNOWN");
			EXT_CRLF();
		}
	} else
	{
		EXT_UART_Transmit("CC*MANUALPAYOUT*FAIL");
		EXT_CRLF();
		if (CoinChangerDevice.OfflinePollsCount > 0) CoinChangerDevice.OfflinePollsCount--;
	}
}