/*
 * Settings_M.h
 *
 * Created: 18.05.2019 10:11:24
 *  Author: root
 */ 


#ifndef SETTINGS_M_H_
#define SETTINGS_M_H_

#define STORAGE_OFFSET	128

typedef struct {
	uint8_t VMC_FEATURE_LEVEL;
	uint8_t VMC_DISPLAY_COLUMNS;	//0x00 = no display available
	uint8_t VMC_DISPLAY_ROWS;
	uint8_t VMC_DISPLAY_TYPE;	//0x00 = Numbers, upper case letters, blank and decimal point; 0x01 = Full ASCII

	uint8_t VMCMfgCode[3];
	uint8_t VMCSerialNumber[12];
	uint8_t VMCModelNumber[12];
	uint8_t VMCSofwareVersion[2];
} VMCData_t;

typedef union {
	unsigned long Value;
	uint8_t Bytes[4];
} cashlessprice32bit;

typedef struct {
	uint8_t FTLEnabled;
	uint8_t MonetaryFormat32bitEnabled;
	uint8_t MultiCurrEnabled;
	uint8_t NegVendEnabled;
	uint8_t DataEntryEnabled;
	uint8_t AlwaysIdleEnabled;
} CDOptFeatures;

typedef struct {
	cashlessprice32bit MaxPrice;
	cashlessprice32bit MinPrice;
	uint8_t CountryOrCurrencyCode[2];
	CDOptFeatures ReaderOptFeatures;
} ReaderOptions_t;

typedef struct {
	uint16_t EnableAcceptCoinsBits;
	uint16_t EnableDispenseCoinsBits;
	uint8_t EnableExtOptionsBits;
} CoinChangerOptions_t;

typedef struct {
	uint16_t BillSecurityBits;
	uint16_t EnableAcceptBillsBits;
	uint16_t EnableEscrowBillsBits;
	uint16_t EnableRecycleBillsBits;
	uint16_t EnableManualDispenseBillsBits;
	uint8_t EnableBillRecycling;
} BillValidatorOptions_t;

typedef struct {
	uint16_t EnableManualDispenseCoinsBits;
	uint8_t EnableExtOptionsBits;
} CoinHopperOptions_t;
	
VMCData_t VMCData;
ReaderOptions_t ReaderOptions[2];
CoinChangerOptions_t CoinChangerOptions;
BillValidatorOptions_t BillValidatorOptions;
CoinHopperOptions_t CoinHopperOptions[2];

void ReadVMCData(void);
void ReadCashlessPrices(void);
void ReadCoinChangerOptions(void);
void WriteCoinChangerOptions(void);
void ResetCoinChangerOptions(void);
void ReadBVOptions(void);
void WriteBVOptions(void);
void ResetBVOptions(void);
void ReadCoinHoppersOptions(void);
void WriteCoinHoppersOptions(void);
void ResetCoinHoppersOptions(void);

#endif /* SETTINGS_M_H_ */