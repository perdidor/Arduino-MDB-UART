/*
 * BillValidator_M.c
 *
 * Created: 18.05.2019 10:05:37
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
#include "BillValidator_M.h"
#include "Settings_M.h"


void BillValidatorPollResponse()
{
	BillValidatorDevice.OfflinePollsCount = 10;
	uint8_t tmpstr[64];
	uint8_t bsbuff[25];
	uint8_t statusbuff[32];
	uint16_t status;
	uint8_t billtype;
	uint16_t tmplen = MDB_BUFFER_COUNT;
	MDB_Byte TMP[tmplen];
	memcpy(&TMP, &MDB_BUFFER, MDB_BUFFER_COUNT * 2);
	for (int i = 0; i < tmplen - 1; i++)
	{
		switch ((TMP[i].data & 0x80) >> 7)
		{
			case 0://Status
			switch ((TMP[i].data & 0xE0) >> 5)
			{
				case 0://Bill Validator status
				status = TMP[i].data & 0x0F;
				switch (status)
				{
					case 1:
					sprintf(statusbuff,"%s", "BADMOTOR");
					break;
					case 2:
					sprintf(statusbuff,"%s", "BADSENSOR");
					break;
					case 3:
					sprintf(statusbuff,"%s", "BUSY");
					break;
					case 4:
					sprintf(statusbuff,"%s", "ROMERROR");
					break;
					case 5:
					sprintf(statusbuff,"%s", "JAM");
					break;
					case 6:
					BillValidatorDevice.Status = 1;
					EXT_UART_Transmit("BV*STATUS*JUSTRESET");
					EXT_CRLF();
					GetBillValidatorSetupData();
					if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
					{
						BillValidatorEnableFeatures();
					}
					GetBillValidatorStackerStatus();
					if (BillValidatorIDData.BillRecyclingSupported) GetBVDispenserStatus();
					//
					return;
					case 7:
					sprintf(statusbuff,"%s", "BILLREMOVED");
					break;
					case 8:
					sprintf(statusbuff,"%s", "CBOXOUT");
					break;
					case 9:
					sprintf(statusbuff,"%s", "DISABLED");
					break;
					case 10:
					sprintf(statusbuff,"%s", "INVESCROW");
					break;
					case 11:
					sprintf(statusbuff,"%s", "REJECT");
					break;
					case 12:
					sprintf(statusbuff,"%s", "FISHING");
					break;
				}
				EXT_UART_Transmit("BV*STATUS*");
				EXT_UART_Transmit(statusbuff);
				EXT_CRLF();
				break;
				case 1://Bill Recycler status
				status = TMP[i].data & 0x0F;
				switch (status)
				{
					case 1:
					sprintf(statusbuff,"%s", "ESCROWREQ");
					break;
					case 2:
					sprintf(statusbuff,"%s", "PAYOUTBUSY");
					break;
					case 3:
					sprintf(statusbuff,"%s", "BUSY");
					break;
					case 4:
					sprintf(statusbuff,"%s", "BADSENSOR");
					break;
					case 6:
					sprintf(statusbuff,"%s", "BADMOTOR");
					break;
					case 7:
					sprintf(statusbuff,"%s", "JAM");
					break;
					case 8:
					sprintf(statusbuff,"%s", "ROMERROR");
					break;
					case 9:
					sprintf(statusbuff,"%s", "DISABLED");
					break;
					case 10:
					sprintf(statusbuff,"%s", "BILLWAIT");
					BillValidatorDevice.Status = 2;
					break;
					case 15:
					sprintf(statusbuff,"%s", "FILLEDKEY");
					GetBVDispenserStatus();
					break;
					default:
					sprintf(statusbuff,"%s", "DISPUNK");
					break;
				}
				EXT_UART_Transmit("BV*DISPSTATUS*");
				EXT_UART_Transmit(statusbuff);
				EXT_CRLF();
				break;
				case 2://Number of attempts to input a bill while validator is disabled
				status = (TMP[i].data & 0x1F);//bits 5-7 of byte 1
				sprintf(tmpstr,"BV*ATTEMPTS*%d", status);
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
				break;
			}
			break;
			case 1://Bills Accepted
			{
				uint16_t routedata = (((TMP[i].data) & 0x70) >> 4);//bits 5-7 of byte 1
				uint8_t route[8];
				switch (routedata)
				{
					case 0:
					sprintf(route,"%s", "STACKER");
					break;
					case 1:
					sprintf(route,"%s", "ESCROW");
					break;
					case 2:
					sprintf(route,"%s", "RETURN");
					break;
					case 3:
					sprintf(route,"%s", "RECYCLER");
					break;
					case 4:
					sprintf(route,"%s", "DISREJECT");
					break;
					case 5:
					sprintf(route,"%s", "RECMANUAL");
					break;
					case 6:
					sprintf(route,"%s", "DISPMANUAL");
					break;
					case 7:
					sprintf(route,"%s", "REC2CB");
					break;
				}
				billtype = TMP[i].data & 0x0f;
				uint8_t buff[7 + BillValidatorSetupData.DecimalPlaces];
				double billvalue = BillValidatorSetupData.BillScalingFactor * (BillValidatorSetupData.BillTypeCredit[billtype] / pow(10, BillValidatorSetupData.DecimalPlaces));
				dtostrf(billvalue,0,BillValidatorSetupData.DecimalPlaces,buff);
				sprintf(bsbuff,"BV*BILLACTION*%d*%s*%s", billtype + 1, buff, route);
				EXT_UART_Transmit(bsbuff);
				EXT_CRLF();
				GetBillValidatorStackerStatus();
			}
			break;
		}
	}
}

void GetBillValidatorIdentification()
{
	uint8_t tmpstr[64];
	uint8_t cmd[3] = {0x00, 0x00, 0x00};
	if (BillValidatorSetupData.BillValidatorFeatureLevel == 1)
	{
		cmd[0] = 0x37;
		cmd[1] = 0x00;
		cmd[2] = 0x37;
	} else if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
	{
		cmd[0] = 0x37;
		cmd[1] = 0x02;
		cmd[2] = 0x39;
		} else {
		EXT_UART_Transmit("BV*ID*LEVEL_LOW");
		EXT_CRLF();
		return;
	}
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
			BillValidatorDevice.OfflinePollsCount = 10;
			for (int i = 0; i < 3; i++)
			{
				BillValidatorIDData.ManufacturerCode[i] = 0x00;
			}
			for (int i = 0; i < 12; i++)
			{
				BillValidatorIDData.SerialNumber[i] = 0x00;
			}
			for (int i = 0; i < 12; i++)
			{
				BillValidatorIDData.ModelRevision[i] = 0x00;
			}
			uint8_t tmpmfg[3] = {MDB_BUFFER[0].data, MDB_BUFFER[1].data, MDB_BUFFER[2].data};
			memcpy(BillValidatorIDData.ManufacturerCode, &tmpmfg, 3);
			EXT_UART_Transmit("BV*ID*");
			EXT_UART_Transmit(BillValidatorIDData.ManufacturerCode);
			uint8_t tmpsn[12] = {MDB_BUFFER[3].data, MDB_BUFFER[4].data, MDB_BUFFER[5].data, MDB_BUFFER[6].data, MDB_BUFFER[7].data, MDB_BUFFER[8].data, MDB_BUFFER[9].data, MDB_BUFFER[10].data, MDB_BUFFER[11].data, MDB_BUFFER[12].data, MDB_BUFFER[13].data, MDB_BUFFER[14].data};
			memcpy(BillValidatorIDData.SerialNumber,&tmpsn, 12);
			EXT_UART_Transmit("*");
			EXT_UART_Transmit(BillValidatorIDData.SerialNumber);
			uint8_t tmpmr[12] = {MDB_BUFFER[15].data, MDB_BUFFER[16].data, MDB_BUFFER[17].data, MDB_BUFFER[18].data, MDB_BUFFER[19].data, MDB_BUFFER[20].data, MDB_BUFFER[21].data, MDB_BUFFER[22].data, MDB_BUFFER[23].data, MDB_BUFFER[24].data, MDB_BUFFER[25].data, MDB_BUFFER[26].data};
			memcpy(BillValidatorIDData.ModelRevision, &tmpmr, 12);
			EXT_UART_Transmit("*");
			EXT_UART_Transmit(BillValidatorIDData.ModelRevision);
			uint8_t srd[2] = {MDB_BUFFER[27].data, MDB_BUFFER[28].data};
			BillValidatorIDData.SoftwareVersion = BCDByteToInt(srd);
			if (BillValidatorSetupData.BillValidatorFeatureLevel == 2 && MDB_BUFFER_COUNT == 33)
			{
				uint16_t flags  = MDB_BUFFER[29].data;
				flags = (flags << 8) | MDB_BUFFER[30].data;
				flags = (flags << 8) | MDB_BUFFER[31].data;
				flags = (flags << 8) | MDB_BUFFER[32].data;
				BillValidatorIDData.FTLSupported = ((flags & (1 << 0)) != 0);
				BillValidatorIDData.BillRecyclingSupported = ((flags & (1 << 1)) != 0);
			}
			sprintf(tmpstr,"*%d*%d*%d", BillValidatorIDData.SoftwareVersion, BillValidatorIDData.BillRecyclingSupported, BillValidatorIDData.FTLSupported);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		} else
		{
			if (MDB_BUFFER[0].data == 0x00)
			{
				
			}
		}
	} else
	{
		EXT_UART_Transmit("BV*CFG4*ERR");
		EXT_CRLF();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void BillValidatorEnableFeatures()
{
	EXT_UART_Transmit("BV*FEATENABLE*");
	uint8_t cmd[7];
	cmd[0] = 0x37;
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = 0x00 | (BillValidatorOptions.EnableBillRecycling << 1);
	cmd[6] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff);
	MDB_Send(cmd, 7);
	while (!MDBReceiveComplete)
	{
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			//BillValidatorIDData.BillRecyclingSupported = 1;
			//BillValidatorIDData.FTLSupported = 1;
			return;
		}
	}
	EXT_UART_FAIL();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void GetBillRecyclerSetupData()
{
	uint8_t cmd[3];
	cmd[0] = 0x37;
	cmd[1] = 0x03;
	cmd[2] = 0x3a;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 3)
		{
			uint16_t tmpcr  = MDB_BUFFER[0].data;
			tmpcr = (tmpcr << 8) | MDB_BUFFER[1].data;
			for (int i = 0; i < 16; i++)
			{
				BillValidatorSetupData.BillRecycleEnabled[i] = ((tmpcr >> i) & 1);
			}
			return;
		}
	}
	EXT_UART_Transmit("BV*RECYCLECFG*FAIL");
	EXT_CRLF();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void BillValidatorRecyclerEnable()
{
	uint8_t cmd[21];
	cmd[0] = 0x37;
	cmd[1] = 0x04;
	cmd[2] = (BillValidatorOptions.EnableManualDispenseBillsBits >> 8) & 0xff;
	cmd[3] = BillValidatorOptions.EnableManualDispenseBillsBits & 0xff;
	for (int i = 4; i < 20; i++)
	{
		cmd[i] = ((BillValidatorOptions.EnableRecycleBillsBits >> (i - 4)) & 1 == 1) ? 0x03 : 0x00;
		//Use all possible bills (this is the recommended setting –
		//the recycler will use its internal setting to determine what bill
		//are put into the recycler)
	}
	uint16_t sum = 0;
	for (int i = 0; i < 20; i++)
	{
		sum += cmd[i];
	}
	cmd[20] = sum & 0xff;
	MDB_Send(cmd, 21);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*RECYCLENABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			//BillValidatorIDData.BillRecyclingSupported = 1;
			//BillValidatorIDData.FTLSupported = 1;
			return;
		}
	}
	EXT_UART_FAIL();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void BillValidatorRecyclerDisable()
{
	uint8_t cmd[21];
	cmd[0] = 0x37;
	cmd[1] = 0x04;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	for (int i = 4; i < 20; i++)
	{
		cmd[i] = 0x00;
		//Use all possible bills (this is the recommended setting –
		//the recycler will use its internal setting to determine what bill
		//are put into the recycler)
	}
	uint16_t sum = 0;
	for (int i = 0; i < 20; i++)
	{
		sum += cmd[i];
	}
	cmd[20] = sum & 0xff;
	MDB_Send(cmd, 21);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*RECYCLEDISABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			//BillValidatorIDData.BillRecyclingSupported = 1;
			//BillValidatorIDData.FTLSupported = 1;
			return;
		}
	}
	EXT_UART_FAIL();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void GetBVDispenserStatus()
{
	uint8_t cmd[3] = {0x37, 0x05, 0x3c};
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete)
	{
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			BillValidatorDevice.OfflinePollsCount = 10;
			uint16_t fullflags  = MDB_BUFFER[0].data;
			fullflags = (fullflags << 8) | MDB_BUFFER[1].data;
			for (int i = 2; i < MDB_BUFFER_COUNT - 1; i++)
			{
				uint8_t tmpstr[64];
				uint16_t billtypecount = MDB_BUFFER[i].data;
				billtypecount = (billtypecount << 8) | MDB_BUFFER[i + 1].data;
				uint8_t billtype = (i - 2) / 2;
				uint8_t buff[6 + BillValidatorSetupData.DecimalPlaces];
				double billvalue = BillValidatorSetupData.BillScalingFactor * (BillValidatorSetupData.BillTypeCredit[billtype] / pow(10, BillValidatorSetupData.DecimalPlaces));
				dtostrf(billvalue,0,BillValidatorSetupData.DecimalPlaces,buff);
				sprintf(tmpstr,"BV*DSTATUS*%d*%s*%d*%d", billtype + 1, buff, billtypecount, fullflags & (1 << ((i - 2))/2));
				if (billtypecount == 0 && fullflags & (1 << ((i - 2)/2)) == 1) EXT_UART_Transmit("*ERR");
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
				i++;
			}
		}
		} else {
		EXT_UART_Transmit("CC*DSTATUS*FAIL");
		EXT_CRLF();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void BVDispenseBills(uint8_t BillType, uint16_t Number)
{
	if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
	{
		if (BillValidatorDevice.Status == 2)
		{
			EXT_UART_Transmit("BV*DISPBILL*BILLWAIT");
			EXT_CRLF();
			return;
		}
		uint8_t cmd[6];
		cmd[0] = 0x37;
		cmd[1] = 0x06;
		cmd[2] = BillType;
		cmd[3] = Number >> 8;
		cmd[4] = Number & 0xff;
		cmd[5] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff;
		MDB_Send(cmd, 6);
		while (!MDBReceiveComplete){
			MDB_read();
		}
		EXT_UART_Transmit("BV*DISPBILL*");
		if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
		{
			BillValidatorDevice.OfflinePollsCount = 10;
			switch (MDB_BUFFER[0].data)
			{
				case 0x00:
				BillValidatorDevice.Status = 3;
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
			EXT_UART_FAIL();
			if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
		}
	} else
	{
		EXT_UART_Transmit("BV*DISPBILL*FL_LOW");
		EXT_CRLF();
	}
}

void BVDispenseValue(uint16_t PayoutValue)
{
	if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
	{
		if (BillValidatorDevice.Status == 2)
		{
			EXT_UART_Transmit("BV*DISPVALUE*BILLWAIT");
			EXT_CRLF();
			return;
		}
		uint8_t cmd[5];
		cmd[0] = 0x37;
		cmd[1] = 0x07;
		cmd[2] = (PayoutValue >> 8) & 0xff;
		cmd[3] = PayoutValue & 0xff;
		cmd[4] = (cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xff;
		MDB_Send(cmd, 5);
		while (!MDBReceiveComplete){
			MDB_read();
		}
		EXT_UART_Transmit("BV*DISPVALUE*");
		if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
		{
			BillValidatorDevice.OfflinePollsCount = 10;
			switch (MDB_BUFFER[0].data)
			{
				case 0x00:
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
			EXT_UART_FAIL();
			if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
		}
	} else
	{
		EXT_UART_Transmit("BV*DISPVALUE*FL_LOW");
		EXT_CRLF();
	}
}

void BillValidatorPayoutStatus()
{
	if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
	{
		uint8_t cmd[3];
		cmd[0] = 0x37;
		cmd[1] = 0x08;
		cmd[2] = 0x3f;
		MDB_Send(cmd, 3);
		while (!MDBReceiveComplete){
			MDB_read();
		}
		if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
		{
			if (!MDB_BUFFER[0].mode)
			{
				MDB_ACK();
				BillValidatorDevice.OfflinePollsCount = 10;
				uint8_t * buff[6];
				for (int i = 0; i < MDB_BUFFER_COUNT - 1; i++)
				{
					if (MDB_BUFFER[i].data > 0)
					{
						uint8_t tmpstr[64];
						uint16_t billtypecount = MDB_BUFFER[i].data;
						billtypecount = (billtypecount << 8) | MDB_BUFFER[i + 1].data;
						uint8_t billtype = (i - 2) / 2;
						uint8_t buff[6 + BillValidatorSetupData.DecimalPlaces];
						double billvalue = BillValidatorSetupData.BillScalingFactor * (BillValidatorSetupData.BillTypeCredit[billtype] / pow(10, BillValidatorSetupData.DecimalPlaces));
						dtostrf(billvalue,0,BillValidatorSetupData.DecimalPlaces,buff);
						sprintf(tmpstr,"BV*DPS*%d*%s*%d", billtype +1, buff, billtypecount);
						EXT_UART_Transmit(tmpstr);
						EXT_CRLF();
					}
					i++;
				}
				GetBVDispenserStatus();
			} else
			{
				EXT_UART_Transmit("BV*DPS*BUSY");
				EXT_CRLF();
			}
		} else
		{
			EXT_UART_Transmit("BV*DPS*FAIL");
			EXT_CRLF();
			if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
		}
	} else
	{
		EXT_UART_Transmit("BV*DPS*FL_LOW");
		EXT_CRLF();
	}
}

void BillValidatorPayoutValue()
{
	if (BillValidatorSetupData.BillValidatorFeatureLevel >= 2)
	{
		uint8_t cmd[3];
		cmd[0] = 0x37;
		cmd[1] = 0x09;
		cmd[2] = 0x40;
		MDB_Send(cmd, 3);
		while (!MDBReceiveComplete){
			MDB_read();
		}
		if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
		{
			if (!MDB_BUFFER[0].mode)
			{
				MDB_ACK();
				BillValidatorDevice.OfflinePollsCount = 10;
				uint8_t tmpstr[64];
				uint16_t billvalue = MDB_BUFFER[0].data;
				billvalue = (billvalue << 8) | MDB_BUFFER[1].data;
				uint8_t buff[6 + BillValidatorSetupData.DecimalPlaces];
				double billpvalue = BillValidatorSetupData.BillScalingFactor * (billvalue / pow(10, BillValidatorSetupData.DecimalPlaces));
				dtostrf(billpvalue,0,BillValidatorSetupData.DecimalPlaces,buff);
				sprintf(tmpstr,"BV*DPV*%s", buff);
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
			} else
			{
				if (MDB_BUFFER[0].data == 0x00)
				{
					EXT_UART_Transmit("BV*DPVFIN");
					EXT_CRLF();
					BillValidatorDevice.Status = 1;
					BillValidatorPayoutStatus();
				} else
				{
					EXT_UART_Transmit("BV*DPVUNK");
					EXT_CRLF();
					BillValidatorDevice.Status = 1;
				}
			}
		} else
		{
			EXT_UART_Transmit("BV*DPV*FAIL");
			EXT_CRLF();
			if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
		}
	} else
	{
		EXT_UART_Transmit("BV*DPV*FL_LOW");
		EXT_CRLF();
	}
}

void BillValidatorEscrow(uint8_t action)
{
	uint8_t cmd[3];
	cmd[0] = 0x35;
	cmd[1] = action;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*ESC*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
		{
			EXT_UART_OK();
		} else
		{
			EXT_UART_FAIL();
		}
	} else
	{
		EXT_UART_FAIL();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void BillValidatorCancelPayout()
{
	uint8_t cmd[3];
	cmd[0] = 0x37;
	cmd[1] = 0x0a;
	cmd[2] = 0x41;
	MDB_Send(cmd, 3);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*DPC*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
		{
			EXT_UART_OK();
		} else
		{
			EXT_UART_FAIL();
		}
	} else
	{
		EXT_UART_FAIL();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void GetBillValidatorSetupData()
{
	uint8_t cmd[2] = {0x31, 0x31};
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			BillValidatorDevice.OfflinePollsCount = 10;
			BillValidatorSetupData.BillValidatorFeatureLevel = MDB_BUFFER[0].data;
			uint8_t cocd[2] = {MDB_BUFFER[1].data, MDB_BUFFER[2].data};
			BillValidatorSetupData.CountryOrCurrencyCode = BCDByteToInt(cocd);
			BillValidatorSetupData.BillScalingFactor = MDB_BUFFER[3].data;
			BillValidatorSetupData.BillScalingFactor = (BillValidatorSetupData.BillScalingFactor << 8) | MDB_BUFFER[4].data;
			BillValidatorSetupData.DecimalPlaces = MDB_BUFFER[5].data;
			BillValidatorSetupData.StackerCapacity = (MDB_BUFFER[6].data << MDB_BUFFER[7].data) | MDB_BUFFER[7].data;
			uint16_t tmpcr  = MDB_BUFFER[8].data;
			tmpcr = (tmpcr << 8) | MDB_BUFFER[9].data;
			for (int i = 0; i < 16; i++)
			{
				BillValidatorSetupData.BillSecurityLevel[i] = ((tmpcr & (1 << i)) != 0);
			}
			BillValidatorSetupData.Escrow = (MDB_BUFFER[10].data == 0xff);
			for (int i = 11; i < MDB_BUFFER_COUNT - 1; i++)
			{
				BillValidatorSetupData.BillTypeCredit[i - 11] = MDB_BUFFER[i].data;
			}
			uint8_t tmpstr[64];
			uint8_t mbvbuff[6 + BillValidatorSetupData.DecimalPlaces];
			double mindispvalue = BillValidatorSetupData.BillScalingFactor / pow(10, BillValidatorSetupData.DecimalPlaces);
			dtostrf(mindispvalue,0,BillValidatorSetupData.DecimalPlaces,mbvbuff);
			sprintf(tmpstr,"BV*CFG*%d*%d*%s*%d*%d*%d", BillValidatorSetupData.BillValidatorFeatureLevel, BillValidatorSetupData.CountryOrCurrencyCode, mbvbuff, BillValidatorSetupData.DecimalPlaces, BillValidatorSetupData.StackerCapacity, BillValidatorSetupData.Escrow);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			GetBillValidatorIdentification();
			if (BillValidatorIDData.BillRecyclingSupported == 1) GetBillRecyclerSetupData();
			for (int i = 0; i < 16; i++)
			{
				//if (BillValidatorSetupData.BillSecurityLevel[i] == 1)
				if (BillValidatorSetupData.BillTypeCredit[i] > 0)
				{
					uint8_t bvbuff[6 + BillValidatorSetupData.DecimalPlaces];
					uint8_t buff[29 + sizeof(bvbuff)];
					double billvalue = BillValidatorSetupData.BillScalingFactor * (BillValidatorSetupData.BillTypeCredit[i] / pow(10, BillValidatorSetupData.DecimalPlaces));
					dtostrf(billvalue,0,BillValidatorSetupData.DecimalPlaces,bvbuff);
					sprintf(buff,"BV*BILLSUP*%d*%s*%d*%d*%d*%d*%d*%d\r\n",i + 1,bvbuff,BillValidatorSetupData.BillRecycleEnabled[i],((BillValidatorOptions.EnableAcceptBillsBits >> i) & 1),((BillValidatorOptions.EnableEscrowBillsBits >> i) & 1),((BillValidatorOptions.EnableBillRecycling >> i) & 1),((BillValidatorOptions.EnableManualDispenseBillsBits >> i) & 1),BillValidatorSetupData.BillSecurityLevel[i]);
					EXT_UART_Transmit(buff);
				}
			}
			} else {
			if (MDB_BUFFER[0].data == 0x00){
				
			}
		}
		} else {
		for (int i = 0; i < 3; i++)
		{
			uint8_t buff[20];
			sprintf(buff,"BV*CFG*ERR",i + 1);
			EXT_UART_Transmit(buff);
			EXT_CRLF();
			if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
		}
	}
}

void BillValidatorSetSecurityLevels()
{
	uint8_t cmd[4];
	cmd[0] = 0x32;
	cmd[1] = (BillValidatorOptions.BillSecurityBits >> 8) & 0xff;
	cmd[2] = BillValidatorOptions.BillSecurityBits & 0xff;
	cmd[3] = (cmd[0] + cmd[1] + cmd[2]) & 0xff;
	MDB_Send(cmd, 4);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*BILLSEC*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
		{
			EXT_UART_OK();
		} else
		{
			EXT_UART_FAIL();
		}
	} else
	{
		EXT_UART_FAIL();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void GetBillValidatorStackerStatus()
{
	uint8_t cmd[2] = {0x36, 0x36};
	MDB_Send(cmd, 2);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT == 3)
		{
			MDB_ACK();
			BillValidatorDevice.OfflinePollsCount = 10;
			uint8_t tmpstr[32];
			uint8_t full = ((MDB_BUFFER[0].data & 0x80) == 1);
			uint16_t billnumber = ((MDB_BUFFER[0].data & 0x7F) << 8) | MDB_BUFFER[1].data;
			sprintf(tmpstr,"BV*STACKER*%d*%d", billnumber, full);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			if (full)
			{
				BillValidatorDisableAcceptBills();
			}
			} else {
			if (MDB_BUFFER[0].data == 0x00){
				EXT_UART_Transmit("BV*STACKER*NAK");
				EXT_CRLF();
			}
		}
		} else {
		EXT_UART_Transmit("BV*STK*ERR");
		EXT_CRLF();
		if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
	}
}

void BillValidatorEnableBillType(uint8_t BillType, uint8_t EnableAccept, uint8_t EnableEscrow, uint8_t EnableRecycle, uint8_t EnableManualDispense, uint8_t HighSecurityLevel)
{
	uint8_t buff[36];
	BillValidatorOptions.BillSecurityBits = (HighSecurityLevel == 1) ? (BillValidatorOptions.BillSecurityBits | (1 << (BillType - 1))) : (BillValidatorOptions.BillSecurityBits & ~(1 << (BillType - 1)));
	BillValidatorOptions.EnableAcceptBillsBits = (EnableAccept == 1) ? (BillValidatorOptions.EnableAcceptBillsBits | (1 << (BillType - 1))) : (BillValidatorOptions.EnableAcceptBillsBits & ~(1 << (BillType - 1)));
	BillValidatorOptions.EnableEscrowBillsBits = (EnableEscrow == 1) ? (BillValidatorOptions.EnableEscrowBillsBits | (1 << (BillType - 1))) : (BillValidatorOptions.EnableEscrowBillsBits & ~(1 << (BillType - 1)));
	BillValidatorOptions.EnableRecycleBillsBits = (EnableRecycle == 1) ? (BillValidatorOptions.EnableRecycleBillsBits | (1 << (BillType - 1))) : (BillValidatorOptions.EnableRecycleBillsBits & ~(1 << (BillType - 1)));
	BillValidatorOptions.EnableManualDispenseBillsBits = (EnableManualDispense == 1) ? (BillValidatorOptions.EnableManualDispenseBillsBits | (1 << (BillType - 1))) : (BillValidatorOptions.EnableManualDispenseBillsBits & ~(1 << (BillType - 1)));
	WriteBVOptions();
	uint8_t bvbuff[7 + BillValidatorSetupData.DecimalPlaces];
	double billvalue = BillValidatorSetupData.BillScalingFactor * (BillValidatorSetupData.BillTypeCredit[BillType - 1] / pow(10, BillValidatorSetupData.DecimalPlaces));
	dtostrf(billvalue,0,BillValidatorSetupData.DecimalPlaces,bvbuff);
	sprintf(buff,"BV*BILLCFG*%d*%s*%d*%d*%d*%d*%d*", BillType, bvbuff, (EnableAccept == 1), (EnableEscrow == 1), (EnableRecycle == 1), (EnableManualDispense == 1), (HighSecurityLevel == 1));
	EXT_UART_Transmit(buff);
	EXT_UART_OK();
}

void BillValidatorEnableAcceptBills()
{
	uint8_t cmd[6];
	cmd[0] = 0x34;
	cmd[1] = (BillValidatorOptions.EnableAcceptBillsBits >> 8) & 0xff;
	cmd[2] = BillValidatorOptions.EnableAcceptBillsBits & 0xff;
	cmd[3] = (BillValidatorOptions.EnableEscrowBillsBits >> 8) & 0xff;
	cmd[4] = BillValidatorOptions.EnableEscrowBillsBits & 0xff;
	cmd[5] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff);
	MDB_Send(cmd, 6);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*ENABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			if (BillValidatorOptions.EnableBillRecycling && BillValidatorIDData.BillRecyclingSupported) BillValidatorRecyclerEnable();
			return;
		}
	}
	EXT_UART_FAIL();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void BillValidatorDisableAcceptBills()
{
	uint8_t cmd[6];
	cmd[0] = 0x34;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = ((cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xff);
	MDB_Send(cmd, 6);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	EXT_UART_Transmit("BV*DISABLE*");
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		BillValidatorDevice.OfflinePollsCount = 10;
		if (MDB_BUFFER_COUNT == 1 && MDB_BUFFER[0].data == 0x00)
		{
			EXT_UART_OK();
			if (BillValidatorOptions.EnableBillRecycling && BillValidatorIDData.BillRecyclingSupported) BillValidatorRecyclerDisable();
			return;
		}
	}
	EXT_UART_FAIL();
	if (BillValidatorDevice.OfflinePollsCount > 0) BillValidatorDevice.OfflinePollsCount--;
}

void BillValidatorConfigFeatures(uint8_t RecyclerEnable)
{
	BillValidatorOptions.EnableBillRecycling = (RecyclerEnable == 1);
	WriteBVOptions();
}