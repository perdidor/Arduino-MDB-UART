/*
 * Cashless.c
 *
 * Created: 11.05.2019 09:35:44
 *  Author: root
 */ 
#ifndef F_CPU
#define F_CPU       16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include "Cashless.h"
#include "MDB.h"
#include "Settings.h"
#include "VMC.h"

void HandleCashlessCommand(int CommandByte)
{
	switch (CommandByte)
	{
		case 0x10:
		Reader_Reset(0);
		break;
		case 0x60:
		Reader_Reset(1);
		break;
		case 0x11:
		Reader_Setup(0);
		break;
		case 0x61:
		Reader_Setup(1);
		break;
		case 0x12:
		Reader_Poll(0);
		break;
		case 0x62:
		Reader_Poll(1);
		break;
		case 0x13:
		Reader_Vend(0);
		break;
		case 0x63:
		Reader_Vend(1);
		break;
		case 0x14:
		Reader_Reader(0);
		break;
		case 0x64:
		Reader_Reader(1);
		break;
		case 0x15:
		Reader_Revalue(0);
		break;
		case 0x65:
		Reader_Revalue(1);
		break;
		case 0x17:
		Reader_Expansion(0);
		break;
		case 0x67:
		Reader_Expansion(1);
		break;
	}
}

void ReaderSetIdle(unsigned char index)
{
	Reader_Active_Command[index] = MDB_IDLE;
	Reader_Poll_Reply[index] = MDB_REPLY_ACK;
}

void Reader_Reset(unsigned char index) {
	// Wait for enough data in buffer to proceed reset
	if(!MDB_readBlock(1))
	{
		MDB_BUFFER_COUNT = 0;
		ReaderSetIdle(index);
		DebugMessage("Error: invalid checksum for [RESET]\r\n");
		return;
	}
	MDB_ACK();
	unsigned char * buff[16];
	sprintf(buff, "CD%d#RESET\r\n", index + 1);
	EXT_UART_Transmit(buff);
	ResetReaderVMCData(index);
	Reader_State[index] = MDB_INACTIVE;
	Reader_Active_Command[index] = MDB_IDLE;
	Reader_Poll_Reply[index] = MDB_REPLY_JUST_RESET;
}

void Reader_Setup(unsigned char index) {  
	//unsigned char * buff[6];
	//sprintf(buff, "%d\r\n", MDB_BUFFER_COUNT);
	//EXT_UART_Transmit(buff);
	//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
	//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
	//EXT_UART_Transmit(buff);
	//}
	//sprintf(&buff, "%02x\r\n", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
	//EXT_UART_Transmit(buff);
	//return;
	unsigned char buff[32];
	double pricevalue;
	unsigned char maxpricebuff[7 + ReaderSetupData[index].Decimal_Places];
	unsigned char minpricebuff[7 + ReaderSetupData[index].Decimal_Places];
	if(!MDB_readBlock(6))
	{
		Reader_Active_Command[index] = MDB_IDLE;
		Reader_Poll_Reply[index] = MDB_REPLY_ACK;
		DebugMessage("Error: invalid checksum for [SETUP]\r\n");
		return;
	}
	Reader_Active_Command[index] = MDB_SETUP;
    switch(MDB_BUFFER[1].data)
	{   
        // Stage 1 - config Data   
        case 0:
			{
				MDB_Byte tmpvmcsetupdata[4];
				memcpy(&tmpvmcsetupdata, &MDB_BUFFER[2], 8);
				// Send own config data
				if (!MDB_SendReaderSetupData(index))
				{
					DebugMessage("Error: no ACK received on [SETUP]\r\n");
				}
				Reader_Poll_Reply[index] = MDB_REPLY_READER_CFG;
				// store VMC configuration data
				ReaderVMCSetupData[index].feature_level = tmpvmcsetupdata[0].data;
				ReaderVMCSetupData[index].dispaly_cols = tmpvmcsetupdata[1].data;
				ReaderVMCSetupData[index].dispaly_rows = tmpvmcsetupdata[2].data;
				ReaderVMCSetupData[index].dispaly_info = tmpvmcsetupdata[3].data;
				sprintf(buff, "CD%d#SETUP#%d#%d#%d#%d\r\n", index + 1, ReaderVMCSetupData[index].feature_level, ReaderVMCSetupData[index].dispaly_cols, ReaderVMCSetupData[index].dispaly_rows, ReaderVMCSetupData[index].dispaly_info);
				EXT_UART_Transmit(buff);
				ReaderSetIdle(index);
			} 
        break;   
        // Stage 2 - price data   
        case 1:   
			MDB_ACK();
            ReaderVMCMaxPrice[index].Bytes[0] = MDB_BUFFER[2].data;   
			ReaderVMCMaxPrice[index].Bytes[1] = MDB_BUFFER[3].data;
            ReaderVMCMinPrice[index].Bytes[0] = MDB_BUFFER[4].data;
            ReaderVMCMinPrice[index].Bytes[1] = MDB_BUFFER[5].data; 
            Reader_State[index] = MDB_DISABLED;
			pricevalue = (ReaderSetupData[index].Scale_Factor * ReaderVMCMaxPrice[index].Value) / pow(10, ReaderSetupData[index].Decimal_Places);
			dtostrf(pricevalue,0,ReaderSetupData[index].Decimal_Places,maxpricebuff);
			pricevalue = (ReaderSetupData[index].Scale_Factor * ReaderVMCMinPrice[index].Value) / pow(10, ReaderSetupData[index].Decimal_Places);
			dtostrf(pricevalue,0,ReaderSetupData[index].Decimal_Places,minpricebuff);
			sprintf(buff, "CD%d#PRICE#%s#%s\r\n", index + 1, maxpricebuff, minpricebuff);
			DebugMessage(buff);
            ReaderSetIdle(index);  
        break;   
        // Unknown Subcommand from VMC   
        default: 
			DebugMessage("Error: unknown subcommand [SETUP]\r\n");
			ReaderSetIdle(index);  
        break;   
    }   
} 

