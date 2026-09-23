#include"main.h"


__IO uint8_t g_PlcComEnable=0;
__IO uint8_t g_PlcComIndex=0;
__IO uint8_t g_ModbusState=MODBUS_IDLE;

PLC_CELLINFOR_TypeDef g_plcCellinfor;



extern UART_HandleTypeDef huart3;
extern __IO uint8_t g_Uart485Buf[UART3_RX_BUF_SIZE];

PLC_BATFRAME_TypeDef g_ChargeFrameInfor;

#ifdef CHARGE_STATION
extern SLOT_BATINFOR_TypeDef g_SlotBmsInfor[CHARGE_NUM];
#endif



const PLC_ADDR_TypeDef  g_PlcAddrTable[]=
{
	
	{PLCADDR_PHASE_A_VOLT,2,(uint8_t*)&g_ChargeFrameInfor.A_InputVoltage},
	{PLCADDR_PHASE_A_CURRENT,2,(uint8_t*)&g_ChargeFrameInfor.A_InputCurrent},
	
	{PLCADDR_PHASE_B_VOLT,2,(uint8_t*)&g_ChargeFrameInfor.B_InputVoltage},
	{PLCADDR_PHASE_B_CURRENT,2,(uint8_t*)&g_ChargeFrameInfor.B_InputCurrent},
	
	{PLCADDR_PHASE_C_VOLT,2,(uint8_t*)&g_ChargeFrameInfor.C_InputVoltage},
	{PLCADDR_PHASE_C_CURRENT,2,(uint8_t*)&g_ChargeFrameInfor.C_InputCurrent},
	
	{PLCADDR_CHARGE_POWER,2,(uint8_t*)&g_ChargeFrameInfor.TotalChargePower},
	{PLCADDR_DAY_DISCHARGE_POWER,2,(uint8_t*)&g_ChargeFrameInfor.DayPower},
	
	{PLCADDR_MONTH_DISCHARGE_POWER,2,(uint8_t*)&g_ChargeFrameInfor.MonthPower},
	{PLCADDR_TOTAL_DISCHARGE_POWER,2,(uint8_t*)&g_ChargeFrameInfor.TotalPower},//9

	#if(CHARGE_NUM == 15)
	
	{PLCADDR_A_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].BatteryID},
	{PLCADDR_A_1_SOC,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].Soc},
	{PLCADDR_A_1_REMAIN,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].RemainCap},
	{PLCADDR_A_1_CHARGE_VOLTAGE,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].ChargeVoltage},
	{PLCADDR_A_1_CHARGE_CURRENT,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].ChargeCurrent},

	{PLCADDR_A_2_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[1].BatteryID},
	{PLCADDR_A_2_SOC,1,NULL},
	{PLCADDR_A_2_REMAIN,1,NULL},
	{PLCADDR_A_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_A_2_CHARGE_CURRENT,1,NULL},

	{PLCADDR_A_3_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[2].BatteryID},
	{PLCADDR_A_3_SOC,1,NULL},
	{PLCADDR_A_3_REMAIN,1,NULL},
	{PLCADDR_A_3_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_A_3_CHARGE_CURRENT,1,NULL},

	{PLCADDR_A_4_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[3].BatteryID},
	{PLCADDR_A_4_SOC,1,NULL},
	{PLCADDR_A_4_REMAIN,1,NULL},
	{PLCADDR_A_4_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_A_4_CHARGE_CURRENT,1,NULL},

	{PLCADDR_A_5_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[4].BatteryID},
	{PLCADDR_A_5_SOC,1,NULL},
	{PLCADDR_A_5_REMAIN,1,NULL},
	{PLCADDR_A_5_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_A_5_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[5].BatteryID},
	{PLCADDR_B_1_SOC,1,NULL},
	{PLCADDR_B_1_REMAIN,1,NULL},
	{PLCADDR_B_1_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_1_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_2_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[6].BatteryID},
	{PLCADDR_B_2_SOC,1,NULL},
	{PLCADDR_B_2_REMAIN,1,NULL},
	{PLCADDR_B_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_2_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_3_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[7].BatteryID},
	{PLCADDR_B_3_SOC,1,NULL},
	{PLCADDR_B_3_REMAIN,1,NULL},
	{PLCADDR_B_3_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_3_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_4_BATT_ID,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[8].BatteryID},
	{PLCADDR_B_4_SOC,1,NULL},
	{PLCADDR_B_4_REMAIN,1,NULL},
	{PLCADDR_B_4_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_4_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_5_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[9].BatteryID},
	{PLCADDR_B_5_SOC,1,NULL},
	{PLCADDR_B_5_REMAIN,1,NULL},
	{PLCADDR_B_5_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_5_CHARGE_CURRENT,1,NULL},
	
	{PLCADDR_C_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[10].BatteryID},
	{PLCADDR_C_1_SOC,1,NULL},
	{PLCADDR_C_1_REMAIN,1,NULL},
	{PLCADDR_C_1_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_1_CHARGE_CURRENT,1,NULL},

	{PLCADDR_C_2_BATT_ID,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[11].BatteryID},
	{PLCADDR_C_2_SOC,1,NULL},
	{PLCADDR_C_2_REMAIN,1,NULL},
	{PLCADDR_C_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_2_CHARGE_CURRENT,1,NULL},

	{PLCADDR_C_3_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[12].BatteryID},
	{PLCADDR_C_3_SOC,1,NULL},
	{PLCADDR_C_3_REMAIN,1,NULL},
	{PLCADDR_C_3_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_3_CHARGE_CURRENT,1,NULL},

	{PLCADDR_C_4_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[13].BatteryID},
	{PLCADDR_C_4_SOC,1,NULL},
	{PLCADDR_C_4_REMAIN,1,NULL},
	{PLCADDR_C_4_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_4_CHARGE_CURRENT,1,NULL},

	{PLCADDR_C_5_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[14].BatteryID},
	{PLCADDR_C_5_SOC,1,NULL},
	{PLCADDR_C_5_REMAIN,1,NULL},
	{PLCADDR_C_5_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_5_CHARGE_CURRENT,1,NULL},

	
	{PLCADDR_A_1_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_2_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_3_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_4_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_5_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_6_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_7_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_8_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_9_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_10_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},

	{PLCADDR_A_11_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_12_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_13_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_14_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_15_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},

	
	#else
	{PLCADDR_A_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].BatteryID},
	{PLCADDR_A_1_SOC,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].Soc},
	{PLCADDR_A_1_REMAIN,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].RemainCap},
	{PLCADDR_A_1_CHARGE_VOLTAGE,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].ChargeVoltage},
	{PLCADDR_A_1_CHARGE_CURRENT,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[0].ChargeCurrent},

	{PLCADDR_A_2_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[1].BatteryID},
	{PLCADDR_A_2_SOC,1,NULL},
	{PLCADDR_A_2_REMAIN,1,NULL},
	{PLCADDR_A_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_A_2_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[2].BatteryID},
	{PLCADDR_B_1_SOC,1,NULL},
	{PLCADDR_B_1_REMAIN,1,NULL},
	{PLCADDR_B_1_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_1_CHARGE_CURRENT,1,NULL},

	{PLCADDR_B_2_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[3].BatteryID},
	{PLCADDR_B_2_SOC,1,NULL},
	{PLCADDR_B_2_REMAIN,1,NULL},
	{PLCADDR_B_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_B_2_CHARGE_CURRENT,1,NULL},
	
	{PLCADDR_C_1_BATT_ID,2,(uint8_t*)&g_ChargeFrameInfor.BatInfor[4].BatteryID},
	{PLCADDR_C_1_SOC,1,NULL},
	{PLCADDR_C_1_REMAIN,1,NULL},
	{PLCADDR_C_1_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_1_CHARGE_CURRENT,1,NULL},

	{PLCADDR_C_2_BATT_ID,1,(uint8_t*)&g_ChargeFrameInfor.BatInfor[5].BatteryID},
	{PLCADDR_C_2_SOC,1,NULL},
	{PLCADDR_C_2_REMAIN,1,NULL},
	{PLCADDR_C_2_CHARGE_VOLTAGE,1,NULL},
	{PLCADDR_C_2_CHARGE_CURRENT,1,NULL},
	
	
	{PLCADDR_A_1_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_2_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_6_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_7_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_11_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	{PLCADDR_A_12_CELL_VOLT,1,(uint8_t*)&g_plcCellinfor},
	#endif
};

