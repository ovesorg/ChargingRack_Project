#include"main.h"


union BitGroup_TypeDef g_CanGRWTEvent;
union BitGroup_TypeDef g_CanGRWTEvent1;


GRWT_BMS0X311_TypeDef g_bms0x311;
GRWT_BMS0X312_TypeDef g_bms0x312;
GRWT_BMS0X313_TypeDef g_bms0x313;
GRWT_BMS0X314_TypeDef g_bms0x314;
GRWT_BMS0X319_TypeDef g_bms0x319;

GRWT_CELL0X315_TypeDef g_bmsCell0x315;
GRWT_CELL0X316_TypeDef g_bmsCell0x316;
GRWT_CELL0X317_TypeDef g_bmsCell0x317;
GRWT_CELL0X318_TypeDef g_bmsCell0x318;


GRWT_WDG_REFRESH_TypeDef g_bmsWdg0x400;

//const uint8_t g_CanWakeCmd[8]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
const uint8_t g_CanWakeCmd[8]={0x0B,0x16,0x21,0x2C,0x37,0x42,0x4D,0x58};

extern CAN_TXSTATE_TypeDef g_CanTransmitState;


void CanGrowattRamInit(void)
{
	g_CanGRWTEvent.BYTE=0;
	g_CanGRWTEvent1.BYTE=0;

	memset((uint8_t*)&g_CanTransmitState,0x00,sizeof(g_CanTransmitState));
	memset((uint8_t*)&g_bms0x311,0x00,sizeof(g_bms0x311));
	memset((uint8_t*)&g_bms0x312,0x00,sizeof(g_bms0x312));
	memset((uint8_t*)&g_bms0x313,0x00,sizeof(g_bms0x313));
	memset((uint8_t*)&g_bms0x314,0x00,sizeof(g_bms0x314));
	memset((uint8_t*)&g_bms0x319,0x00,sizeof(g_bms0x319));
	
	memset((uint8_t*)&g_bmsCell0x315,0x00,sizeof(g_bmsCell0x315));
	memset((uint8_t*)&g_bmsCell0x316,0x00,sizeof(g_bmsCell0x316));
	memset((uint8_t*)&g_bmsCell0x317,0x00,sizeof(g_bmsCell0x317));
	memset((uint8_t*)&g_bmsCell0x318,0x00,sizeof(g_bmsCell0x318));
	memset((uint8_t*)&g_bmsWdg0x400,0x00,sizeof(g_bmsWdg0x400));

}


void CanGrowattParse(uint32_t id,uint8_t *data,uint8_t len)
{
	switch(id)
	{
		case 0x311:
			memcpy((uint8_t*)&g_bms0x311,data,len);
			BmsGrwt0X311Event=TRUE;
			break;
		case 0x312:
			memcpy((uint8_t*)&g_bms0x312,data,len);
			BmsGrwt0X312Event=TRUE;
			break;
		case 0x313:
			memcpy((uint8_t*)&g_bms0x313,data,len);
			BmsGrwt0X313Event=TRUE;
			break;
		case 0x314:
			memcpy((uint8_t*)&g_bms0x314,data,len);
			BmsGrwt0X314Event=TRUE;
			break;	
		case 0x315:
			memcpy((uint8_t*)&g_bmsCell0x315,data,len);
			BmsGrwt0X315Event=TRUE;
			break;
		case 0x316:
			memcpy((uint8_t*)&g_bmsCell0x316,data,len);
			BmsGrwt0X316Event=TRUE;
			break;
		case 0x317:
			memcpy((uint8_t*)&g_bmsCell0x317,data,len);
			BmsGrwt0X317Event=TRUE;
			break;
		case 0x318:
			memcpy((uint8_t*)&g_bmsCell0x318,data,len);
			BmsGrwt0X318Event=TRUE;
			break;
		case 0x319:
			memcpy((uint8_t*)&g_bms0x319,data,len);
			BmsGrwt0X319Event=TRUE;
			break;	
		}
	
}