void Reader_Reader(unsigned char index) {
	MDB_read();
	unsigned char * buff[16];
	if(MDBReceiveErrorFlag) 
	{
		Reader_Active_Command[index] = MDB_IDLE;
		Reader_Poll_Reply[index] = MDB_REPLY_ACK;
		DebugMessage("Error: Error on [READER]\r\n");
		return;
	}
	Reader_Active_Command[index] = MDB_READER;
	switch(MDB_BUFFER[1].data)
	{
		case 0:
		// READER DISABLE
			if(!MDB_readBlock(1))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [DISABLE]\r\n");
				return;
			}
			sprintf(buff, "CD%d#DISABLE", index + 1);
			EXT_UART_Transmit(buff);
			Reader_State[index] = MDB_DISABLED;
			MDB_ACK();
			ReaderSetIdle(index);
		break;
		case 1:
		// READER ENABLE
			if(!MDB_readBlock(1))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [ENABLE]\r\n");
				return;
			}
			MDB_ACK();
			sprintf(buff, "CD%d#ENABLE", index + 1);
			EXT_UART_Transmit(buff);
			Reader_State[index] = MDB_ENABLED;
			ReaderSetIdle(index);
		break;
		case 2:
		// READER CANCEL
			if(!MDB_readBlock(1))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [CANCEL]\r\n");
				return;
			}
			//
			//ToDo: send cancel command to external UART
			//
			if (!MDB_SendReaderCancelled(index))
			{
				DebugMessage("Error: no ACK received on [CANCELLED]\r\n");
			} else
			{
				ReaderSetIdle(index);
			}
			sprintf(buff, "CD%d#CANCEL", index + 1);
			EXT_UART_Transmit(buff);
			Reader_State[index] = MDB_ENABLED;
		break;
		case 3:
		// READER DATA ENTRY RESPONSE
			if(!MDB_readBlock(9))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [DATA ENTRY RESPONSE]\r\n");
				return;
			}
			MDB_ACK();
			sprintf(buff, "CD%d#DER#", index + 1);
			EXT_UART_Transmit(buff);
			for (unsigned char i = 2; i < 10; i++)
			{
				if (MDB_BUFFER[i].data != 0x00) DebugMessage(MDB_BUFFER[i].data);
			}
			DebugMessage("#\r\n");
			//
			//ToDo: send received keys to external UART
			//
			ReaderSetIdle(index);
		break;
		default:
		// Unknown Subcommand from VMC
			DebugMessage("Error: unknown subcommand [READER]\r\n");
			ReaderSetIdle(index);
		break;
	}
}

