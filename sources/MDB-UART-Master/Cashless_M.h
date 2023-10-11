/*
 * Cashless_M.h
 *
 * Created: 18.05.2019 10:08:09
 *  Author: root
 */ 
#include "MDB_M.h"


#ifndef CASHLESS_M_H_
#define CASHLESS_M_H_

typedef struct {
	uint8_t ReaderFeatureLevel;
	uint8_t ScalingFactor;
	uint8_t DecimalPlaces;
	uint8_t MaxResponseTime;
	uint8_t Refundable;
	uint8_t Multivend;
	uint8_t DisplayAvailable;
	uint8_t VendCashSaleSupport;
	uint8_t CountryOrCurrencyCode[2];
} cdsetupdata;

typedef struct {
	uint8_t FTLSupported;
	uint8_t Monetary32bitSupported;
	uint8_t MultiCurrencySupported;
	uint8_t NVendSupported;
	uint8_t DataEntrySupported;
	uint8_t AlwaysIdleSupported;
	uint8_t ManufacturerCode[3];
	uint8_t SerialNumber[12];
	uint8_t ModelRevision[12];
	uint16_t SoftwareVersion;
} cdiddata;

cdiddata ReaderIDData[2];
cdsetupdata ReaderSetupData[2];
mdbdevice CashlessDevice[2];

void CashlessDeviceSetup(uint8_t index);
void CashlessDeviceRequestExpansionID(uint8_t index);
void CashlessDeviceSetupPrices16bit(uint8_t index);
void CashlessDeviceSetupPrices32bit(uint8_t index);
void ReaderResponse(uint8_t index);
void CashlessDeviceEnableOptFetures(uint8_t index);
void ProcessReaderConfig(uint8_t index, uint8_t startindex);
void ProcessReaderExpID(uint8_t index, uint8_t startindex);
void ReaderVendRequest(uint8_t index, double price, uint16_t itemnumber);
void ProcessReaderVendApproved(uint8_t index, MDB_Byte vendappdata[]);
void ProcessReaderSessionBegin(uint8_t index, MDB_Byte sbdata[]);
void ProcessReaderError(uint8_t index, MDB_Byte errdata[]);
void ReaderReset(uint8_t index);
void ProcessReaderRevalueLimit(uint8_t index, MDB_Byte rlimdata[]);
void ReaderWriteDateTime(uint8_t index, uint8_t BCDDateTimeData[10]);
void ReaderRevalueLimitRequest(uint8_t index);
void ReaderRevalueRequestExp(uint8_t index, double amount);
void ReaderRevalueRequest(uint8_t index, double amount);
void ReaderEDC(uint8_t index, uint8_t action);
void ReaderNegativeVendExp(uint8_t index, double price, uint16_t itemnumber);
void ReaderNegativeVend(uint8_t index, double price, uint16_t itemnumber);
void ReaderCashSaleExp(uint8_t index, double price, uint16_t itemnumber, uint8_t currency[2]);
void ReaderCashSale(uint8_t index, double price, uint16_t itemnumber);
void ReaderSessionComplete(uint8_t index);
void ReaderVendFailure(uint8_t index);
void ReaderVendSuccess(uint8_t index, uint16_t itemnumber);
void ReaderVendCancel(uint8_t index);
void ReaderDataEntryResponse(uint8_t index, uint8_t Keys[8]);
void ReaderProcessResponse(uint8_t index, uint8_t resp[]);



#endif /* CASHLESS_M_H_ */