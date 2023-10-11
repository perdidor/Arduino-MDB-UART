/*
 * CoinHopper_M.c
 *
 * Created: 18.05.2019 10:03:26
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
#include "CoinHopper_M.h"
#include "Settings_M.h"


void GetCoinHopperSetupData(uint8_t index)
{
	char tmpstr2[16];
	uint8_t cmd[2];
	if (!index)
	{
		cmd[0] = 0x71;
		cmd[1] = 0x71;
	} else
	{
		cmd[0] = 0x59;
		cmd[1] = 0x59;
	}
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CHLED_ON(index);
			CoinHopperSetupData[index].DispenserFeatureLevel = MDB_BUFFER[0].data;
			uint8_t cocd[2] = {MDB_BUFFER[1].data, MDB_BUFFER[2].data};
			CoinHopperSetupData[index].CountryOrCurrencyCode = BCDByteToInt(cocd);
			CoinHopperSetupData[index].CoinScalingFactor = MDB_BUFFER[3].data;
			CoinHopperSetupData[index].DecimalPlaces = MDB_BUFFER[4].data;
			CoinHopperSetupData[index].MaxResponseTime = MDB_BUFFER[5].data;
			uint16_t tmpdc  = MDB_BUFFER[6].data;
			tmpdc = (tmpdc << 8) | MDB_BUFFER[7].data;
			for (int i = 0; i < 16; i++)
			{
				CoinHopperSetupData[index].DisabledCoinTypes[i] = ((tmpdc & (1 << i)) != 0);
			}
			uint16_t tmpcsf  = MDB_BUFFER[8].data;
			tmpcsf = (tmpcsf << 8) | MDB_BUFFER[9].data;
			for (int i = 0; i < 16; i++)
			{
				CoinHopperSetupData[index].CoinSelfFilling[i] = ((tmpcsf & (1 << i)) != 0);
			}
			for (int i = 10; i < MDB_BUFFER_COUNT - 1; i++)
			{
				CoinHopperSetupData[index].CoinTypeCredit[i - 7] = MDB_BUFFER[i].data;
			}
			char tmpstr[80];
			uint8_t mcvbuff[5 + CoinHopperSetupData[index].DecimalPlaces];
			double mindispvalue = CoinHopperSetupData[index].CoinScalingFactor / pow(10, CoinHopperSetupData[index].DecimalPlaces);
			dtostrf(mindispvalue,0,CoinHopperSetupData[index].DecimalPlaces,mcvbuff);
			sprintf(tmpstr,"CH%d*CFG*%d*%d*%s", index + 1, CoinHopperSetupData[index].DispenserFeatureLevel, CoinHopperSetupData[index].CountryOrCurrencyCode, mcvbuff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			for (int i = 0; i < 16; i++)
			{
				uint8_t cvbuff[10 + CoinHopperSetupData[index].DecimalPlaces];
				uint8_t buff[18 + sizeof(cvbuff)];
				double coinvalue = (CoinHopperSetupData[index].CoinScalingFactor * CoinHopperSetupData[index].CoinTypeCredit[i]) / pow(10, CoinHopperSetupData[index].DecimalPlaces);
				dtostrf(coinvalue,0,CoinHopperSetupData[index].DecimalPlaces,cvbuff);
				sprintf(buff,"CH%d*COINSUP*%d*%s*%d*%d", index + 1, i + 1,cvbuff, (CoinHopperSetupData[index].DisabledCoinTypes[i] == 0x00), CoinHopperSetupData[index].CoinSelfFilling[i]);
				EXT_UART_Transmit(buff);
				EXT_CRLF();
			}
			} else {
			if (MDB_BUFFER[0].data == 0x00){
				
			}
		}
		} else {
		sprintf(tmpstr2,"CH%d*CFGERR", index + 1);
		EXT_UART_Transmit(tmpstr2);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void GetCoinHopperDispenserStatus(uint8_t index)
{
	char tmpstr2[16];
	uint8_t cmd[2];
	if (!index)
	{
		cmd[0] = 0x72;
		cmd[1] = 0x72;
	} else
	{
		cmd[0] = 0x5A;
		cmd[1] = 0x5A;
	}
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1){
			MDB_ACK();
			CHLED_ON(index);
			uint16_t fullflags  = MDB_BUFFER[0].data;
			fullflags = (fullflags << 8) | MDB_BUFFER[1].data;
			for (int i = 2; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if (CoinHopperSetupData[index].DisabledCoinTypes[(i - 2) / 2] == 0)
				{
					uint8_t tmpstr[70];
					uint8_t buff[5 + CoinHopperSetupData[index].DecimalPlaces];
					double coinvalue = (CoinHopperSetupData[index].CoinScalingFactor * CoinHopperSetupData[index].CoinTypeCredit[i]) / pow(10, CoinHopperSetupData[index].DecimalPlaces);
					dtostrf(coinvalue,0,CoinHopperSetupData[index].DecimalPlaces,buff);
					uint16_t coinsqty = MDB_BUFFER[i].data;
					coinsqty = (coinsqty << 8) | MDB_BUFFER[i + 1].data;
					sprintf(tmpstr,"CH%d*FILL*%s*%d*%d", index + 1, buff, coinsqty, fullflags & (1 << ((i - 2) / 2)));
					if ((coinsqty == 0x00) && (fullflags & (1 << ((i - 2) / 2)) == 1)) EXT_UART_Transmit("*ERR");
					EXT_UART_Transmit(tmpstr);
					EXT_CRLF();
					i++;
				}
			}
			} else{
			if (MDB_BUFFER[0].data == 0x00){
				
			}
		}
		} else {
		sprintf(tmpstr2,"CH%d*FILLERR", index + 1);
		EXT_UART_Transmit(tmpstr2);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void CoinHopperPollResponse(uint8_t index)
{
	CHLED_ON(index);
	uint8_t tmpstr[64];
	uint8_t cvbuff[8];
	uint16_t tmplen = MDB_BUFFER_COUNT;
	MDB_Byte TMP[tmplen];
	memcpy(&TMP, &MDB_BUFFER, MDB_BUFFER_COUNT * 2);
	for (int i = 0; i < tmplen - 1; i++)
	{
		if ((TMP[i].data >> 6) == 2)
		{
			uint8_t dispmode[8];
			uint8_t dispres[8];
			if ((TMP[i].data & (1 << 4)) != 0) sprintf(dispmode, "%s", "MANUAL"); else sprintf(dispmode, "%s", "AUTO");
			if ((TMP[i].data & (1 << 5)) != 0) sprintf(dispres, "%s", "OK"); else sprintf(dispres, "%s", "FAIL");
			uint8_t buff[5 + CoinHopperSetupData[index].DecimalPlaces];
			double coinvalue = (CoinHopperSetupData[index].CoinScalingFactor * CoinHopperSetupData[index].CoinTypeCredit[TMP[i].data & 0x0f]) / pow(10, CoinHopperSetupData[index].DecimalPlaces);
			dtostrf(coinvalue,0,CoinHopperSetupData[index].DecimalPlaces,buff);
			uint16_t coinsqty = TMP[i + 1].data;
			coinsqty = (coinsqty << 8) | TMP[i + 2].data;
			uint16_t coinsleft = TMP[i + 3].data;
			coinsleft = (coinsleft << 8) | TMP[i + 4].data;
			sprintf(tmpstr,"CH%d*DISPENSED*%s*%s*%s*%d*%d", index + 1, dispmode, dispres, buff, coinsqty, coinsleft);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			i += 4;
		}
		if ((TMP[i].data & 0xf0) == 0)
		{
			uint8_t statusbuff[32];
			switch (TMP[i].data & 0x0f)
			{
				case 1:
				sprintf(statusbuff,"%s", "ESCROWREQ");
				break;
				case 2:
				sprintf(statusbuff,"%s", "PAYOUTBUSY");
				break;
				case 3:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 4:
				sprintf(statusbuff,"%s", "BADSENSOR");
				break;
				case 5:
				sprintf(statusbuff,"%s", "NOTUSED");
				break;
				case 6:
				sprintf(statusbuff,"%s", "NOSTART");
				break;
				case 7:
				sprintf(statusbuff,"%s", "DISPJAM");
				break;
				case 8:
				sprintf(statusbuff,"%s", "ROMERROR");
				break;
				case 9:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 10:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 11:
				sprintf(statusbuff,"%s", "JUSTRESET");
				//The following initialization sequence is recommended for all new VMCs
				//designed after July, 2000. It should be used after “power up”, after issuing
				//the RESET command, after issuing the Bus Reset (pulling the transmit line
				//“active” for a minimum of 100 mS), or anytime a POLL command results in a
				//“JUST RESET” response (i.e., peripheral self resets).
				CoinHopperDevice[index].Status = 1;
				sprintf(tmpstr,"CH%d*STATUS*%s", index + 1, &statusbuff);
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
				GetCoinHopperSetupData(index);
				GetCoinHopperIdentification(index);
				GetCoinHopperDispenserStatus(index);
				return;
				case 12:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 13:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 14:
				sprintf(statusbuff,"%s", "NA");
				break;
				case 15:
				sprintf(statusbuff,"%s", "FILLEDKEY");
				GetCoinHopperDispenserStatus(index);
				break;
			}
			sprintf(tmpstr,"CH%d*STATUS*%s", index + 1, &statusbuff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
	}
}

void CoinHopperEnableManualDispenseCoinType(uint8_t index, uint8_t CoinType, uint8_t EnableManualDispense)
{
	CoinHopperOptions[index].EnableManualDispenseCoinsBits = (EnableManualDispense == 1) ? (CoinHopperOptions[index].EnableManualDispenseCoinsBits | (1 << (CoinType - 1))) : (CoinHopperOptions[index].EnableManualDispenseCoinsBits & ~(1 << (CoinType - 1)));
	WriteCoinHoppersOptions();
	uint8_t buff[25 + CoinHopperSetupData[index].DecimalPlaces];
	uint8_t cvbuff[5 + CoinHopperSetupData[index].DecimalPlaces];
	double coinvalue = (CoinHopperSetupData[index].CoinScalingFactor * CoinHopperSetupData[index].CoinTypeCredit[CoinType - 1]) / pow(10, CoinHopperSetupData[index].DecimalPlaces);
	dtostrf(coinvalue,0,CoinHopperSetupData[index].DecimalPlaces,cvbuff);
	sprintf(buff,"CH%d*COINCFG*%d*%s*%d*", index + 1, CoinType, cvbuff, (EnableManualDispense == 1));
	EXT_UART_Transmit(buff);
	EXT_UART_OK();
	uint8_t cmd[4];
	cmd[0] = (index) ? 0x74 : 0x5C;
	cmd[1] = (CoinHopperOptions[index].EnableManualDispenseCoinsBits >> 8) & 0xff;
	cmd[2] = CoinHopperOptions[index].EnableManualDispenseCoinsBits & 0xff;
	cmd[3] = (cmd[0] + cmd[1] + cmd[2]) & 0xff;
	MDB_Send(cmd, 4);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CHLED_ON(index);
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			return;
		}
	}
	CHLED_OFF(index);
}

void CoinHopperDispenseCoins(uint8_t index, uint8_t CoinType, uint16_t CoinsCount)
{
	char tmpstr[16];
	uint8_t cmd[6];
	if (!index)
	{
		cmd[0] = 0x75;
	} else
	{
		cmd[0] = 0x5D;
	}
	cmd[1] = 0x00;
	cmd[2] = CoinType;
	cmd[3] = CoinsCount & 0xff;
	cmd[4] = (CoinsCount >> 8) & 0xff;
	cmd[5] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff;
	MDB_Send(cmd, 6);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CHLED_ON(index);
		sprintf(tmpstr,"CH%d*DISPENSE*", index + 1);
		EXT_UART_Transmit(tmpstr);
		switch (MDB_BUFFER[0].data)
		{
			case 0x00:
			CoinHopperDevice[index].Status = 2;//awaiting dispense
			EXT_UART_OK();
			break;
			case 0xff:
			EXT_UART_NAK();
			break;
			default:
			EXT_UART_Transmit("UNK");
			EXT_CRLF();
			break;
		}
	} else
	{
		sprintf(tmpstr,"CH%d*DISPENSE*FAIL", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void CoinHopperDispenseValue(uint8_t index, uint16_t PayoutValue)
{
	char tmpstr[16];
	uint8_t cmd[5];
	if (!index)
	{
		cmd[0] = 0x75;
	} else
	{
		cmd[0] = 0x5D;
	}
	cmd[1] = 0x01;
	cmd[2] = PayoutValue & 0xff;
	cmd[3] = (PayoutValue >> 8) & 0xff;
	cmd[4] = (cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xff;
	MDB_Send(cmd, 5);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CHLED_ON(index);
		sprintf(tmpstr,"CH%d*SUMPAYOUT*", index + 1);
		EXT_UART_Transmit(tmpstr);
		switch (MDB_BUFFER[0].data)
		{
			case 0x00:
			CoinHopperDevice[index].Status = 3;//awaiting dispense
			EXT_UART_OK();
			break;
			case 0xff:
			EXT_UART_NAK();
			break;
			default:
			EXT_UART_Transmit("UNK");
			EXT_CRLF();
			break;
		}
	} else
	{
		sprintf(tmpstr,"CH%d*SUMPAYOUT*FAIL", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void CoinHopperPayoutStatus(uint8_t index)
{
	char tmpstr[32];
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x76;
	} else
	{
		cmd[0] = 0x5E;
	}
	cmd[1] = 0x00;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (!MDB_BUFFER[0].mode)
		{
			MDB_ACK();
			CHLED_ON(index);
			for (int i = 0; i < MDB_BUFFER_COUNT - 1; i++)
			{
				if (MDB_BUFFER[i].data > 0)
				{
					sprintf(tmpstr,"CH%d*PAYSTATUS*%d*", index + 1, (i / 2) + 1);
					EXT_UART_Transmit(tmpstr);
					uint8_t buff[5 + CoinHopperSetupData[index].DecimalPlaces];
					double coinvalue = (CoinHopperSetupData[index].CoinScalingFactor * CoinHopperSetupData[index].CoinTypeCredit[i / 2]) / pow(10, CoinHopperSetupData[index].DecimalPlaces);
					dtostrf(coinvalue,0,CoinHopperSetupData[index].DecimalPlaces,buff);
					sprintf(tmpstr,"*%s", buff);
					EXT_UART_Transmit(tmpstr);
					uint16_t coinsqty = MDB_BUFFER[i].data;
					coinsqty = (coinsqty << 8) | MDB_BUFFER[i + 1].data;
					sprintf(buff,"*%d", coinsqty);
					EXT_UART_Transmit(buff);
					EXT_CRLF();
					i++;
				}
			}
			GetCoinHopperDispenserStatus(index);
		} else
		{
			sprintf(tmpstr,"CH%d*PAYSTATUS*BUSY", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
	} else
	{
		sprintf(tmpstr,"CH%d*PAYSTATUS*FAIL", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void GetCoinHopperPayoutValue(uint8_t index)
{
	char tmpstr[30];
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x76;
	} else
	{
		cmd[0] = 0x5E;
	}
	cmd[1] = 0x01;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (!MDB_BUFFER[0].mode)
		{
			MDB_ACK();
			CHLED_ON(index);
			uint16_t paidvalue = MDB_BUFFER[0].data;
			paidvalue = (paidvalue << 8) | MDB_BUFFER[1].data;
			uint8_t buff[5 + CoinHopperSetupData[index].DecimalPlaces];
			double cpvalue = paidvalue / pow(10, CoinHopperSetupData[index].DecimalPlaces);
			dtostrf(cpvalue,0,CoinHopperSetupData[index].DecimalPlaces,buff);
			sprintf(tmpstr,"CH%d*PAID*%s", index + 1, buff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		} else
		{
			sprintf(tmpstr,"CH%d*PAYOUTEND", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			CoinHopperDevice[index].Status = 1;
			CoinHopperPayoutStatus(index);
		}
	} else
	{
		sprintf(tmpstr,"CH%d*PAYSTATUSFAIL", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}

void GetCoinHopperIdentification(uint8_t index)
{
	uint8_t tmpstr[64];
	if (CoinHopperSetupData[index].DispenserFeatureLevel >= 1)
	{
		uint8_t cmd[3];
		if (!index)
		{
			cmd[0] = 0x77;
		} else
		{
			cmd[0] = 0x5F;
		}
		cmd[1] = 0x00;
		cmd[2] = (cmd[0] + cmd[1]) & 0xff;
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
				CHLED_ON(index);
				for (int i = 0; i < 3; i++)
				{
					CoinHopperIDData[index].ManufacturerCode[i] = 0x00;
				}
				for (int i = 0; i < 12; i++)
				{
					CoinHopperIDData[index].SerialNumber[i] = 0x00;
				}
				for (int i = 0; i < 12; i++)
				{
					CoinHopperIDData[index].ModelRevision[i] = 0x00;
				}
				uint8_t tmpmfg[3] = {MDB_BUFFER[0].data, MDB_BUFFER[1].data, MDB_BUFFER[2].data};
				memcpy(CoinHopperIDData[index].ManufacturerCode, tmpmfg, 3);
				sprintf(tmpstr,"CH%d*ID", index + 1);
				EXT_UART_Transmit(tmpstr);
				EXT_UART_Transmit(CoinHopperIDData[index].ManufacturerCode);
				uint8_t tmpsn[12] = {MDB_BUFFER[3].data, MDB_BUFFER[4].data, MDB_BUFFER[5].data, MDB_BUFFER[6].data, MDB_BUFFER[7].data, MDB_BUFFER[8].data, MDB_BUFFER[9].data, MDB_BUFFER[10].data, MDB_BUFFER[11].data, MDB_BUFFER[12].data, MDB_BUFFER[13].data, MDB_BUFFER[14].data};
				memcpy(CoinHopperIDData[index].SerialNumber,tmpsn, 12);
				EXT_UART_Transmit("*");
				EXT_UART_Transmit(CoinHopperIDData[index].SerialNumber);
				uint8_t tmpmr[12] = {MDB_BUFFER[15].data, MDB_BUFFER[16].data, MDB_BUFFER[17].data, MDB_BUFFER[18].data, MDB_BUFFER[19].data, MDB_BUFFER[20].data, MDB_BUFFER[21].data, MDB_BUFFER[22].data, MDB_BUFFER[23].data, MDB_BUFFER[24].data, MDB_BUFFER[25].data, MDB_BUFFER[26].data};
				memcpy(CoinHopperIDData[index].ModelRevision,tmpmr, 12);
				EXT_UART_Transmit("*");
				EXT_UART_Transmit(CoinHopperIDData[index].ModelRevision);
				uint8_t srd[2] = {MDB_BUFFER[27].data, MDB_BUFFER[28].data};
				CoinHopperIDData[index].SoftwareVersion = BCDByteToInt(srd);
				uint16_t flags  = MDB_BUFFER[29].data;
				flags = (flags << 8) | MDB_BUFFER[30].data;
				flags = (flags << 8) | MDB_BUFFER[31].data;
				flags = (flags << 8) | MDB_BUFFER[32].data;
				CoinHopperIDData[index].FTLSupported = ((flags & 0x01) == 1);
				sprintf(tmpstr,"*%d*%d", CoinHopperIDData[index].SoftwareVersion, CoinHopperIDData[index].FTLSupported);
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
			} else
			{
				if (MDB_BUFFER[0].data == 0x00){
					
				}
			}
		}
	} else
	{
		sprintf(tmpstr,"CH%d*ID*FL_LOW", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CHLED_OFF(index);
	}
}