/*
 * CoinChanger_M.h
 *
 * Created: 18.05.2019 09:59:01
 *  Author: root
 */ 
#include "MDB_M.h"

#ifndef COINCHANGER_M_H_
#define COINCHANGER_M_H_

typedef struct {
	uint16_t SoftwareVersion;
	uint8_t AlternativePayout;
	uint8_t ExtendedDiagnostic;
	uint8_t ControlledManualFillAndPayout;
	uint8_t FTLSupported;
	uint8_t ManufacturerCode[3];
	uint8_t SerialNumber[12];
	uint8_t ModelRevision[12];
} cciddata;

typedef struct {
	uint8_t CoinChangerFeatureLevel;
	uint16_t CountryOrCurrencyCode;
	uint8_t CoinScalingFactor;
	uint8_t DecimalPlaces;
	uint8_t CoinsRouteable[16];
	uint8_t CoinTypeCredit[16];
} ccsetupdata;

cciddata CoinChangerIDData;
ccsetupdata CoinChangerSetupData;
mdbdevice CoinChangerDevice;

void GetCoinChangerTubeStatus(void);
void GetCoinChangerSetupData(void);
void CoinChangerPollResponse(void);
void CoinChangerDispense(uint8_t DispenseParams);
void CoinChangerEnableFeatures(void);
void CoinChangerAlternativePayout(uint8_t PayoutValue);
void CoinChangerAlternativePayoutStatus(void);
void CoinChangerAlternativePayoutValue(void);
void CoinChangerControlledManualFillReport(void);
void CoinChangerControlledManualPayoutReport(void);
void GetCoinChangerIdentification(void);
void GetCoinChangerDiagnosticStatus(void);
void CoinChangerDisableAcceptCoins(void);
void CoinChangerEnableAcceptCoins(void);
void CoinChangerEnableCoinType(uint8_t CoinType, uint8_t EnableAccept, uint8_t EnableDispense);
void CoinChangerConfigFeatures(uint8_t AlternativePayout, uint8_t ExtendedDiagnostic, uint8_t ControlledManualFillAndPayout);



#endif /* COINCHANGER_M_H_ */