void Reader_Expansion(unsigned char index) {
	MDB_read();
	if(MDBReceiveErrorFlag) {
		Reader_Active_Command[index] = MDB_IDLE;
		Reader_Poll_Reply[index] = MDB_REPLY_ACK;
		DebugMessage("Error: Error on [EXPANSION]\r\n");
		return;
	}
	Reader_Active_Command[index] = MDB_EXPANSION;
	switch(MDB_BUFFER[1].data)
	{
		case 0:
		// Peripheral ID request
			//unsigned char * buff[6];
			//sprintf(buff, "%d\r\n", MDB_BUFFER_COUNT);
			//EXT_UART_Transmit(buff);
			//for (int a = 0; a < MDB_BUFFER_COUNT - 1; a++){
				//sprintf(&buff, "%02x ", MDB_BUFFER[a].data);
				//EXT_UART_Transmit(buff);
			//}
			//sprintf(&buff, "%02x\r\n", MDB_BUFFER[MDB_BUFFER_COUNT - 1].data);
			//EXT_UART_Transmit(buff);
			//return;
			{
				if(!MDB_readBlock(30))
				{
					Reader_Active_Command[index] = MDB_IDLE;
					Reader_Poll_Reply[index] = MDB_REPLY_ACK;
					DebugMessage("Error: invalid checksum for [PERIPHERIAL ID]\r\n");
					return;
				}
				// store VMC configuration data
				MDB_Byte tmpvmcid[29];
				memcpy(&tmpvmcid, &MDB_BUFFER[2], 58);
				if (!MDB_SendReaderPeriphIDData(index))
				{
					DebugMessage("Error: no ACK received on [PERIPHERIAL ID]\r\n");
				}
				unsigned char buff[16];
				sprintf(buff, "CD%d#PERIPHID#", index + 1);
				DebugMessage(buff);
				ReaderVMCIDData[index].ManufacturerCode[0] = 0x00;
				ReaderVMCIDData[index].SerialNumber[0] = 0x00;
				ReaderVMCIDData[index].ModelRevision[0] = 0x00;
				unsigned char tmpmfg[3] = {tmpvmcid[0].data, tmpvmcid[1].data, tmpvmcid[2].data};
				memcpy(&ReaderVMCIDData[index].ManufacturerCode, &tmpmfg, 3);
				sprintf(buff, "%s#", ReaderVMCIDData[index].ManufacturerCode);
				DebugMessage(buff);
				unsigned char tmpsn[12] = {tmpvmcid[3].data, tmpvmcid[4].data, tmpvmcid[5].data, tmpvmcid[6].data, tmpvmcid[7].data, tmpvmcid[8].data, tmpvmcid[9].data, tmpvmcid[10].data, tmpvmcid[11].data, tmpvmcid[12].data, tmpvmcid[13].data, tmpvmcid[14].data};
				memcpy(&ReaderVMCIDData[index].SerialNumber,&tmpsn, 12);
				sprintf(buff, "%s#", ReaderVMCIDData[index].SerialNumber);
				DebugMessage(buff);
				unsigned char tmpmr[12] = {tmpvmcid[15].data, tmpvmcid[16].data, tmpvmcid[17].data, tmpvmcid[18].data, tmpvmcid[19].data, tmpvmcid[20].data, tmpvmcid[21].data, tmpvmcid[22].data, tmpvmcid[23].data, tmpvmcid[24].data, tmpvmcid[25].data, tmpvmcid[26].data};
				memcpy(&ReaderVMCIDData[index].ModelRevision,&tmpmr, 12);
				sprintf(buff, "%s#", ReaderVMCIDData[index].ModelRevision);
				DebugMessage(buff);
				unsigned char tmpswversion[2] = {tmpvmcid[27].data, tmpvmcid[28].data};
				ReaderVMCIDData[index].SoftwareVersion = BCDByteToInt(tmpswversion);
				sprintf(buff, "%d\r\n", ReaderVMCIDData[index].SoftwareVersion);
				DebugMessage(buff);
				ReaderSetIdle(index);
			}
		break;
		case 3:
		// Write Time/Date
			if(!MDB_readBlock(11))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [TIME-DATE]\r\n");
				return;
			}
			MDB_ACK();
			DebugMessage("TIMEDATE\r\n");
			Reader_Active_Command[index] = MDB_EXPANSION;
			Reader_Poll_Reply[index] = MDB_REPLY_PERIPHERIAL_ID;
			//
			//ToDo: Send time and date to external UART
			//
			ReaderSetIdle(index);
		break;
		case 4:
		// Enable Options
			if(!MDB_readBlock(5))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [ENABLE OPTIONS]\r\n");
				return;
			}
			MDB_ACK();
			ReaderOptFeatures[index].NegativeVendEnabled = ((MDB_BUFFER[5].data >> 3) & 1) & ((ReaderPeriphID_Data[index].OptionalFeatureBits > 3) & 1);
			ReaderOptFeatures[index].DataEntryEnabled = ((MDB_BUFFER[5].data >> 4) & 1) & ((ReaderPeriphID_Data[index].OptionalFeatureBits > 4) & 1);
			ReaderOptFeatures[index].AlwaysIdleEnabled = ((MDB_BUFFER[5].data >> 5) & 1) & ((ReaderPeriphID_Data[index].OptionalFeatureBits > 5) & 1);
			unsigned char buff[24];
			sprintf(buff, "CD%d#ENOPTS#%d#%d#%d\r\n", index + 1, ReaderOptFeatures[index].NegativeVendEnabled, ReaderOptFeatures[index].DataEntryEnabled, ReaderOptFeatures[index].AlwaysIdleEnabled);
			DebugMessage(buff);
			ReaderSetIdle(index);
		break;
		default:
		// Unknown Subcommand from VMC
			DebugMessage("Error: unsupported [EXPANSION]\r\n");
			ReaderSetIdle(index);
		break;
	}
}

