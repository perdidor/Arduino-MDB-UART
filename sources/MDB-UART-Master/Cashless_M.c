/*
 * Cashless_M.c
 *
 * Created: 18.05.2019 10:07:59
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
#include "Settings_M.h"
#include "USART_M.h"
#include "Cashless_M.h"


void CashlessDeviceSetup(uint8_t index)
{
	uint8_t tmpstr[80];
	uint8_t cmd[7];
	if (!index)
	{
		cmd[0] = 0x11;
	} else
	{
		cmd[0] = 0x61;
	}
	cmd[1] = 0x00;
	cmd[2] = VMCData.VMC_FEATURE_LEVEL;
	cmd[3] = VMCData.VMC_DISPLAY_COLUMNS;
	cmd[4] = VMCData.VMC_DISPLAY_ROWS;
	cmd[5] = VMCData.VMC_DISPLAY_TYPE;
	cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
	MDB_Send(cmd, 7);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	//uint8_t * buff[6];
	//sprintf(buff, "%d\r\n", MDB_BUFFER_COUNT);
	//EXT_UART_Transmit(buff);
	//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
	//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
	//EXT_UART_Transmit(buff);
	//}
	//sprintf(&buff, "%02x ", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
	//EXT_UART_Transmit(buff);
	//EXT_CRLF();
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CDLED_ON(index);
			ProcessReaderConfig(index, 0);
			if (ReaderIDData[index].Monetary32bitSupported || ReaderIDData[index].MultiCurrencySupported)
			{
				
			}
		}
		if (MDB_BUFFER_COUNT == 1)
		{
			sprintf(tmpstr,"CD%d*CONFIG*", index + 1);
			EXT_UART_Transmit(tmpstr);
			if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
			{
				EXT_UART_OK();
			} else
			{
				EXT_UART_NAK();
			}
			//return;
		}
	} else
	{
		sprintf(tmpstr,"CD%d*CONFIG*", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_UART_FAIL();
		CDLED_OFF(index);
	}
	//ProcessReaderConfig();
	//ReaderProcessResponse(index, "CONFIG");
}

void CashlessDeviceSetupPrices16bit(uint8_t index)
{
	uint8_t tmpstr[64];
	uint8_t cmd[7];
	cmd[0] = (index) ? 0x61 : 0x11;
	cmd[1] = 0x01;
	cmd[2] = ReaderOptions[index].MaxPrice.Bytes[0];
	cmd[3] = ReaderOptions[index].MaxPrice.Bytes[1];
	cmd[4] = ReaderOptions[index].MinPrice.Bytes[0];
	cmd[5] = ReaderOptions[index].MinPrice.Bytes[1];
	cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
	MDB_Send(cmd, 7);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CDLED_ON(index);
		if (MDB_BUFFER_COUNT == 1)
		{
			sprintf(tmpstr,"CD%d*CFGPRICE1*", index + 1);
			EXT_UART_Transmit(tmpstr);
			if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
			{
				EXT_UART_OK();
			} else
			{
				EXT_UART_NAK();
			}
		}
	} else
	{
		sprintf(tmpstr,"CD%d*CFGPRICE1*", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_UART_FAIL();
		CDLED_OFF(index);
	}
}

void CashlessDeviceSetupPrices32bit(uint8_t index)
{
	uint8_t tmpstr[64];
	uint8_t cmd[13];
	if (!index)
	{
		cmd[0] = 0x11;
	} else
	{
		cmd[0] = 0x61;
	}
	cmd[1] = 0x01;
	cmd[2] = ReaderOptions[index].MaxPrice.Bytes[0];
	cmd[3] = ReaderOptions[index].MaxPrice.Bytes[1];
	cmd[4] = ReaderOptions[index].MaxPrice.Bytes[2];
	cmd[5] = ReaderOptions[index].MaxPrice.Bytes[3];
	cmd[6] = ReaderOptions[index].MinPrice.Bytes[0];
	cmd[7] = ReaderOptions[index].MinPrice.Bytes[1];
	cmd[8] = ReaderOptions[index].MinPrice.Bytes[2];
	cmd[9] = ReaderOptions[index].MinPrice.Bytes[3];
	cmd[8] = ReaderOptions[index].CountryOrCurrencyCode[0];
	cmd[9] = ReaderOptions[index].CountryOrCurrencyCode[1];
	cmd[12] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7] + cmd[8] + cmd[9] + cmd[10] + cmd[11]) & 0xff;
	MDB_Send(cmd, 13);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		CDLED_ON(index);
		if (MDB_BUFFER_COUNT == 1)
		{
			sprintf(tmpstr,"CD%d*CFGPRICE2*", index + 1);
			EXT_UART_Transmit(tmpstr);
			if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
			{
				EXT_UART_OK();
			} else
			{
				EXT_UART_NAK();
			}
		}
	} else
	{
		sprintf(tmpstr,"CD%d*CFGPRICE2*", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_UART_FAIL();
		CDLED_OFF(index);
	}
}

void CashlessDeviceRequestExpansionID(uint8_t index)
{
	uint8_t tmpstr[64];
	uint8_t cmd[32];
	cmd[0] = (index) ? 0x67 : 0x17;
	cmd[1] = 0x00;
	uint16_t mdbsum = cmd[0] + cmd[1];
	for (int i = 2; i < 5; i++)
	{
		cmd[i] = VMCData.VMCMfgCode[i - 2];
		mdbsum += cmd[i];
	}
	for (int i = 5; i < 17; i++)
	{
		cmd[i] = VMCData.VMCSerialNumber[i - 5];
		mdbsum += cmd[i];
	}
	for (int i = 17; i < 29; i++)
	{
		cmd[i] = VMCData.VMCModelNumber[i - 17];
		mdbsum += cmd[i];
	}
	for (int i = 29; i < 31; i++)
	{
		cmd[i] = VMCData.VMCSofwareVersion[i - 29];
		mdbsum += cmd[i];
	}
	cmd[31] = mdbsum & 0xff;
	MDB_Send(cmd, 32);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			CDLED_ON(index);
			ProcessReaderExpID(index, 0);
			//return;
			
		}
		if (MDB_BUFFER_COUNT == 1)
		{
			sprintf(tmpstr,"CD%d*EXPIDREQ*", index + 1);
			EXT_UART_Transmit(tmpstr);
			if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
			{
				EXT_UART_OK();
			} else
			{
				EXT_UART_NAK();
			}
		}
	} else
	{
		sprintf(tmpstr,"CD%d*EXPIDREQ*", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_UART_FAIL();
		CDLED_OFF(index);
	}
}

void CashlessDeviceEnableOptFetures(uint8_t index)
{
	uint8_t tmpstr[64];
	if (ReaderSetupData[index].ReaderFeatureLevel == 3)
	{
		uint8_t cmd[7];
		cmd[0] = (index) ? 0x67 : 0x17;
		cmd[1] = 0x04;
		cmd[2] = 0x00;
		cmd[3] = 0x00;
		cmd[4] = 0x00;
		//Enable all supported features
		cmd[5] = 0x00 | ((((ReaderIDData[index].FTLSupported & ReaderOptions[index].ReaderOptFeatures.FTLEnabled) == 1) << 0) | (((ReaderIDData[index].Monetary32bitSupported & ReaderOptions[index].ReaderOptFeatures.MonetaryFormat32bitEnabled) == 1) << 1) | (((ReaderIDData[index].MultiCurrencySupported & ReaderOptions[index].ReaderOptFeatures.MultiCurrEnabled) == 1) << 2) |
		 (((ReaderIDData[index].NVendSupported & ReaderOptions[index].ReaderOptFeatures.NegVendEnabled) == 1) << 3) | (((ReaderIDData[index].DataEntrySupported & ReaderOptions[index].ReaderOptFeatures.DataEntryEnabled) == 1) << 4) | (((ReaderIDData[index].AlwaysIdleSupported & ReaderOptions[index].ReaderOptFeatures.AlwaysIdleEnabled) == 1) << 5));
		//cmd[5] = 0x38;
		cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
		MDB_Send(cmd, 7);
		while (!MDBReceiveComplete){
			MDB_read();
		}
		if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
		{
			CDLED_ON(index);
			if (MDB_BUFFER_COUNT == 1)
			{
				sprintf(tmpstr,"CD%d*ENFEAT*", index + 1);
				EXT_UART_Transmit(tmpstr);
				if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
				{
					EXT_UART_OK();
				} else
				{
					EXT_UART_NAK();
				}
			}
		} else
		{
			sprintf(tmpstr,"CD%d*ENFEAT*", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_UART_FAIL();
		}
	} else
	{
		uint8_t tmpstr[32];
		sprintf(tmpstr,"CD%d*ENFEAT*FL_LOW", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_CRLF();
		CDLED_OFF(index);
	}
}

void ProcessReaderExpID(uint8_t index, uint8_t startindex)
{
	//uint8_t * buff[6];
	//sprintf(buff, "%d\r\n", MDB_BUFFER_COUNT);
	//EXT_UART_Transmit(buff);
	//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
	//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
	//EXT_UART_Transmit(buff);
	//}
	//sprintf(&buff, "%02x ", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
	//EXT_UART_Transmit(buff);
	//EXT_CRLF();
	//return;
	uint8_t tmpstr[32];
	ReaderIDData[index].ManufacturerCode[0] = 0x00;
	ReaderIDData[index].SerialNumber[0] = 0x00;
	ReaderIDData[index].ModelRevision[0] = 0x00;
	uint8_t tmpmfg[3] = {MDB_BUFFER[1].data, MDB_BUFFER[2].data, MDB_BUFFER[3].data};
	memcpy(&ReaderIDData[index].ManufacturerCode, &tmpmfg, 3);
	sprintf(tmpstr,"CD%d*CFG2*", index + 1);
	EXT_UART_Transmit(tmpstr);
	EXT_UART_Transmit(ReaderIDData[index].ManufacturerCode);
	uint8_t tmpsn[12] = {MDB_BUFFER[4].data, MDB_BUFFER[5].data, MDB_BUFFER[6].data, MDB_BUFFER[7].data, MDB_BUFFER[8].data, MDB_BUFFER[9].data, MDB_BUFFER[10].data, MDB_BUFFER[11].data, MDB_BUFFER[12].data, MDB_BUFFER[13].data, MDB_BUFFER[14].data, MDB_BUFFER[15].data};
	memcpy(&ReaderIDData[index].SerialNumber,&tmpsn, 12);
	EXT_UART_Transmit("*");
	EXT_UART_Transmit(ReaderIDData[index].SerialNumber);
	uint8_t tmpmr[12] = {MDB_BUFFER[16].data, MDB_BUFFER[17].data, MDB_BUFFER[18].data, MDB_BUFFER[19].data, MDB_BUFFER[20].data, MDB_BUFFER[21].data, MDB_BUFFER[22].data, MDB_BUFFER[23].data, MDB_BUFFER[24].data, MDB_BUFFER[25].data, MDB_BUFFER[26].data, MDB_BUFFER[27].data};
	memcpy(&ReaderIDData[index].ModelRevision,&tmpmr, 12);
	EXT_UART_Transmit("*");
	EXT_UART_Transmit(ReaderIDData[index].ModelRevision);
	EXT_UART_Transmit("*");
	uint8_t srd[2] = {MDB_BUFFER[28].data, MDB_BUFFER[29].data};
	ReaderIDData[index].SoftwareVersion = BCDByteToInt(srd);
	if (MDB_BUFFER_COUNT == 35)
	{
		ReaderIDData[index].FTLSupported = ((MDB_BUFFER[startindex + 33].data & (1 << 0)) != 0);
		ReaderIDData[index].Monetary32bitSupported = ((MDB_BUFFER[startindex + 33].data & (1 << 1)) != 0);
		ReaderIDData[index].MultiCurrencySupported = ((MDB_BUFFER[startindex + 33].data & (1 << 2)) != 0);
		ReaderIDData[index].NVendSupported = ((MDB_BUFFER[startindex + 33].data & (1 << 3)) != 0);
		ReaderIDData[index].DataEntrySupported = ((MDB_BUFFER[startindex + 33].data & (1 << 4)) != 0);
		ReaderIDData[index].AlwaysIdleSupported = ((MDB_BUFFER[startindex + 33].data & (1 << 5)) != 0);
	}
	sprintf(tmpstr,"%d*%d*%d*%d*%d*%d*%d\r\n", ReaderIDData[index].SoftwareVersion, ReaderIDData[index].FTLSupported, ReaderIDData[index].Monetary32bitSupported, ReaderIDData[index].MultiCurrencySupported, ReaderIDData[index].NVendSupported, ReaderIDData[index].DataEntrySupported, ReaderIDData[index].AlwaysIdleSupported);
	EXT_UART_Transmit(tmpstr);
}

void ProcessReaderConfig(uint8_t index, uint8_t startindex)
{
	uint8_t tmpstr[80];
	ReaderSetupData[index].ReaderFeatureLevel = MDB_BUFFER[startindex + 1].data;
	ReaderSetupData[index].CountryOrCurrencyCode[0] = MDB_BUFFER[startindex + 2].data;
	ReaderSetupData[index].CountryOrCurrencyCode[1] = MDB_BUFFER[startindex + 3].data;
	ReaderSetupData[index].ScalingFactor = MDB_BUFFER[startindex + 4].data;
	ReaderSetupData[index].DecimalPlaces = MDB_BUFFER[startindex + 5].data;
	ReaderSetupData[index].MaxResponseTime = MDB_BUFFER[startindex + 6].data;
	ReaderSetupData[index].Refundable = ((MDB_BUFFER[startindex + 7].data & (1 << 0)) != 0);
	ReaderSetupData[index].Multivend = ((MDB_BUFFER[startindex + 7].data & (1 << 1)) != 0);
	ReaderSetupData[index].DisplayAvailable = ((MDB_BUFFER[startindex + 7].data & (1 << 2)) != 0);
	ReaderSetupData[index].VendCashSaleSupport = ((MDB_BUFFER[startindex + 7].data & (1 << 3)) != 0);
	uint16_t usercountrycode = BCDByteToInt(ReaderSetupData[index].CountryOrCurrencyCode);
	sprintf(tmpstr,"CD%d*CFG1*%d*%d*%d*%d*%d*%d*%d*%d*%d", index + 1, ReaderSetupData[index].ReaderFeatureLevel, usercountrycode, ReaderSetupData[index].ScalingFactor, \
	ReaderSetupData[index].DecimalPlaces, ReaderSetupData[index].MaxResponseTime, ReaderSetupData[index].Refundable, ReaderSetupData[index].Multivend, ReaderSetupData[index].DisplayAvailable, ReaderSetupData[index].VendCashSaleSupport);
	EXT_UART_Transmit(tmpstr);
	EXT_CRLF();
}

void ProcessReaderVendApproved(uint8_t index, MDB_Byte vendappdata[])
{
	uint8_t buff[10 + ReaderSetupData[index].DecimalPlaces];
	uint8_t tmpstr[32];
	unsigned long availablefundsdata;
	if (sizeof(vendappdata) == 10)
	{
		availablefundsdata = vendappdata[1].data << 24;
		availablefundsdata |= (vendappdata[2].data << 16) | (vendappdata[3].data << 8) | (vendappdata[4].data);
	}
	if (sizeof(vendappdata) == 6)
	{
		availablefundsdata = vendappdata[1].data << 8;
		availablefundsdata |= vendappdata[2].data;
	}
	dtostrf(availablefundsdata / pow(10, ReaderSetupData[index].DecimalPlaces),0,ReaderSetupData[index].DecimalPlaces,buff);
	sprintf(tmpstr,"CD%d*VAPPR*%s", index + 1, buff);
	EXT_UART_Transmit(tmpstr);
	EXT_CRLF();
}

void ProcessReaderSessionBegin(uint8_t index, MDB_Byte sbdata[])
{
	uint8_t buff[10 + ReaderSetupData[index].DecimalPlaces];
	uint8_t tmpstr[32];
	unsigned long availablefundsdata;
	uint8_t sbsize = sizeof(sbdata);
	switch (sbsize)
	{
		case 34:
		{
			unsigned long availablefundsdata = sbdata[1].data << 24;
			availablefundsdata |= (sbdata[2].data << 16) | (sbdata[3].data << 8) | (sbdata[4].data);
			dtostrf(availablefundsdata / pow(10, ReaderSetupData[index].DecimalPlaces),0,ReaderSetupData[index].DecimalPlaces,buff);
			uint8_t paymentmediaid[9];
			uint8_t paymenttype[16];
			switch (sbdata[9].data >> 6)
			{
				case 0:
				sprintf(paymenttype,"%s","NORMAL");
				break;
				case 1:
				sprintf(paymenttype,"%s","TEST");
				break;
				case 2:
				sprintf(paymenttype,"%s","FREE");
				break;
				default:
				switch (sbdata[9].data & 0x3f)
				{
					case 0:
					sprintf(paymenttype,"%s","VMCDP");
					break;
					case 1:
					sprintf(paymenttype,"UG%d*PLN%d", sbdata[10].data, sbdata[11].data);
					break;
					case 2:
					sprintf(paymenttype,"UG%d*DGI%d", sbdata[10].data, sbdata[11].data);
					break;
					case 3:
					sprintf(paymenttype,"DISCP%d", sbdata[11].data);
					break;
					case 4:
					sprintf(paymenttype,"SURCP%d", sbdata[11].data);
					break;
				}
				break;
			}
			sprintf(paymentmediaid,"%02x%02x%02x%02x",sbdata[5].data,sbdata[6].data,sbdata[7].data,sbdata[8].data);
			uint8_t userlanguage[2] = {sbdata[12].data,sbdata[13].data};
			uint8_t usercountrycodedata[2] = {sbdata[14].data,sbdata[15].data};
			uint16_t usercountrycode = BCDByteToInt(usercountrycodedata);
			sprintf(tmpstr,"CD%d*SBEGIN*%s*%s*%s*%s*%d*%d*%d*%d", index + 1, buff, paymentmediaid, paymenttype, \
			userlanguage, usercountrycode, ((sbdata[16].data & (1 << 0)) == 0), ((sbdata[16].data & (1 << 1)) != 0), ((sbdata[16].data & (1 << 2)) != 0));
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
		break;
		case 10:
		{
			availablefundsdata = sbdata[1].data << 8;
			availablefundsdata |= sbdata[2].data;
			dtostrf(availablefundsdata / pow(10, ReaderSetupData[index].DecimalPlaces),0,ReaderSetupData[index].DecimalPlaces,buff);
			uint8_t paymentmediaid[9];
			uint8_t paymenttype[16];
			switch (sbdata[7].data >> 6)
			{
				case 0:
				sprintf(paymenttype,"%s","NORMAL");
				break;
				case 1:
				sprintf(paymenttype,"%s","TEST");
				break;
				case 2:
				sprintf(paymenttype,"%s","FREE");
				break;
				default:
				switch (sbdata[7].data & 0x3f)
				{
					case 0:
					sprintf(paymenttype,"%s","VMCDP");
					break;
					case 1:
					sprintf(paymenttype,"UG%d*PLN%d", sbdata[8].data, sbdata[9].data);
					break;
					case 2:
					sprintf(paymenttype,"UG%d*DGI%d", sbdata[8].data, sbdata[9].data);
					break;
					case 3:
					sprintf(paymenttype,"DISCP%d", sbdata[9].data);
					break;
					case 4:
					sprintf(paymenttype,"SURCP%d", sbdata[9].data);
					break;
				}
				break;
			}
			sprintf(paymentmediaid,"%02x%02x%02x%02x",sbdata[3].data,sbdata[4].data,sbdata[5].data,sbdata[6].data);
			sprintf(tmpstr,"CD%d*SBEGIN*%s*%s*%s", index + 1, buff, paymentmediaid, paymenttype);
			EXT_UART_Transmit(tmpstr);
		}
		break;
		case 6:
		{
			availablefundsdata = sbdata[1].data << 8;
			availablefundsdata |= sbdata[2].data;
			dtostrf(availablefundsdata / pow(10, ReaderSetupData[index].DecimalPlaces),0,ReaderSetupData[index].DecimalPlaces,buff);
			sprintf(tmpstr,"CD%d*SBEGIN*%s", index + 1, buff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
		}
		break;
		default:
		break;
	}
}

void ProcessReaderError(uint8_t index, MDB_Byte errdata[])
{
	uint8_t tmpstr[32];
	uint8_t error[8];
	switch (errdata[1].data >> 4)
	{
		case 0:
		sprintf(error,"%s", "PMERR");
		break;
		case 1:
		sprintf(error,"%s", "IPM");
		break;
		case 2:
		sprintf(error,"%s", "TAMP");
		break;
		case 3:
		sprintf(error,"%s", "MERR1");
		break;
		case 4:
		sprintf(error,"%s", "COMERR");
		break;
		case 5:
		sprintf(error,"%s", "SERV");
		break;
		case 6:
		sprintf(error,"%s", "6");
		break;
		case 7:
		sprintf(error,"%s", "MERR2");
		break;
		case 8:
		sprintf(error,"%s", "RFAIL");
		break;
		case 9:
		sprintf(error,"%s", "COMERR2");
		break;
		case 10:
		sprintf(error,"%s", "PMJAM");
		break;
		case 11:
		sprintf(error,"%s", "MERR3");
		break;
		case 12:
		sprintf(error,"%s", "REFERR");
		break;
		default:
		sprintf(error,"%s", "UNASGND");
		break;
	}
	sprintf(tmpstr,"CD%d*ERROR*%s*%d", index + 1, error, errdata[1].data & 0x0f);
	EXT_UART_Transmit(tmpstr);
	EXT_CRLF();
}

void ReaderReset(uint8_t index)
{
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[2] = { addr, addr };
	MDB_Send(cmd, 2);
	ReaderProcessResponse(index, "RESET");
}

void ProcessReaderRevalueLimit(uint8_t index, MDB_Byte rlimdata[])
{
	unsigned long availablefundsdata;
	uint8_t tmpstr[32];
	uint8_t buff[10];
	if (sizeof(rlimdata) == 10)
	{
		availablefundsdata = rlimdata[1].data << 24;
		availablefundsdata |= (rlimdata[2].data << 16) | (rlimdata[3].data << 8) | (rlimdata[4].data);
	}
	if (sizeof(rlimdata) == 6)
	{
		availablefundsdata = rlimdata[1].data << 8;
		availablefundsdata |= rlimdata[2].data;
	}
	dtostrf(availablefundsdata / pow(10, ReaderSetupData[index].DecimalPlaces),0,ReaderSetupData[index].DecimalPlaces,buff);
	sprintf(tmpstr,"CD%d*REVLIMIT*%s", index + 1, buff);
	EXT_UART_Transmit(tmpstr);
	EXT_CRLF();
}

void ReaderResponse(uint8_t index)
{
	uint8_t tmpstr[80];
	uint8_t dispbuff[32];
	MDB_Byte tmpsetup[8];
	MDB_Byte tmpidlevel12[30];
	MDB_Byte tmpidlevel3[34];
	uint8_t buff[16];
	uint16_t tmplen = MDB_BUFFER_COUNT;
	MDB_Byte TMP[tmplen];
	memcpy(&TMP, &MDB_BUFFER[0], MDB_BUFFER_COUNT * 2);
	for (int i = 0; i < tmplen - 1; i++)
	{
		switch (TMP[i].data)
		{
			case 0x00:
			sprintf(tmpstr,"CD%d*JSTRST", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			CashlessDeviceSetup(index);
			CashlessDeviceSetupPrices16bit(index);
			CashlessDeviceRequestExpansionID(index);
			CashlessDeviceEnableOptFetures(index);
			return;
			case 0x01:
			//memcpy(&tmpsetup[0], &TMP[i], 16);
			ProcessReaderConfig(index, i);
			if (tmplen > 8) i += 7;
			break;
			case 0x02:
			for (int a = 2; a < 34; a++)
			{
				dispbuff[a - 2] = TMP[a].data;
			}
			sprintf(tmpstr,"CD%d*DISPREQ*%d*%s", index + 1, TMP[i + 1].data, dispbuff);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			if (tmplen > 2) i++;
			return;
			case 0x03:
			if (ReaderSetupData[index].ReaderFeatureLevel == 1)
			{
				MDB_Byte sbdata[3];
				memcpy(&sbdata, &TMP[i], 6);
				ProcessReaderSessionBegin(index,sbdata);
				if (tmplen > 4) i += 3;
			}
			if (ReaderSetupData[index].ReaderFeatureLevel >= 2)
			{
				uint8_t buff[16];
				if (!((ReaderSetupData[index].ReaderFeatureLevel == 3) && (ReaderOptions[index].ReaderOptFeatures.MonetaryFormat32bitEnabled || ReaderOptions[index].ReaderOptFeatures.MultiCurrEnabled)))
				{
					MDB_Byte sbdata[10];
					memcpy(&sbdata, &TMP[i], 20);
					ProcessReaderSessionBegin(index,sbdata);
					if (tmplen > 10) i += 9;
				} else
				{
					MDB_Byte sbdata[17];
					memcpy(&sbdata, &TMP[i], 34);
					ProcessReaderSessionBegin(index,sbdata);
					if (tmplen > 17) i += 16;
				}
			}
			break;
			case 0x04:
			sprintf(tmpstr,"CD%d*SCANCREQ", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			break;
			case 0x05:
			{
				if ((ReaderSetupData[index].ReaderFeatureLevel == 3) && (ReaderOptions[index].ReaderOptFeatures.MonetaryFormat32bitEnabled || ReaderOptions[index].ReaderOptFeatures.MultiCurrEnabled))
				{
					MDB_Byte tmpvendappdata[5];
					memcpy(&tmpvendappdata, &TMP[i], 10);
					ProcessReaderVendApproved(index,tmpvendappdata);
					i += 4;
				} else
				{
					MDB_Byte tmpvendappdata[3];
					memcpy(&tmpvendappdata, &TMP[i], 6);
					ProcessReaderVendApproved(index,tmpvendappdata);
					i += 2;
				}
			}
			break;
			case 0x06:
			{
				//uint8_t * buff[6];
				//sprintf(buff, "%d\r\n", MDB_BUFFER_COUNT);
				//EXT_UART_Transmit(buff);
				//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
					//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
					//EXT_UART_Transmit(buff);
				//}
				//sprintf(&buff, "%02x ", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
				//EXT_UART_Transmit(buff);
				//EXT_CRLF();
				sprintf(tmpstr,"CD%d*VDENY", index + 1);
				EXT_UART_Transmit(tmpstr);
				EXT_CRLF();
			}
			if (tmplen > 2) i++;
			break;
			case 0x07:
			sprintf(tmpstr,"CD%d*SEND", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			break;
			case 0x08:
			sprintf(tmpstr,"CD%d*CNCLD", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			break;
			case 0x09:
			if (VMCData.VMC_FEATURE_LEVEL == 3)
			{
				memcpy(&tmpidlevel3, &TMP[i], 68);
				ProcessReaderExpID(index, tmpidlevel3);
				i += 33;
			} else
			{
				memcpy(&tmpidlevel12, &TMP[i], 60);
				ProcessReaderExpID(index, tmpidlevel12);
				i += 29;
			}
			break;
			case 0x0a:
			{
				MDB_Byte errdata[2];
				memcpy(&errdata, &TMP[i], 4);
				ProcessReaderError(index,errdata);
				i ++;
			}
			break;
			case 0x0b:
			sprintf(tmpstr,"CD%d*COOS", index + 1);
			EXT_UART_Transmit(tmpstr);
			if (ReaderSetupData[index].ReaderFeatureLevel >= 2)
			{
				sprintf(tmpstr,"*%d", TMP[i + 1].data);
				EXT_UART_Transmit(tmpstr);
				CashlessDevice[index].Status = TMP[i + 1].data;
				i++;
			}
			EXT_CRLF();
			ReaderReset(index);
			return;
			case 0x0d:
			sprintf(tmpstr,"CD%d*REVAPP", index + 1);
			EXT_UART_Transmit(tmpstr);
			break;
			case 0x0e:
			sprintf(tmpstr,"CD%d*REVDENY", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			break;
			case 0x0f:
			{
				if ((ReaderSetupData[index].ReaderFeatureLevel == 3) && (ReaderOptions[index].ReaderOptFeatures.MonetaryFormat32bitEnabled || ReaderOptions[index].ReaderOptFeatures.MultiCurrEnabled))
				{
					MDB_Byte rlimdata[5];
					memcpy(&rlimdata, &TMP[i], 10);
					ProcessReaderRevalueLimit(index,rlimdata);
					i += 4;
					
				} else
				{
					MDB_Byte rlimdata[3];
					memcpy(&rlimdata, &TMP[i], 6);
					ProcessReaderRevalueLimit(index,rlimdata);
					i += 2;
				}
			}
			break;
			case 0x11:
			sprintf(tmpstr,"CD%d*DTR", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			//TODO:The VMC will follow with the EXPANSION-WRITE
			//TIME/DATE FILE to the card reader. Refer to paragraph 7.4.19.
			break;
			case 0x12:
			sprintf(tmpstr,"CD%d*DER*%d*%d", index + 1, TMP[i + 1].data >> 7, TMP[i + 1].data & 0x7f);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			i++;
			break;
			case 0x13:
			sprintf(tmpstr,"CD%d*DECNCL", index + 1);
			EXT_UART_Transmit(tmpstr);
			EXT_CRLF();
			//The user has pushed the reader’s RETURN button before completing the
			//DATA ENTRY. The VMC should terminate all DATA ENTRY activity in
			//progress.
			break;
		}
	}
}

void ReaderDataEntryResponse(uint8_t index, uint8_t Keys[8])
{
	uint8_t cmd[11];
	uint8_t addr = (index) ? 0x60 : 0x10;
	if (!index)
	{
		cmd[0] = 0x14;
	} else
	{
		cmd[0] = 0x64;
	}
	cmd[1] = 0x03;
	cmd[2] = Keys[0];
	cmd[3] = Keys[1];
	cmd[4] = Keys[2];
	cmd[5] = Keys[3];
	cmd[6] = Keys[4];
	cmd[7] = Keys[5];
	cmd[8] = Keys[6];
	cmd[9] = Keys[7];
	cmd[10] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7] + cmd[8] + cmd[9]) & 0xff;
	MDB_Send(cmd, 11);
	ReaderProcessResponse(index, "DERESP");
}

void ReaderVendRequest(uint8_t index, double price, uint16_t itemnumber)
{
	uint32_t tmpprice = round((price * pow(10, ReaderSetupData[index].DecimalPlaces)) * pow(10, ReaderSetupData[index].DecimalPlaces)) / (ReaderSetupData[index].ScalingFactor * pow(10, ReaderSetupData[index].DecimalPlaces));
	if (ReaderOptions[index].ReaderOptFeatures.MonetaryFormat32bitEnabled || ReaderOptions[index].ReaderOptFeatures.MultiCurrEnabled)
	{
		uint8_t cmd[9];
		cmd[0] = (index) ? 0x63 : 0x13;
		cmd[1] = 0x00;
		cmd[2] = (tmpprice >> 24) & 0xff;
		cmd[3] = (tmpprice >> 16) & 0xff;
		cmd[4] = (tmpprice >> 8) & 0xff;
		cmd[5] = tmpprice & 0xff;
		cmd[6] = (itemnumber >> 8) & 0xff;
		cmd[7] = itemnumber & 0xff;
		cmd[8] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7]) & 0xff;
		MDB_Send(cmd, 9);
	} else
	{
		uint8_t cmd[7];
		cmd[0] = (index) ? 0x63 : 0x13;
		cmd[1] = 0x00;
		cmd[2] = (tmpprice >> 8) & 0xff;
		cmd[3] = tmpprice & 0xff;
		cmd[4] = (itemnumber >> 8) & 0xff;
		cmd[5] = itemnumber & 0xff;
		cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
		MDB_Send(cmd, 7);
	}
	ReaderProcessResponse(index, "VENDREQ");
}

void ReaderVendCancel(uint8_t index)
{
	uint8_t cmd[3];
	cmd[0] = (index) ? 0x63 : 0x13;
	cmd[1] = 0x01;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	ReaderProcessResponse(index, "VENDCANCEL");
	//while (!MDBReceiveComplete){
		//MDB_read();
	//}
	//if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	//{
		//if (MDB_BUFFER[0].data == 0x06)
		//{
			//MDB_ACK();
			//sprintf(tmpstr,"CD%d*VENDCANCEL*OK\r\n", index + 1);
			//EXT_UART_Transmit(tmpstr);
			//return;
		//}
	//} 
	//sprintf(tmpstr,"CD%d*VENDCANCEL*", index + 1);
	//EXT_UART_Transmit(tmpstr);
	//EXT_UART_FAIL();
}

void ReaderVendSuccess(uint8_t index, uint16_t itemnumber)
{
	uint8_t tmpstr[32];
	uint8_t cmd[5];
	cmd[0] = (index) ? 0x63 : 0x13;
	cmd[1] = 0x02;
	cmd[2] = (itemnumber >> 8) & 0xff;
	cmd[3] = itemnumber & 0xff;
	cmd[4] = (cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xff;
	MDB_Send(cmd, 5);
	while (!MDBReceiveComplete){
		MDB_read();
	}
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
			sprintf(tmpstr,"CD%d*VENDSUCCESS*OK\r\n", index + 1);
			EXT_UART_Transmit(tmpstr);
	} else
	{
		sprintf(tmpstr,"CD%d*VENDSUCCESS*", index + 1);
		EXT_UART_Transmit(tmpstr);
		EXT_UART_FAIL();
	}
}

void ReaderVendFailure(uint8_t index)
{
	uint8_t tmpstr[32];
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x03;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	ReaderProcessResponse(index, "VENDFAIL");
}

void ReaderSessionComplete(uint8_t index)
{
	uint8_t tmpstr[32];
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x04;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	ReaderProcessResponse(index, "SCOMPL");
}

void ReaderCashSale(uint8_t index, double price, uint16_t itemnumber)
{
	uint32_t tmpprice = round((price * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[7];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x05;
	cmd[2] = (tmpprice >> 8) & 0xff;
	cmd[3] = tmpprice & 0xff;
	cmd[4] = (itemnumber >> 8) & 0xff;
	cmd[5] = itemnumber & 0xff;
	cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
	MDB_Send(cmd, 7);
	ReaderProcessResponse(index, "CSHSALE");
}

void ReaderCashSaleExp(uint8_t index, double price, uint16_t itemnumber, uint8_t currency[2])
{
	uint32_t tmpprice = round((price * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[11];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x05;
	cmd[2] = (tmpprice >> 24) & 0xff;
	cmd[3] = (tmpprice >> 16) & 0xff;
	cmd[4] = (tmpprice >> 8) & 0xff;
	cmd[5] = tmpprice & 0xff;
	cmd[6] = (itemnumber >> 8) & 0xff;
	cmd[7] = itemnumber & 0xff;
	cmd[8] = currency[0];
	cmd[9] = currency[1];
	cmd[10] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7] + cmd[8] + cmd[9]) & 0xff;
	MDB_Send(cmd, 11);
	ReaderProcessResponse(index, "CSHSALE");
}

void ReaderNegativeVend(uint8_t index, double price, uint16_t itemnumber)
{
	uint32_t tmpprice = round((price * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[7];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x06;
	cmd[2] = (tmpprice >> 8) & 0xff;
	cmd[3] = tmpprice & 0xff;
	cmd[4] = (itemnumber >> 8) & 0xff;
	cmd[5] = itemnumber & 0xff;
	cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
	MDB_Send(cmd, 7);
	ReaderProcessResponse(index, "NVEND");
}

void ReaderNegativeVendExp(uint8_t index, double price, uint16_t itemnumber)
{
	uint32_t tmpprice = round((price * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[11];
	if (!index)
	{
		cmd[0] = 0x13;
	} else
	{
		cmd[0] = 0x63;
	}
	cmd[1] = 0x06;
	cmd[2] = (tmpprice >> 24) & 0xff;
	cmd[3] = (tmpprice >> 16) & 0xff;
	cmd[4] = (tmpprice >> 8) & 0xff;
	cmd[5] = tmpprice & 0xff;
	cmd[6] = (itemnumber >> 8) & 0xff;
	cmd[7] = itemnumber & 0xff;
	cmd[8] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7]) & 0xff;
	MDB_Send(cmd, 11);
	ReaderProcessResponse(index, "NVEND");
}

void ReaderEDC(uint8_t index, uint8_t action)
{
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x14;
	} else
	{
		cmd[0] = 0x64;
	}
	cmd[1] = action;//0x00 = Disable; 0x01 = Enable; 0x02 = Cancel
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	ReaderProcessResponse(index, "EDC");
}

void ReaderRevalueRequest(uint8_t index, double amount)
{
	uint32_t tmppamount = round((amount * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[5];
	if (!index)
	{
		cmd[0] = 0x15;
	} else
	{
		cmd[0] = 0x65;
	}
	cmd[1] = 0x00;
	cmd[2] = (tmppamount >> 8) & 0xff;
	cmd[3] = tmppamount & 0xff;
	cmd[4] = (cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xff;
	MDB_Send(cmd, 5);
	ReaderProcessResponse(index, "REVRQ");
}

void ReaderRevalueRequestExp(uint8_t index, double amount)
{
	uint32_t tmppamount = round((amount * pow(10, ReaderSetupData[index].DecimalPlaces)) * 100) / (ReaderSetupData[index].ScalingFactor * 100);
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[7];
	if (!index)
	{
		cmd[0] = 0x15;
	} else
	{
		cmd[0] = 0x65;
	}
	cmd[1] = 0x00;
	cmd[2] = (tmppamount >> 24) & 0xff;
	cmd[3] = (tmppamount >> 16) & 0xff;
	cmd[4] = (tmppamount >> 8) & 0xff;
	cmd[5] = tmppamount & 0xff;
	cmd[6] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
	MDB_Send(cmd, 7);
	ReaderProcessResponse(index, "REVRQ");
}

void ReaderRevalueLimitRequest(uint8_t index)
{
	uint8_t addr = (index) ? 0x60 : 0x10;
	uint8_t cmd[3];
	if (!index)
	{
		cmd[0] = 0x15;
	} else
	{
		cmd[0] = 0x65;
	}
	cmd[1] = 0x01;
	cmd[2] = (cmd[0] + cmd[1]) & 0xff;
	MDB_Send(cmd, 3);
	ReaderProcessResponse(index, "RLIMRQ");
}

void ReaderWriteDateTime(uint8_t index, uint8_t BCDDateTimeData[10])
{
	uint8_t cmd[13];
	cmd[0] = (index) ? 0x67 : 0x17;
	cmd[1] = 0x03;
	memcpy(&cmd[2], &BCDDateTimeData, 10);
	cmd[12] = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6] + cmd[7] + cmd[8] + cmd[9] + cmd[10] + cmd[11]) & 0xff;
	MDB_Send(cmd, 13);
	ReaderProcessResponse(index, "WTD");
}

void ReaderProcessResponse(uint8_t index, uint8_t resp[])
{
	uint8_t tmpstr[32];
	uint8_t * buff[6];
	while (!MDBReceiveComplete){
		MDB_read();
	}
	//return;
	if ((MDBReceiveComplete) && (!MDBReceiveErrorFlag))
	{
		if (MDB_BUFFER_COUNT > 1)
		{
			MDB_ACK();
			//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
			//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
			//EXT_UART_Transmit(buff);
			//}
			//sprintf(&buff, "%02x ", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
			//EXT_UART_Transmit(buff);
			//EXT_CRLF();
			//return;
			ReaderResponse(index);
			//return;
		}
		CDLED_ON(index);
		if (MDB_BUFFER_COUNT == 1)
		{
			if (strlen(resp) > 1)
			{
				sprintf(tmpstr,"CD%d*%s*", index + 1, resp);
				EXT_UART_Transmit(tmpstr);
				if (MDB_BUFFER[0].data == 0x00 && MDB_BUFFER[0].mode)
				{
					EXT_UART_OK();
				} else
				{
					EXT_UART_NAK();
				}
			}
		}
	} else
	{
		if (resp[0] != 0x00)
		{
			sprintf(tmpstr,"CD%d*%s*", index + 1, resp);
			EXT_UART_Transmit(tmpstr);
			EXT_UART_FAIL();
			CDLED_OFF(index);
		}
	}
}