
#ifndef __CAN_GROWATT_H__

#define __CAN_GROWATT_H__




#define BmsGrwt0X315Event     g_CanGRWTEvent.BIT.B0
#define BmsGrwt0X316Event     g_CanGRWTEvent.BIT.B1
#define BmsGrwt0X317Event     g_CanGRWTEvent.BIT.B2
#define BmsGrwt0X318Event    g_CanGRWTEvent.BIT.B3

#define BmsGrwt0X311Event     g_CanGRWTEvent.BIT.B4
#define BmsGrwt0X312Event     g_CanGRWTEvent.BIT.B5
#define BmsGrwt0X313Event     g_CanGRWTEvent.BIT.B6
#define BmsGrwt0X314Event    g_CanGRWTEvent.BIT.B7

#define BmsGrwt0X319Event    g_CanGRWTEvent1.BIT.B0


typedef struct
{
   //01.v
	uint8_t BatChargeVoltageH;
	uint8_t BatChargeVoltageL;
	//0.1A
	uint8_t ChrgeCurrentLimitH;
	uint8_t ChrgeCurrentLimitL;

	uint8_t DischrgeCurrentLimitH;
	uint8_t DischrgeCurrentLimitL;

	uint8_t MasterBoxOpMOde:2;
	uint8_t SP_Status:2;
	uint8_t Reserved:4;
	
	uint8_t state:2;
	uint8_t Err_bit_flag:1;
	uint8_t cell_blance_state:1;
	uint8_t Sleep_status:1;
	uint8_t OutputDischargeStatus:1;
	uint8_t OutputChargeStatus:1;
	uint8_t BatTerminalStatus:1;
	

}GRWT_BMS0X311_TypeDef;

typedef struct
{
   //Protection
	uint8_t SoftStartFail:1;
    uint8_t ModuleUnderVoltage:1;
	uint8_t ModuleOverVoltage:1;
	uint8_t CellUnderVoltage:1;
	uint8_t CellOverVoltage:1;
	uint8_t SCD_Protection:1;//Short circuit discharge protection
	uint8_t ChargeOverCurrent:1;
	uint8_t DisChargeCurrent:1;
	
	uint8_t Reserved1_bit01:2;
	uint8_t DetalVFail:1;
	uint8_t SystemError:1;
	uint8_t UTC_protection:1; //under temperature Charge 
	uint8_t UTD_Protection:1; //Under Temperature Discharge
	uint8_t OTC_Protection:1;
	uint8_t OTD_Protection:1;

	//warn
	uint8_t WReserved2_bit0:1;
	uint8_t WModuleUnderVoltage:1;
	uint8_t WModuleOverVoltage:1;
	uint8_t WCellUnderVoltage:1; //under temperature Charge 
	uint8_t WCellOverVoltage:1; //Under Temperature Discharge
	uint8_t WReserved2_bit5:1;
	uint8_t WChargeOverCurrent:1;
	uint8_t WDischargeOverCurrent:1;

	uint8_t WInternalCommFail:1;
	uint8_t WPackBeforTurnoff:1;
	uint8_t WDeltaVfail:1;
	uint8_t WReserve3_bit4:1; //under temperature Charge 
	uint8_t WUTC_Protection:1; //Under Temperature Discharge
	uint8_t WUTD_Protection:1;
	uint8_t WOTC_Protection:1;
	uint8_t WOTD_Protection:1;

	uint8_t PackNumber;
	uint8_t manu_code1;

	uint8_t manu_code2;
	uint8_t totalCellNumber;

}GRWT_BMS0X312_TypeDef;

typedef struct
{
	uint8_t AverageVoltageH;//0.01v
	uint8_t AverageVoltageL;

	uint8_t TotalCurrentH;  //0.1A
	uint8_t TotalCurrentL;

	uint8_t MaxTemperatureH; //0.1C
	uint8_t MaxTemperatureL;

	uint8_t SOC; //1%
	uint8_t SOH; //bit7 soh flag

}GRWT_BMS0X313_TypeDef;

typedef struct
{
	uint8_t CurrentCapacityH;//10mAH
	uint8_t CurrentCapacityL;

	uint8_t GaugeFCCH;  //10mAH
	uint8_t GaugeFCCL;

	uint8_t Delta_VH; //1mV
	uint8_t Delta_VL;

	uint8_t CycleCountH; //hour
	uint8_t CycleCountL;

}GRWT_BMS0X314_TypeDef;



typedef struct
{
   //mV
	uint8_t CellVoltage1H;
	uint8_t CellVoltage1L;

	uint8_t CellVoltage2H;
	uint8_t CellVoltage2L;

	uint8_t CellVoltage3H;
	uint8_t CellVoltage3L;

	uint8_t CellVoltage4H;
	uint8_t CellVoltage4L;

}GRWT_CELL0X315_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage5H;
	uint8_t CellVoltage5L;

	uint8_t CellVoltage6H;
	uint8_t CellVoltage6L;

	uint8_t CellVoltage7H;
	uint8_t CellVoltage7L;

	uint8_t CellVoltage8H;
	uint8_t CellVoltage8L;

}GRWT_CELL0X316_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage9H;
	uint8_t CellVoltage9L;

	uint8_t CellVoltage10H;
	uint8_t CellVoltage10L;

	uint8_t CellVoltage11H;
	uint8_t CellVoltage11L;

	uint8_t CellVoltage12H;
	uint8_t CellVoltage12L;

}GRWT_CELL0X317_TypeDef;

typedef struct
{
   //mV
	uint8_t CellVoltage13H;
	uint8_t CellVoltage13L;

	uint8_t CellVoltage14H;
	uint8_t CellVoltage14L;

	uint8_t CellVoltage15H;
	uint8_t CellVoltage15L;

	uint8_t CellVoltage16H;
	uint8_t CellVoltage16L;

}GRWT_CELL0X318_TypeDef;



typedef struct
{
	uint8_t CellType:2;
	uint8_t Reseved0_Bit23:2;
	uint8_t ForceChargeMark2:1;
	uint8_t ForceChargeMark1:1;
	uint8_t DischargeEnable:1;
	uint8_t ChargeEnable:1;
	
	uint8_t MaxCellVoltage;  //1mv
	uint8_t Reserved2;
	uint8_t MinCellVoltage;  //1mv

	uint8_t Reserved4; 
	uint8_t MaxCellNumber;

	uint8_t MinCellNumber; 
	uint8_t ProtectPackID;

}GRWT_BMS0X319_TypeDef;


typedef struct
{
	uint8_t Wdgrefresh:1;
	uint8_t reserved:7;

	uint8_t Heartbeat:1;
	uint8_t reserved1:7;

	uint8_t WdgCounter;//RESERVED

	uint8_t Reserved2[5];
}GRWT_WDG_REFRESH_TypeDef;


void CanGrowattRamInit(void);
void CanGrowattProc(void);
void CanGrowattParse(uint32_t id,uint8_t *data,uint8_t len);
void CanTransmitStd(uint32_t id,uint8_t *data,uint8_t len);

#endif