void Reader_Revalue(unsigned char index) {
	MDB_read();
	if(MDBReceiveErrorFlag) {
		Reader_Active_Command[index] = MDB_IDLE;
		Reader_Poll_Reply[index] = MDB_REPLY_ACK;
		DebugMessage("Error: Error on [REVALUE]\r\n");
		return;
	}
	Reader_Active_Command[index] = MDB_REVALUE;
	switch(MDB_BUFFER[1].data)
	{
		case 0:
		// REVALUE REQUEST
			if(!MDB_readBlock(3))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
				DebugMessage("Error: invalid checksum for [REQUEST]\r\n");
				return;
			}
			uint16_t revalue_amount = MDB_BUFFER[2].data << 8;
			revalue_amount |= MDB_BUFFER[3].data;
			double tmpamount = ReaderSetupData[index].Scale_Factor * (revalue_amount / pow(10, ReaderSetupData[index].Decimal_Places));
			if (tmpamount <= ReaderRevalueAmountLimit[index])
			{
				Reader_Poll_Reply[index] = MDB_REPLY_REVALUE_APPROVED;
				if (!MDB_SendReaderRevalueApproved(index))
				{
					DebugMessage("Error: no ACK received on [REVALUE APPROVED]\r\n");
				} else
				{
					ReaderSetIdle(index);
				}
			} else
			{
				Reader_Poll_Reply[index] = MDB_REPLY_REVALUE_DENIED;
				if (!MDB_SendReaderRevalueDenied(index))
				{
					DebugMessage("Error: no ACK received on [REVALUE DENY]\r\n");
				} else
				{
					ReaderSetIdle(index);
				}
			}
			DebugMessage("REVREQ\r\n");
		break;
		case 1:
		//REVALUE LIMIT REQUEST
			if(!MDB_readBlock(1))
			{
				Reader_Active_Command[index] = MDB_IDLE;
				Reader_Poll_Reply[index] = MDB_REPLY_REVALUE_LIMIT_AMOUNT;
				DebugMessage("Error: invalid checksum for [LIMIT REQUEST]\r\n");
				return;
			}
			if (!MDB_SendReaderRevalueAmountLimit(index))
			{
				DebugMessage("Error: no ACK received on [REVALUE LIMIT]\r\n");
			} else
			{
				ReaderSetIdle(index);
			}
		break;
		default:
		// Unknown Subcommand from VMC
			DebugMessage("Error: unsupported [EXPANSION]\r\n");
			ReaderSetIdle(index);
		break;
	}
}

