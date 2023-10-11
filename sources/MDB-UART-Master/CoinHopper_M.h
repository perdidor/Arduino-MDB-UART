/*
 * CoinHopper_M.h
 *
 * Created: 18.05.2019 10:03:37
 *  Author: root
 */ 
#include "MDB_M.h"

#ifndef COINHOPPER_M_H_
#define COINHOPPER_M_H_

typedef struct {
	uint8_t DispenserFeatureLevel;
	uint16_t CountryOrCurrencyCode;
	uint8_t CoinScalingFactor;
	uint8_t DecimalPlaces;
	uint8_t MaxResponseTime;
	uint8_t DisabledCoinTypes[16];
	uint8_t CoinSelfFilling[16];
	uint8_t CoinTypeCredit[16];
} chsetupdata;

typedef struct {
	uint16_t SoftwareVersion;
	uint8_t FTLSupported;
	uint8_t ManufacturerCode[3];
	uint8_t SerialNumber[12];
	uint8_t ModelRevision[12];
} chiddata;

chiddata CoinHopperIDData[2];
chsetupdata CoinHopperSetupData[2];
mdbdevice CoinHopperDevice[2];

void GetCoinHopperSetupData(uint8_t index);
void GetCoinHopperDispenserStatus(uint8_t index);
void GetCoinHopperIdentification(uint8_t index);
void GetCoinHopperPayoutValue(uint8_t index);
void CoinHopperPayoutStatus(uint8_t index);
void CoinHopperDispenseValue(uint8_t index, uint16_t PayoutValue);
void CoinHopperDispenseCoins(uint8_t index, uint8_t CoinType, uint16_t CoinsCount);
void CoinHopperEnableManualDispenseCoinType(uint8_t index, uint8_t CoinType, uint8_t EnableManualDispense);
void CoinHopperPollResponse(uint8_t index);




#endif /* COINHOPPER_M_H_ */