void CanTransmitStd(uint32_t id,uint8_t *data,uint8_t len)
{
	can_trasnmit_message_struct transmit_message;
	
	can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
    transmit_message.tx_sfid = id;
    transmit_message.tx_efid = 0x00;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = CAN_FF_STANDARD;
    transmit_message.tx_dlen = 8;
	memcpy(transmit_message.tx_data,data,len);
	can_message_transmit(CAN0, &transmit_message);
}

void CanGrowattProc(void)
{
	uint16_t temp16;
	uint32_t temp32;
	int32_t tempint32;
    int16_t tempInt16;
	uint8_t *p_u8;
	uint32_t power;

	#ifdef GROWATT_BMS
		
    if(HAL_GetTick()-g_CanTransmitState.t1000ms>=1000)
    {
    	
    	g_CanTransmitState.t1000ms=HAL_GetTick();
		
		if(PaygGetPayState()||PaygGetFreeState()||GetDemoState())
		{	g_bmsWdg0x400.Wdgrefresh=1;
	
		    g_bmsWdg0x400.Heartbeat=1;
		    g_bmsWdg0x400.WdgCounter=0x00ff; 
			}
		else
		{	
			g_bmsWdg0x400.Wdgrefresh=0;
			g_bmsWdg0x400.Heartbeat=0;
			g_bmsWdg0x400.WdgCounter=0x00ff;
			}
		
		CanTransmitStd(0x400,(uint8_t*)&g_bmsWdg0x400,8);
		HAL_Delay(10);
		CanTransmitStd(0x301,(uint8_t*)&g_CanWakeCmd,8);
    	}

    if(HAL_GetTick()-g_CanTransmitState.t2000ms>=5000) //5s
    {
    	g_CanTransmitState.t2000ms=HAL_GetTick();

		/*if(PaygGetFreeState())
			g_Vcu_0x00632.WatchdogEnable=FALSE;
		else
			g_Vcu_0x00632.WatchdogEnable=TRUE;*/
		
    	}

	if(BmsGrwt0X311Event)
	{
		BmsGrwt0X311Event=FALSE;
		}
	if(BmsGrwt0X312Event)
	{
		memcpy((uint8_t*)&temp16,(uint8_t*)&g_bms0x312,2);
		GattSetData(LIST_DTA,DTA_PPST,(uint8_t*)&temp16);
		BmsGrwt0X312Event=FALSE;
		}
	if(BmsGrwt0X313Event)
	{
		temp16=(g_bms0x313.AverageVoltageH<<8)+g_bms0x313.AverageVoltageL;
		temp16*=10;
		GattSetData(LIST_DTA,DTA_PCKV,(uint8_t*)&temp16);
		GattSetData(LIST_DTA,DTA_RVLT,(uint8_t*)&temp16);

		power=temp16;//voltage

		temp16=(g_bms0x313.TotalCurrentH<<8)+g_bms0x313.TotalCurrentL;

		memcpy((uint8_t*)&tempInt16,(uint8_t*)&temp16,2);
		
		tempInt16=tempInt16/10;
		GattSetData(LIST_DTA,DTA_PCKC,(uint8_t*)&tempInt16);
		GattSetData(LIST_DTA,DTA_RCUR,(uint8_t*)&tempInt16);

		if(tempInt16>=0)
		{
			power=power*tempInt16/1000;
			temp16=power;
			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			}
		else
		{	
			tempInt16*=-1;
			power=power*tempInt16/1000;
			temp16=power;
			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
			}
		

		temp16=((g_bms0x313.MaxTemperatureH<<8)+g_bms0x313.MaxTemperatureL)/10;
		GattSetData(LIST_DTA,DTA_PCKT,(uint8_t*)&temp16);

		
		temp16=g_bms0x313.SOC;
		GattSetData(LIST_DTA,DTA_RSOC,(uint8_t*)&temp16);

		
		
		BmsGrwt0X313Event=FALSE;
		}
	if(BmsGrwt0X314Event)
	{
		temp16=(g_bms0x314.CurrentCapacityH<<8)+g_bms0x314.CurrentCapacityL;
		temp16/=100;
		GattSetData(LIST_DTA,DTA_RCAP,(uint8_t*)&temp16);

		temp16=(g_bms0x314.GaugeFCCH<<8)+g_bms0x314.GaugeFCCL;
		temp16/=100;
		GattSetData(LIST_DTA,DTA_FCCP,(uint8_t*)&temp16);

		temp16=(g_bms0x314.CycleCountH<<8)+g_bms0x314.CycleCountL;
		//temp16;
		GattSetData(LIST_DTA,DTA_ACYC,(uint8_t*)&temp16);

		temp16=temp16*(((g_bms0x314.GaugeFCCH<<8)+g_bms0x314.GaugeFCCL)/100);
		GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&temp16);
		
		BmsGrwt0X314Event=FALSE;
		}
	
	if(BmsGrwt0X315Event)
	{
		temp16=(g_bmsCell0x315.CellVoltage1H<<8)+g_bmsCell0x315.CellVoltage1L;
		GattSetData(LIST_DIA,DIA_CV01,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x315.CellVoltage2H<<8)+g_bmsCell0x315.CellVoltage2L;
		GattSetData(LIST_DIA,DIA_CV02,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x315.CellVoltage3H<<8)+g_bmsCell0x315.CellVoltage3L;
		GattSetData(LIST_DIA,DIA_CV03,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x315.CellVoltage4H<<8)+g_bmsCell0x315.CellVoltage4L;
		GattSetData(LIST_DIA,DIA_CV04,(uint8_t*)&temp16);
		
		BmsGrwt0X315Event=FALSE;
		}
	if(BmsGrwt0X316Event)
	{
		temp16=(g_bmsCell0x316.CellVoltage5H<<8)+g_bmsCell0x316.CellVoltage5L;
		GattSetData(LIST_DIA,DIA_CV05,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x316.CellVoltage6H<<8)+g_bmsCell0x316.CellVoltage6L;
		GattSetData(LIST_DIA,DIA_CV06,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x316.CellVoltage7H<<8)+g_bmsCell0x316.CellVoltage7L;
		GattSetData(LIST_DIA,DIA_CV07,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x316.CellVoltage8H<<8)+g_bmsCell0x316.CellVoltage8L;
		GattSetData(LIST_DIA,DIA_CV08,(uint8_t*)&temp16);
		BmsGrwt0X316Event=FALSE;
		}
	if(BmsGrwt0X317Event)
	{
		temp16=(g_bmsCell0x317.CellVoltage9H<<8)+g_bmsCell0x317.CellVoltage9L;
		GattSetData(LIST_DIA,DIA_CV09,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x317.CellVoltage10H<<8)+g_bmsCell0x317.CellVoltage10L;
		GattSetData(LIST_DIA,DIA_CV10,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x317.CellVoltage11H<<8)+g_bmsCell0x317.CellVoltage11L;
		GattSetData(LIST_DIA,DIA_CV11,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x317.CellVoltage12H<<8)+g_bmsCell0x317.CellVoltage12L;
		GattSetData(LIST_DIA,DIA_CV12,(uint8_t*)&temp16);
		BmsGrwt0X317Event=FALSE;
		}
	if(BmsGrwt0X318Event)
	{
		temp16=(g_bmsCell0x318.CellVoltage13H<<8)+g_bmsCell0x318.CellVoltage13L;
		GattSetData(LIST_DIA,DIA_CV13,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x318.CellVoltage14H<<8)+g_bmsCell0x318.CellVoltage14L;
		GattSetData(LIST_DIA,DIA_CV14,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x318.CellVoltage15H<<8)+g_bmsCell0x318.CellVoltage15L;
		GattSetData(LIST_DIA,DIA_CV15,(uint8_t*)&temp16);
		temp16=(g_bmsCell0x318.CellVoltage16H<<8)+g_bmsCell0x318.CellVoltage16L;
		GattSetData(LIST_DIA,DIA_CV16,(uint8_t*)&temp16);
		BmsGrwt0X318Event=FALSE;
	}

	if(BmsGrwt0X319Event)
	{
		BmsGrwt0X319Event=FALSE;
	}

	#endif
}