void Reader_Vend(unsigned char index) {
	unsigned char readerAddress = (index) ? 0x10 : 0x60;
	unsigned char buff[32];
	MDB_getByte(&MDB_BUFFER[MDB_BUFFER_COUNT++]);
	//if(!MDBReceiveComplete || MDBReceiveErrorFlag) {
		//Reader_Active_Command[index] = MDB_IDLE;
		//Reader_Poll_Reply[index] = MDB_REPLY_ACK;
		//DebugMessage("Error: Error on [VEND]\r\n");
		//return;
	//}
	switch(MDB_BUFFER[1].data)
	{
		case 0:
		// VEND REQUEST
			{
				if(!MDB_readBlock(5))
				{
					DebugMessage("Error: invalid checksum for [VEND REQUEST]\r\n");
					return;
				}
				//MDB_ACK();
				//send request data to cashless hardware for authorization
				//unsigned char buff[6 + ReaderSetupData[index].Decimal_Places];
				MDB_SendReaderVendApproved(index);
				uint16_t amount = MDB_BUFFER[2].data << 8;
				amount |= MDB_BUFFER[3].data;
				uint16_t itemnumber = MDB_BUFFER[4].data << 8;
				itemnumber |= MDB_BUFFER[5].data;
				sprintf(buff, "CD%d#VENDREQ#", index + 1);
				DebugMessage(buff);
				double tmpamount = ReaderSetupData[index].Scale_Factor * (amount / pow(10, ReaderSetupData[index].Decimal_Places));
				
				dtostrf(tmpamount,0,ReaderSetupData[index].Decimal_Places,buff);
				DebugMessage(buff);
				DebugMessage("#");
				sprintf(buff, "%d#\r\n", itemnumber);
				DebugMessage(buff);
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
			}
			break;
		case 1:
		// VEND CANCEL
			{
				if(MDB_Receive() != 0x014)
				{
					DebugMessage("Error: invalid checksum for [CANCEL]\r\n");
					return;
				}
				if (!MDB_SendReaderVendDenied(index))
				{
					Reader_Poll_Reply[index] = MDB_REPLY_VEND_DENIED;
					DebugMessage("Error: no ACK received on [VEND DENY]\r\n");
					return;
				} 
				sprintf(buff, "CD%d#VENDCANCEL\r\n", index + 1);
				DebugMessage(buff);
				ReaderSetIdle(index);
			}
			break;
		case 2:
		// VEND SUCCESS
			if(!MDB_readBlock(3))
			{
				DebugMessage("Error: invalid checksum for [SUCCESS]\r\n");
				return;
			}
			uint16_t itemnumber = MDB_BUFFER[2].data << 8;
			//itemnumber |= MDB_BUFFER[3].data;
			MDB_ACK();
			sprintf(&buff, "CD%d#VENDSUCCESS#%d\r\n", index + 1, itemnumber);
			DebugMessage(buff);
			ReaderSetIdle(index);
			if (Reader_State[index] == MDB_NEGATIVE_VEND)
			{
				//Credit should be generated on the media upon final
				//reception of VEND SUCCESS to avoid unwanted credit in the system
				Reader_State[index] = MDB_SESSION_IDLE;
			}
			Reader_Poll_Reply[index] = MDB_REPLY_ACK;
			break;
		case 3:
		// VEND FAILURE
			{
				if(!MDB_readBlock(1))
				{
					DebugMessage("Error: invalid checksum for [FAILURE]\r\n");
					return;
				}
				//send failure to cashless hardware, Funds should
				//be refunded to user’s account.
				//no answer to poll until refund completed or failed
				sprintf(buff, "CD%d#VENDFAIL\r\n", index + 1);
				DebugMessage(buff);
				Reader_Poll_Reply[index] = MDB_REPLY_SILENCE;
			}
		break;
		case 4:
		// SESSION COMPLETE
			{
				if(!MDB_readBlock(1))
				{
					DebugMessage("Error: invalid checksum for [SESSION COMPLETE]\r\n");
					return;
				}
				Reader_Poll_Reply[index] = MDB_REPLY_END_SESSION;
				sprintf(buff, "CD%d#SCOMPLETE\r\n", index + 1);
				DebugMessage(buff);
				if (!MDB_SendReaderEndSession(index))
				{
					DebugMessage("Error: no ACK received on [END SESSION]\r\n");
				} else
				{
					ReaderSetIdle(index);
				}
				Reader_State[index] = MDB_ENABLED;
				Reader_Poll_Reply[index] = MDB_REPLY_ACK;
			}
		break;
		case 6:
		// NEGATIVE VEND REQUEST
			{
				if(!MDB_readBlock(5))
				{
					DebugMessage("Error: invalid checksum for [NEGATIVE VEND REQUEST]\r\n");
					return;
				}
				Reader_State[index] = MDB_NEGATIVE_VEND;
				if (ReaderOptFeatures[index].NegativeVendEnabled)
				{
					Reader_Poll_Reply[index] = MDB_REPLY_VEND_APPROVED;
					if (!MDB_SendReaderVendApproved(index))
					{
						DebugMessage("Error: no ACK received on [NEGATIVE VEND APPROVED]\r\n");
					} else
					{
						Reader_Poll_Reply[index] = MDB_REPLY_ACK;
					}
				} else
				{
					Reader_Poll_Reply[index] = MDB_REPLY_VEND_DENIED;
					if (!MDB_SendReaderVendDenied(index))
					{
						DebugMessage("Error: no ACK received on [NEGATIVE VEND DENIED]\r\n");
					} else
					{
						Reader_Poll_Reply[index] = MDB_REPLY_ACK;
					}
				}
				//The VMC is requesting negative vend
				//approval from the payment media reader before accepting the returned
				//product.
				sprintf(buff, "CD%d#NEGVENDREQ#\r\n", index + 1);
				DebugMessage(buff);
				//unsigned char buff[6 + ReaderSetupData[index].Decimal_Places];
				uint16_t amount = MDB_BUFFER[2].data << 8;
				amount |= MDB_BUFFER[3].data;
				uint16_t itemnumber = MDB_BUFFER[4].data << 8;
				itemnumber |= MDB_BUFFER[5].data;
				double tmpamount = ReaderSetupData[index].Scale_Factor * (amount / pow(10, ReaderSetupData[index].Decimal_Places));
				dtostrf(tmpamount,0,ReaderSetupData[index].Decimal_Places,buff);
				DebugMessage(buff);
				DebugMessage("#");
				sprintf(buff, "%d#\r\n", itemnumber);
				DebugMessage(buff);
			}
		break;
		default:
		// Unknown Subcommand from VMC
		{
			sprintf(buff, "CD%d#unsupported [VEND]\r\n", index + 1);
			DebugMessage(buff);
			ReaderSetIdle(index);
		}
		break;
	}
}