void PlcInit(void)
{

	g_PlcComEnable=0;

	memset((uint8_t*)&g_ChargeFrameInfor,0X00,sizeof(g_ChargeFrameInfor));

	//g_ChargeFrameInfor.A_InputVoltage=0x0102;

	g_ModbusState=MODBUS_EMETER;
}


void PlcEmeterInforUpdate(EMETER_INFOR_TypeDef *pemeter)
{
	float tempfloat;
	
	tempfloat=(float)pemeter->phase_a_voltage/10.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.A_InputVoltage);
	//printf("g_ChargeFrameInfor.A_InputVoltage  =%f %x\r\n",tempfloat ,g_ChargeFrameInfor.A_InputVoltage);
	
	tempfloat=(float)pemeter->phase_b_voltage/10.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.B_InputVoltage);
	
	
	tempfloat=(float)pemeter->phase_c_voltage/10.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.C_InputVoltage);
	
	tempfloat=(float)pemeter->phase_a_current/1000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.A_InputCurrent);
	//printf("g_ChargeFrameInfor.A_InputCurrent  =%f %x\r\n",tempfloat ,g_ChargeFrameInfor.A_InputCurrent);
	
	tempfloat=(float)pemeter->phase_b_current/1000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.B_InputCurrent);
	tempfloat=(float)pemeter->phase_c_current/1000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.C_InputCurrent);
	

	tempfloat=(float)pemeter->total_power/10000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.TotalChargePower);
	//printf("g_ChargeFrameInfor.TotalChargePower  =%f %x\r\n",tempfloat ,g_ChargeFrameInfor.TotalChargePower);
	

	tempfloat=(float)pemeter->current_energy/100.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)&g_ChargeFrameInfor.DayPower);
		//printf("g_ChargeFrameInfor.DayPower  =%f %x\r\n",tempfloat ,g_ChargeFrameInfor.DayPower);
}



void PlcBmsInforUpdate(uint8_t bat_no,PLC_BATINFOR_TypeDef *pbms)
{
	float tempfloat;
	uint32_t *p;

	//tempfloat=pbms->RemainCap/100;
	p=(uint32_t*)&pbms->ChargeVoltage;
	tempfloat=(float)*p/1000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)g_ChargeFrameInfor.BatInfor[bat_no].ChargeVoltage);
	p=(uint32_t*)&pbms->ChargeCurrent;
	tempfloat=(float)*p/10.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)g_ChargeFrameInfor.BatInfor[bat_no].ChargeCurrent);
	
	g_ChargeFrameInfor.BatInfor[bat_no].Soc=pbms->Soc<<8;

	p=(uint32_t*)&pbms->RemainCap;
	tempfloat=(float)*p/1000.0f/1000.0f;
	FloatToMbUint8(tempfloat,(uint8_t*)g_ChargeFrameInfor.BatInfor[bat_no].RemainCap);
	
	//memcpy((uint8_t*)&g_ChargeFrameInfor.BatInfor[bat_no],pbms,sizeof(PLC_BATINFOR_TypeDef));
}

void PlcBmsBatIdUpdate(uint8_t bat_no,uint32_t batId)
{
	//memcpy((uint8_t*)&g_ChargeFrameInfor.BatInfor[bat_no].BatteryID,(uint8_t*)&batId,4);
	g_ChargeFrameInfor.BatInfor[bat_no].BatteryID=Uint32Uint8ToMb((uint8_t*)&batId);
}

void PlcBmsSocUpdate(uint8_t bat_no,uint16_t soc)
{
	g_ChargeFrameInfor.BatInfor[bat_no].Soc=soc;
}

void PlcBmsRemainCapUpdate(uint8_t bat_no,uint32_t remain)
{
	memcpy((uint8_t*)&g_ChargeFrameInfor.BatInfor[bat_no].RemainCap,(uint8_t*)&remain,4);
}

void PlcBmsChargeVoltageUpdate(uint8_t bat_no,uint16_t voltage)
{
	//g_ChargeFrameInfor.BatInfor[bat_no].ChargeVoltage=voltage;
}
void PlcBmsChargeCurrentUpdate(uint8_t bat_no,uint16_t current)
{
	//g_ChargeFrameInfor.BatInfor[bat_no].ChargeCurrent=current;
}


void PlcComEnable(void)
{

	g_PlcComEnable=TRUE;

}