void Reader_Poll(unsigned char index) {
	unsigned char readerAddress = (index) ? 0x10 : 0x60;
	if(!MDB_readBlock(1)) {
		MDB_BUFFER_COUNT = 0;
		ReaderSetIdle(index);
		DebugMessage("Error: invalid checksum for [POLL]\r\n");
		return;
	}
	switch(Reader_Poll_Reply[index])
	{
		case MDB_REPLY_ACK:
			MDB_ACK();
			//DebugMessage("POLL\r\n");
			break;
			case MDB_REPLY_JUST_RESET:
			if (!MDB_SendReaderJustReset())
			{
				DebugMessage("Error: no ACK received on [JUST RESET]");
			}
			unsigned char buff[24];
			sprintf(buff, "CD%d#JUSTRESET\r\n", index + 1);
			DebugMessage(buff);
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_READER_CFG:
			if (!MDB_SendReaderSetupData(index))
			{
				DebugMessage("Error: no ACK received on [SETUP]");
			}
			else
			{
				ReaderSetIdle(index);
			}
			DebugMessage("SETUP\r\n");
		break;
		case MDB_REPLY_DISPLAY_REQ:
			if (!MDB_SendReaderDisplayRequest(index))
			{
				DebugMessage("Error: no ACK received on [DISPLAY REQ]");
				return;
			}
			ReaderSetIdle(index);
			ReaderDisplayRequestData[index].DisplayTime = 0;
			for (int i = 0; i < 32; i++)
			{
				ReaderDisplayRequestData[index].DisplayData[i] = 0;
			}
			DebugMessage("DISPREQ\r\n");
		break;
		case MDB_REPLY_BEGIN_SESSION:
			if (!MDB_SendReaderBeginSession(index))
			{
				DebugMessage("Error: no ACK received on [START SESSION]");
				return;
			}
			DebugMessage("STARTSESSION\r\n");
			ReaderSessionData[index].StartData.Funds = 0;
			Reader_State[index] = MDB_SESSION_IDLE;
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_SESSION_CANCEL_REQ:
			if (!MDB_SendReaderCancelSessionRequest(index))
			{
				DebugMessage("Error: no ACK received on [SESSION CANCEL REQ]");
				return;
			}
			DebugMessage("SCANCELREQ\r\n");
			ReaderSessionData[index].StartData.Funds = 0;
			Reader_State[index] = MDB_ENABLED;
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_VEND_APPROVED:
			if (!MDB_SendReaderCancelSessionRequest(index))
			{
				DebugMessage("Error: no ACK received on [VEND APPROVED]");
				return;
			}
			DebugMessage("VENDAPPROVED\r\n");
			ReaderSessionData[index].ResultData.Result = VEND_UNKNOWN;
			ReaderSessionData[index].ResultData.Amount = 0;
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_VEND_DENIED:
			if (!MDB_SendReaderVendDenied(index))
			{
				DebugMessage("Error: no ACK received on [VEND DENIED]");
				return;
			}
			DebugMessage("VENDDENY\r\n");
			ReaderSessionData[index].StartData.Funds = 0;
			ReaderSessionData[index].ResultData.Result = VEND_UNKNOWN;
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_END_SESSION:
			if (!MDB_SendReaderEndSession(index))
			{
				DebugMessage("Error: no ACK received on [END SESSION]");
				return;
			}
			DebugMessage("ENDSESSION\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_CANCELLED:
			if (!MDB_SendReaderCancelled(index))
			{
				DebugMessage("Error: no ACK received on [REPLY CANCELED]");
				return;
			}
			DebugMessage("CANCELED\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_PERIPHERIAL_ID:
			if (!MDB_SendReaderPeriphIDData(index))
			{
				DebugMessage("Error: no ACK received on [REPLY PERIPHERIAL ID]");
				return;
			}
			DebugMessage("PERID\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_ERROR:
			if (!MDB_SendReaderError(index))
			{
				DebugMessage("Error: no ACK received on [REPLY ERROR]");
				return;
			}
			DebugMessage("ERROR\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_CMD_OUT_SEQUENCE:
			if (!MDB_SendReaderCMDOutOfSequence(index))
			{
				DebugMessage("Error: no ACK received on [REPLY CMD_OUT OF SEQUENCE]");
				return;
			}
			DebugMessage("CMDOUTOFSEQ\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_REVALUE_APPROVED:
			if (!MDB_SendReaderRevalueApproved(index))
			{
				DebugMessage("Error: no ACK received on [REVALUE APPROVED]");
				return;
			}
			DebugMessage("REVAPPROVED\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_REVALUE_DENIED:
			if (!MDB_SendReaderRevalueDenied(index))
			{
				DebugMessage("Error: no ACK received on [REVALUE DENIED]");
				return;
			}
			DebugMessage("[REVALUE DENIED]\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_REVALUE_LIMIT_AMOUNT:
			if (!MDB_SendReaderRevalueAmountLimit(index))
			{
				DebugMessage("Error: no ACK received on [REVALUE LIMIT AMOUNT]");
				return;
			}
			DebugMessage("REVLIMIT\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_TIMEDATE_REQUEST:
			if (!MDB_SendReaderDateTimeRequest(index))
			{
				DebugMessage("Error: no ACK received on [TIMEDATE REQUEST]");
				return;
			}
			DebugMessage("TIMEDATEREQ\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_DATAENTRY_REQUEST:
			if (!MDB_SendReaderDataEntryRequest(index))
			{
				DebugMessage("Error: no ACK received on [DATAENTRY REQUEST]");
				return;
			}
			DebugMessage("DATAREQ\r\n");
			ReaderSetIdle(index);
		break;
		case MDB_REPLY_DATAENTRY_CANCEL:
			if (!MDB_SendReaderDataEntryCancel(index))
			{
				DebugMessage("Error: no ACK received on [DATAENTRY CANCEL]");
				return;
			}
			DebugMessage("DATACANCEL\r\n");
			ReaderSetIdle(index);
		break;
	}
}