uint8_t getModbusState(void)
{
return g_ModbusState;
}

 void setModbusState(uint8_t flag)
{
  g_ModbusState=flag;
}
void PlcProc(void)
{
 
//	ModbusWrite(0x02,0x06,g_PlcAddrTable[g_PlcComIndex].Address,((uint16_t*)g_PlcAddrTable[g_PlcComIndex].data)[0]);
	
	
	if(g_PlcComEnable)
	{
		g_PlcComEnable=FALSE;

		if(g_ModbusState==MODBUS_PLC)
		{
		
			if(g_PlcAddrTable[g_PlcComIndex].Address <= PLCADDR_TOTAL_DISCHARGE_POWER)
			{	
				if(g_PlcAddrTable[g_PlcComIndex].data!=NULL)
				{
					if(g_PlcAddrTable[g_PlcComIndex].number==1)
						ModbusWrite(0x02,0x06,g_PlcAddrTable[g_PlcComIndex].Address,((uint16_t*)g_PlcAddrTable[g_PlcComIndex].data)[0]);
					else
						ModbusMultipWrite(0x02,0x10,g_PlcAddrTable[g_PlcComIndex].Address,g_PlcAddrTable[g_PlcComIndex].data,g_PlcAddrTable[g_PlcComIndex].number);
				}
				g_PlcComIndex++;
			}
			else if(g_PlcAddrTable[g_PlcComIndex].Address <= PLCADDR_C_5_CHARGE_CURRENT)
			{	
				if(g_PlcAddrTable[g_PlcComIndex].data!=NULL)
					ModbusMultipWrite(0x02,0x10,g_PlcAddrTable[g_PlcComIndex].Address,g_PlcAddrTable[g_PlcComIndex].data,(sizeof(PLC_BATINFOR_TypeDef)-2)/2);
				g_PlcComIndex+=5;

				/*if(g_PlcComIndex>=sizeof(g_PlcAddrTable)/sizeof(PLC_ADDR_TypeDef))
				{	
					g_PlcComIndex=0;
					g_ModbusState=MODBUS_EMETER;
					EmeterReadModeSet(EMETER_READ_POWER);	
					
				}*/
			}
			else 
			{
				if(g_PlcAddrTable[g_PlcComIndex].data!=NULL)
				{	
					float tempfloat;
					uint32_t *p;
					uint8_t i,addr=0;
			
					memset((uint8_t*)&g_plcCellinfor,0x00,sizeof(g_plcCellinfor));
					
					#if(CHARGE_NUM == 15)
					addr=(g_PlcAddrTable[g_PlcComIndex].Address-PLCADDR_A_1_CELL_VOLT)/100;
					#else
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_1_CELL_VOLT )   addr= 0;
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_2_CELL_VOLT )   addr= 1;
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_6_CELL_VOLT )   addr= 2;
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_7_CELL_VOLT )   addr= 3;
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_11_CELL_VOLT )   addr= 4;
					if( g_PlcAddrTable[g_PlcComIndex].Address == PLCADDR_A_12_CELL_VOLT )   addr= 5;
					#endif
					#ifdef CHARGE_STATION
					
  					 for(i=0;i<23;i++)
  					 {
  						g_plcCellinfor.cellVoltage[i]=g_SlotBmsInfor[addr].cellVoltage[i];
  						p=(uint32_t*)&g_plcCellinfor.cellVoltage[i];
  						tempfloat=(float)*p/1000.0f;
  						FloatToMbUint8(tempfloat,(uint8_t*)&g_plcCellinfor.cellVoltage[i]);
					 }
					 
					// for(i=0;i<6;i++)
					  g_plcCellinfor.temp[0]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[5]);
					  g_plcCellinfor.temp[1]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[4]);
					  g_plcCellinfor.temp[2]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[0]);
					  g_plcCellinfor.temp[3]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[1]);
					  g_plcCellinfor.temp[4]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[2]);
					  g_plcCellinfor.temp[5]=Uint16Uint8ToMb((uint8_t*)&g_SlotBmsInfor[addr].temp[3]);
					
					 ModbusMultipWrite(0x02,0x10,g_PlcAddrTable[g_PlcComIndex].Address,g_PlcAddrTable[g_PlcComIndex].data,(sizeof(PLC_CELLINFOR_TypeDef))/2);	
					 #endif
				}
				g_PlcComIndex+=1;

				if(g_PlcComIndex>=sizeof(g_PlcAddrTable)/sizeof(PLC_ADDR_TypeDef))
				{	
					g_PlcComIndex=0;
					g_ModbusState=MODBUS_EMETER;
					EmeterReadModeSet(EMETER_READ_POWER);	
					printf("  TFT tx tx_size = OVER>>>>>>>>>>>>>>>>>>>>>>>\r\n");	
					
					memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
					huart3.RxXferCount=0;
					huart3.RxXferSize=UART3_RX_BUF_SIZE;
					huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
				}
			}
		}  
		else
		{
//				memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
//				huart3.RxXferCount=0;
//				huart3.RxXferSize=UART485_RX_BUF_SIZE;
//    			huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
//				if(EmeterReadModeGet()==EMETER_READ_IDLE)
//				g_ModbusState=MODBUS_PLC;
				
				EmeterReadEnable();
		}
			
	}
}




