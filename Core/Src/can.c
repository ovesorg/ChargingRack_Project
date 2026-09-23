#include"main.h"


union BitGroup_TypeDef g_CanMcuEvent;
union BitGroup_TypeDef g_CanBmsEvent;
union BitGroup_TypeDef g_CanBms1Event;

MCU_FAULT_TypeDef g_McuFaultInfor;
MCU_RUNINFOR_TypeDef  g_McuRunInfor;
MCU_POWEROUT_TypeDef g_McuPowerOut;
MCU_SYSINFOR1_TypeDef  g_McuSysInfor1;
MCU_SYSINFOR2_TypeDef  g_McuSysInfor2;
MCU_FUNSWCH_TypeDef g_McuFunSwitch;
MCU_RTC_TypeDef g_McuRtcInfor;

HM7280_IOT_CHARGE_CMD_TypeDef g_Hm7280IotChargeCmd;


BMS_RT_STATUS1_TypeDef g_BmsRtStatus1[CHARGE_NUM];
BMS_RT_STATUS2_TypeDef g_BmsRtStatus2[CHARGE_NUM];
BMS_RT_STATUS3_TypeDef g_BmsRtStatus3[CHARGE_NUM];


BMS_CELLVOLT1_TypeDef g_BmsCellVolt1[CHARGE_NUM];
BMS_CELLVOLT2_TypeDef g_BmsCellVolt2[CHARGE_NUM];
BMS_CELLVOLT3_TypeDef g_BmsCellVolt3[CHARGE_NUM];
BMS_CELLVOLT4_TypeDef g_BmsCellVolt4[CHARGE_NUM];
BMS_CELLVOLT5_TypeDef g_BmsCellVolt5[CHARGE_NUM];
BMS_CELLVOLT6_TypeDef g_BmsCellVolt6[CHARGE_NUM];

BMS_RTTEMP_TypeDef g_BmsRtTemp[CHARGE_NUM];


BMS_SYSINFOR_TypeDef g_BmsSysInfor[CHARGE_NUM];

BMS_SYSINAP_TypeDef g_BmsSysAP[CHARGE_NUM];


#ifdef CHARGE_STATION
__IO uint32_t g_BmsLostTimeCount[CHARGE_NUM];
BMS_SN_TypeDef g_BmSerialNo[CHARGE_NUM][4];
SLOT_BATINFOR_TypeDef g_SlotBmsInfor[CHARGE_NUM];

#endif
uint8_t g_ChargeIndex=0;	
float total_power_all =0.0;
uint32_t  cur_total_power =0;
VCU_WDG_TypeDef g_Vcu_0x00632;
VCU_WDG_REFRESH_TypeDef g_Vcu_0x00633,g_Bms_0x635;
VCU_LIMIT_TypeDef g_VcuLimit_0x00634;


CAN_TXSTATE_TypeDef g_CanTransmitState;

uint8_t g_SupPwrrun_protect=0;
uint8_t g_SupPwrCharge_Counter=0;

extern RTC_TimeTypeDef g_gpstime;
extern USER_SET_TypeDef g_UserSet;


uint16_t g_bat_cur[CHARGE_NUM];
uint8_t  g_bat_soc[CHARGE_NUM];
uint8_t max_soc_numb =0,bat_act_number =0;

static int16_t bms_vol[15] ={0},bms_num[15] ={0},tempbms_vol_act[15]={0};	
float bat_per_vol[15] ={0.0f},bat_all_vol[15] ={0.0f};


#if 1

#define CAN_ERR_REG(a) CAN_ERR(a)
#define CAN_TSTAT_REG(a) CAN_TSTAT(a)
#define CAN_CTRL_REG(a) CAN_TSTAT(a)

CAN_RECOVERY_TypeDef g_CanRecoverState;
//BUS OFF BEHAVIOR 

#define MCR_INRQ	 ((uint32_t)0x00000001) /* Initialization request */

void CanRecoveryProc(void)
{
	
	uint32_t time_out=(g_CanRecoverState.busoff_state==BUSOFF_SLOW)?T_BUSOFF_SLOW:T_BUSOFF_QUICK;


	if(g_CanRecoverState.busoff_state!=BUSOFF_NONE)
	{
		if(HAL_GetTick()-g_CanRecoverState.busoff_start_timer>=time_out)
		{	
			if(!(CAN_ERR(CAN0)&CAN_ERR_BOERR))
			{	
				//LogPrintf(" --BUSOFF_CLOSE %d  %d  %d\r\n",HAL_GetTick(),g_CanRecoverState.busoff_start_timer,time_out);
				g_CanRecoverState.tx_disable=FALSE;
				g_CanRecoverState.busoff_state=BUSOFF_NONE;
				}
			}
		}
	
	if((CAN_ERR(CAN0)&CAN_ERR_BOERR)&&(HAL_GetTick()-g_CanRecoverState.busoff_start_timer>=time_out))
	{

		CAN_TSTAT(CAN0) |=0x00808080;

		CAN_CTL(CAN0) |= MCR_INRQ;
		CAN_CTL(CAN0) &=~ MCR_INRQ;

		if(g_CanRecoverState.busoff_counter<1000)
			g_CanRecoverState.busoff_counter++;

		g_CanRecoverState.tx_disable=TRUE;
		

		if(g_CanRecoverState.busoff_counter>=5)//slow  recovery
		{	
			g_CanRecoverState.busoff_start_timer=HAL_GetTick();//T_BUSOFF_SLOW;
			g_CanRecoverState.busoff_state=BUSOFF_SLOW;
			//LogPrintf(" --BUSOFF_SLOW %d  %d\r\n",HAL_GetTick(),g_CanRecoverState.busoff_counter);
		}
		else
		{	
			g_CanRecoverState.busoff_start_timer=HAL_GetTick();//T_BUSOFF_QUICK;
			g_CanRecoverState.busoff_state=BUSOFF_QUICK;
			//LogPrintf(" --BUSOFF_QUICK %d  %d\r\n",HAL_GetTick(),g_CanRecoverState.busoff_counter);
		}

	}


	if((((CAN_ERR(CAN0)&0x00000070)==0x00000000)||((CAN_ERR(CAN0)&0x00000070)==0x00000030))
		&&((CAN_ERR(CAN0)&CAN_ERR_BOERR)==RESET)&&g_CanRecoverState.busoff_counter)
		{
			g_CanRecoverState.busoff_counter=0;
			 //LogPrintf(" --BUSOFF_CLEAR_NONE %d\r\n",HAL_GetTick());
		}

	//MISS ACK BEHAVIOR
	if((CAN_ERR(CAN0)&0x00000070)==0x00000030&&g_CanRecoverState.busoff_state==BUSOFF_NONE)
	{

		switch(g_CanRecoverState.miss_ack_state)
		{
			case MISS_ACK_NONE:
				g_CanRecoverState.miss_ack_start_timer=HAL_GetTick();//TX_TIMEOUT;
				g_CanRecoverState.miss_ack_state=MISS_ACK_TIMEOUT;
			   // LogPrintf(" --MISS_ACK_NONE %d\r\n",HAL_GetTick());
				break;
			case MISS_ACK_TIMEOUT:
				if(HAL_GetTick()-g_CanRecoverState.miss_ack_start_timer>=TX_TIMEOUT)
				{	//stop send
					g_CanRecoverState.miss_ack_start_timer=HAL_GetTick();//TX_RECOVERY;
					g_CanRecoverState.miss_ack_state=MISS_ACK_RECOVERY;
					//stop transmit
					CAN_TSTAT(CAN0) |=0x00808080;
					g_CanRecoverState.tx_disable=TRUE;

					//LogPrintf(" --MISS_ACK_TIMEOUT %d\r\n",HAL_GetTick());
				}
				break;
			case MISS_ACK_RECOVERY:
				if(HAL_GetTick()-g_CanRecoverState.miss_ack_start_timer>=TX_RECOVERY)
				{	//start send
					g_CanRecoverState.miss_ack_start_timer=HAL_GetTick();//TX_TIMEOUT;
					g_CanRecoverState.miss_ack_state=MISS_ACK_TIMEOUT;
					//LogPrintf(" --MISS_ACK_RECOVERY %d\r\n",HAL_GetTick());
					
					g_CanRecoverState.tx_disable=FALSE;
				}
				break;
			}
		}
		else
		{
			g_CanRecoverState.miss_ack_state=MISS_ACK_NONE;
			g_CanRecoverState.miss_ack_start_timer=HAL_GetTick();

			if(g_CanRecoverState.busoff_state==BUSOFF_NONE)
				g_CanRecoverState.tx_disable=FALSE;

			//LogPrintf(" --MISS_ACK_none %d\r\n",HAL_GetTick());
	  	}

}
#endif



void CanRamInit(void)
{
	uint8_t i=0;
	g_CanMcuEvent.BYTE=0;
	g_CanBmsEvent.BYTE=0;
	g_CanBms1Event.BYTE=0;
	g_SupPwrrun_protect=0;
	g_SupPwrCharge_Counter=0;

	memset((uint8_t*)&g_McuFaultInfor,0x00,8);
	memset((uint8_t*)&g_McuRunInfor,0x00,8);
	memset((uint8_t*)&g_McuPowerOut,0x00,8);
	memset((uint8_t*)&g_McuSysInfor1,0x00,8);
	memset((uint8_t*)&g_McuSysInfor2,0x00,8);
	memset((uint8_t*)&g_McuFunSwitch,0x00,8);
	memset((uint8_t*)&g_McuRtcInfor,0x00,8);

	memset((uint8_t*)&g_BmsRtStatus1,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsRtStatus2,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsRtStatus3,0x00,8*CHARGE_NUM);

	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);
	memset((uint8_t*)&g_BmsCellVolt5,0x00,8*CHARGE_NUM);

	memset((uint8_t*)&g_BmsRtTemp,0x00,8*CHARGE_NUM);
	
	memset((uint8_t*)&g_BmsSysInfor,0x00,8*CHARGE_NUM);

	#ifdef CHARGE_STATION
	memset((uint8_t*)&g_BmSerialNo,0x00,8*4*CHARGE_NUM);

	memset((uint8_t*)&g_BmsLostTimeCount,0x00,4*CHARGE_NUM);

	memset((uint8_t*)&g_SlotBmsInfor,0x00,sizeof(SLOT_BATINFOR_TypeDef)*CHARGE_NUM);
	

	for(i=0;i<CHARGE_NUM;i++)
	{
		g_BmsRtStatus1[i].RealtimeCurrentH=0x07;
		g_BmsRtStatus1[i].RealtimeCurrentL=0xd0;
		}
	#endif

	g_VcuLimit_0x00634.SpeedLimit=g_UserSet.max_speed_limit;

	memset((uint8_t*)&g_CanTransmitState,0x00,sizeof(g_CanTransmitState));

}

void CanMcuParse(uint32_t id,uint8_t *data,uint8_t len)
{

	switch(id)
	{
		case 0x01806E502:
			memcpy((uint8_t*)&g_McuSysInfor1,data,len);
			McuSysInfor1Event=TRUE;
			break;
		case 0x01806E503:
			memcpy((uint8_t*)&g_McuSysInfor2,data,len);
			McuSysInfor2Event=TRUE;
			break;
		case 0x01806E600:
			memcpy((uint8_t*)&g_McuFaultInfor,data,len);
			McuFaultEvent=TRUE;
			break;
		case 0x01806E601:
			memcpy((uint8_t*)&g_McuRunInfor,data,len);
			McuRunInforEvent=TRUE;
			break;
		case 0x01806E602:
			memcpy((uint8_t*)&g_McuPowerOut,data,len);
			McuPwrOutEvent=TRUE;
			break;
		}

	
}

void CanBmsParse(uint32_t id,uint8_t *data,uint8_t len)
{
	uint8_t number =0;
	g_ChargeIndex=0;
	#ifdef CHARGE_STATION
	g_ChargeIndex=id>>20;
	g_ChargeIndex&=0x0f;

	id&=0xff0EFfff;
	
	//id|=0x0060000;
	if(id==0x01806E610||id==0x01806E611||id==0x01806E612)
		g_BmsLostTimeCount[g_ChargeIndex]=0;
	
	if((id >=0x1802F000 )&&(id <= 0x1802F010 ))
	{
		number = id & 0xff; 
		memcpy((uint8_t*)&g_BmsSysAP[number],data,len);
	} 
	#endif
	switch(id)
	{
		case 0x01806E516:
			memcpy((uint8_t*)&g_BmsSysInfor[g_ChargeIndex],data,len);
			BmsSysInforEvent=TRUE;
			break;
		case 0x01806E610:
			memcpy((uint8_t*)&g_BmsRtStatus1[g_ChargeIndex],data,len);
			BmsRtState1Event=TRUE;
			break;
		case 0x01806E611:
			memcpy((uint8_t*)&g_BmsRtStatus2[g_ChargeIndex],data,len);
			BmsRtState2Event=TRUE;
			break;
		case 0x01806E612:
			memcpy((uint8_t*)&g_BmsRtStatus3[g_ChargeIndex],data,len);
			BmsRtState3Event=TRUE;
			break;
		case 0x01806E613:
			memcpy((uint8_t*)&g_BmsCellVolt1[g_ChargeIndex],data,len);
			BmsCellVolt1Event=TRUE;
			break;
		case 0x01806E614:
			memcpy((uint8_t*)&g_BmsCellVolt2[g_ChargeIndex],data,len);
			BmsCellVolt2Event=TRUE;
			break;
		case 0x01806E615:
			memcpy((uint8_t*)&g_BmsCellVolt3[g_ChargeIndex],data,len);
			BmsCellVolt3Event=TRUE;
			break;
		case 0x01806E616:
			memcpy((uint8_t*)&g_BmsCellVolt4[g_ChargeIndex],data,len);
			BmsCellVolt4Event=TRUE;
			break;
		case 0x01806E617:
			memcpy((uint8_t*)&g_BmsCellVolt5[g_ChargeIndex],data,len);
			BmsCellVolt5Event=TRUE;
			break;
		case 0x01806E618:
			memcpy((uint8_t*)&g_BmsCellVolt6[g_ChargeIndex],data,len);
			BmsCellVolt6Event=TRUE;
			break;	
		//case 0x01806E620:
		case 0x01806E61A:// 20250422 
			memcpy((uint8_t*)&g_BmsRtTemp[g_ChargeIndex],data,len);
			BmsRtTempEvent=TRUE;
			break;
		#ifdef CHARGE_STATION
		case 0x01806E510:
			if(data[0]<4)
			{
				memcpy((uint8_t*)&g_BmSerialNo[g_ChargeIndex][data[0]],data,len);
				BmsSerialNoEvent=TRUE;
			}
			break;
		#endif
	 
		}
	
}

void CanTransmit(uint32_t id,uint8_t *data,uint8_t len)
{
	can_trasnmit_message_struct transmit_message;

	if(g_CanRecoverState.tx_disable)
		return ;
	
	can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message);
	transmit_message.tx_sfid = 0x00;
	transmit_message.tx_efid = id;
	transmit_message.tx_ft = CAN_FT_DATA;
	transmit_message.tx_ff = CAN_FF_EXTENDED;
	transmit_message.tx_dlen = 8;
	memcpy(transmit_message.tx_data,data,len);

	can_message_transmit(CAN0, &transmit_message);
}

#ifdef MILEAGE_RECORD_SUPPORT

__IO uint8_t stateFlag;
const double conversionFactor = 0.01294 / 18000;
__IO double Mileage = 0;
__IO double To_mileage = 0;

/*void TimeWriteMileageCallback(TIMER_TypeDef *p)
{

	uint16_t Speed_temp;
	double Tomd_temp;
	uint8_t T_temp[32] = {0};

#ifdef SIF
	p->count = 205;
#else
	p->count = 105;
#endif
	GattGetData(LIST_DTA, DTA_MTRD, (uint8_t *)&Speed_temp);
	Mileage += Speed_temp * conversionFactor;
	To_mileage += Speed_temp * conversionFactor;
	//	memset(T_temp, 0, 32);
	snprintf((char *)T_temp, sizeof(T_temp), "%.1f", Mileage);
	GattSetData(LIST_DTA, DTA_CUMD, T_temp);
	// memset(T_temp, 0, 32);
	if (Speed_temp == 0)
	{
		if (stateFlag == TRUE)
		{
			Tomd_temp = EEpGetTomd();
			To_mileage += Tomd_temp;
			EEpSetTomd(To_mileage);
			To_mileage = 0;
			stateFlag = FALSE;
		}
	}
	else
	{
		stateFlag = TRUE;
	}
	if (gpio_input_bit_get(GPIOC, GPIO_PIN_2) == 0)
	{
		Mileage = 0;
	}
	Tomd_temp = EEpGetTomd();
	snprintf((char *)T_temp, sizeof(T_temp), "%.1f", Tomd_temp);
	GattSetData(LIST_DTA, DTA_TOMD, T_temp);
	GattSetData(LIST_CMD, CMD_RTMD, T_temp);
} */

void CalculateMileage(uint16_t Speed_temp)
{
	Mileage += Speed_temp * conversionFactor;
	To_mileage += Speed_temp * conversionFactor;
}

void UpdateTOMileageData()
{
	double Tomd_temp;
	uint8_t T_temp[32] = {0};
	Tomd_temp = EEpGetTomd();
	snprintf((char *)T_temp, sizeof(T_temp), "%.1f", Tomd_temp);
	// GattSetData(LIST_DTA, DTA_CUMD, T_temp);
	GattSetData(LIST_DTA, DTA_TOMD, T_temp);
	GattSetData(LIST_CMD, CMD_RTMD, T_temp);
}

void UpdateMileageData(double mileage)
{
	uint8_t T_temp[32] = {0};
	snprintf((char *)T_temp, sizeof(T_temp), "%.1f", mileage);
	GattSetData(LIST_DTA, DTA_CUMD, T_temp);
}

void HandleEEPROM(uint16_t Speed_temp)
{
	double Tomd_temp;

	if (Speed_temp == 0)
	{
		if (stateFlag == TRUE)
		{
			Tomd_temp = EEpGetTomd();
			To_mileage += Tomd_temp;
			EEpSetTomd(To_mileage);
			To_mileage = 0;
			stateFlag = FALSE;
		}
	}
	else
	{
		stateFlag = TRUE;
	}
}

void MileageCallback(void)
{
	uint16_t Speed_temp;


	// 鑾峰彇閫熷害鏁版嵁
	GattGetData(LIST_DTA, DTA_MTRD, (uint8_t *)&Speed_temp);

	// 璁＄畻閲岀▼
	CalculateMileage(Speed_temp);
	// 鏇存柊褰撴閲岀▼鏁版嵁
	UpdateMileageData(Mileage);
	// 澶勭悊EEPROM璁块棶
	HandleEEPROM(Speed_temp);

	// 妫€鏌ラ噷绋嬮噸缃潯浠?
	if (gpio_input_bit_get(GPIOC, GPIO_PIN_2) == 0)
	{
		Mileage = 0;
	}
	// 鏇存柊鎬婚噷绋?
	UpdateTOMileageData();
}
#endif


int findMaxIndex(void) 
{
    int maxIndex = 0,i=0;
    for (i = 1; i < CHARGE_NUM; i++) 
	{
        if (g_bat_soc[i] > g_bat_soc[maxIndex]) {
            maxIndex = i;
        }
    }
    return maxIndex;
}

void vcu_canbox_mode(uint8_t num,uint16_t dc_charge_cur,uint8_t bat_number)
{
	uint32_t set_power =0;
	uint8_t temp[4];
	float power = 0.0;
	

	set_power = get_set_power();
	temp[0] = set_power>>8;
	temp[1] = set_power;
	temp[2] = set_power>>24;
	temp[3] = set_power>>16;
	power = MbUint8ToFloat(temp);
	
//	if(PaygGetPayState()== FALSE )
//	{
//		g_Hm7280IotChargeCmd.TargetChargeVoltageH = 1; 
//	}
//	else
	{
		g_Hm7280IotChargeCmd.TargetChargeVoltageH = 0; 
	}


	if(power > 1.0)
	g_Hm7280IotChargeCmd.ChargeEnable = 0X55;
	else 
	g_Hm7280IotChargeCmd.ChargeEnable = 0;	
	
	g_Hm7280IotChargeCmd.TargetChargeVoltageL =   num;
	g_Hm7280IotChargeCmd.TargetChargeCurrentH = dc_charge_cur>>8;
	g_Hm7280IotChargeCmd.TargetChargeCurrentL = dc_charge_cur;
	CanTransmit(0x1806E640UL,(uint8_t*)&g_Hm7280IotChargeCmd,8); //下发充电电压以及充电电流

}


/*
 * 按SOC大小动态分配功率, 并下发给每个子设备
 * power           : 设置的功率(与 total_power_all 同单位)
 * total_power_all : 全部子设备额定功率之和(ap_vol*ap_cur)
 * power >= total_power_all : 所有子设备按 g_BmsSysAP 中对应序号的额定电压/电流下发
 * power <  total_power_all : 按SOC从大到小依次分配, 先满足额定功率, 剩余功率不足时按 剩余功率/额定电压 下发电流
 */
void vcu_canbox_cur( void)
{
	uint32_t set_power =0,candi = 0;
	uint8_t temp[4];
	float power = 0.0;
	uint8_t i,j,tmp;
	uint8_t soc_order[CHARGE_NUM];
	uint16_t ap_vol[CHARGE_NUM],ap_cur[CHARGE_NUM],out_cur[CHARGE_NUM];
	float power_raw,left_power,rated_power;
//	static uint8_t target_vol_h = 0;   // fa xia  分配的电压电流


	set_power = get_set_power();
	temp[0] = set_power>>8;
	temp[1] = set_power;
	temp[2] = set_power>>24;
	temp[3] = set_power>>16;
	power = MbUint8ToFloat(temp);

	printf("********* set power =   %f   %f\r\n" ,power,total_power_all); 

	//读取每个子设备的额定电压、额定电流
	for(i=0;i<CHARGE_NUM;i++)
	{
		ap_vol[i] = (g_BmsSysAP[i].TargetAPVoltageH<<8)|g_BmsSysAP[i].TargetAPVoltageL;
		ap_cur[i] = (g_BmsSysAP[i].TargetAPCurrentH<<8)|g_BmsSysAP[i].TargetAPCurrentL;
		out_cur[i]= 0;
		soc_order[i]= i;
	}

	//按SOC从大到小排序, SOC大的优先分配功率
	for(i=0;i<CHARGE_NUM;i++)
	{
		
		
			for(j=i+1;j<CHARGE_NUM;j++)
			{
					if(g_SlotBmsInfor[soc_order[j]].Soc > g_SlotBmsInfor[soc_order[i]].Soc)
					{
						tmp = soc_order[i];
						soc_order[i] = soc_order[j];
						soc_order[j] = tmp;
					}
		  }
		
	}

	power_raw = power*1000.0f;   //换算成与 ap_vol*ap_cur 相同的量纲

	if(power >= total_power_all)  //功率充足: 全部子设备按额定下发
	{
		for(i=0;i<CHARGE_NUM;i++)
			out_cur[i] = ap_cur[i];
	}
	else                          //功率不足: 按SOC顺序依次分配剩余功率
	{
		left_power = power_raw;

		for(i=0;i<CHARGE_NUM;i++)
		{
			j = soc_order[i];
			
			if(g_SlotBmsInfor[j].Soc == 100)  ap_cur[j] = 0;
			
			if((ap_vol[j]==0)||(ap_cur[j]==0)||(left_power<=0))
				continue;

			rated_power = (float)ap_vol[j]*(float)ap_cur[j];
			
			rated_power /=100;

			if(left_power >= rated_power)
			{
				out_cur[j] = ap_cur[j];          //功率够, 按额定功率下发
				left_power -= rated_power;
			}
			else
			{
				out_cur[j] = (uint16_t)((left_power*10)/(float)ap_vol[j]); //剩余功率不足, 电流 = 剩余功率/额定电压
				
				out_cur[j] *=10;
				left_power = 0;
			}
		}
	}

	for(i=0;i<CHARGE_NUM;i++)
	{
		candi = 0x1803F000UL + i;   //子设备序号


		//电压按额定电压下发, 电流按分配的功率下发
		g_Hm7280IotChargeCmd.TargetChargeVoltageH = (uint8_t)(ap_vol[i]>>8);
		g_Hm7280IotChargeCmd.TargetChargeVoltageL = (uint8_t)ap_vol[i];
		g_Hm7280IotChargeCmd.TargetChargeCurrentH = (uint8_t)(out_cur[i]>>8);
		g_Hm7280IotChargeCmd.TargetChargeCurrentL = (uint8_t)out_cur[i];
		CanTransmit(candi,(uint8_t*)&g_Hm7280IotChargeCmd,8); //下发充电电压以及充电电流

		printf("********* slot %d : soc %d  vol %d  cur %d\r\n" ,i,g_SlotBmsInfor[i].Soc,ap_vol[i],out_cur[i]); 
	}
}




void CanProc(void)
{
	#if defined(E_MOB48V_PROJECT)||defined(CHARGE_STATION)
    #ifndef GD32F10X_MD
	uint16_t temp16;
	uint32_t temp32;
    int16_t tempInt16;
	uint8_t *p_u8;
	uint32_t power;
	uint8_t i;
	uint8_t limit_speed=90;
	static uint16_t dc_charge_vol =0,dc_charge_cur = 100,count_flag = 0,print_count =0;
	uint16_t ap_vol =0,ap_cur =0;
	
	#ifndef SUPERPOWER_UART_VERSION

	#ifdef MILEAGE_RECORD_SUPPORT	
	if(TimerGetEventState(TIMER_MILEAGE))
	{
		MileageCallback();
		TimerEventClear(TIMER_MILEAGE);
	}
	#endif

	CanRecoveryProc();
	
	
	//g_SlotBmsInfor[i].Soc  g_SlotBmsInfor[i].ChargeCurrent  g_SlotBmsInfor[i].RemainCap
	
	
	for(i=0;i<CHARGE_NUM;i++)
	{
		g_bat_soc[i] = g_SlotBmsInfor[i].Soc;
	}
	
	for(i=0;i<CHARGE_NUM;i++)
	{
		g_bat_cur[i] = g_SlotBmsInfor[i].ChargeCurrent;
	}
	
	bat_act_number =0;
	
	for(i=0;i<CHARGE_NUM;i++)
	{
		if(g_SlotBmsInfor[i].ChargeCurrent > 0 )
		bat_act_number ++ ;
	}
	max_soc_numb = findMaxIndex();
	
	
	if(HAL_GetTick()-g_CanTransmitState.t200ms>=10000)
	{
		g_CanTransmitState.t200ms=HAL_GetTick();
		vcu_canbox_mode(max_soc_numb,g_SlotBmsInfor[max_soc_numb].ChargeCurrent,bat_act_number);   
	}
	
	
	
	if(HAL_GetTick()-g_CanTransmitState.t500ms>=5000)
    {
    	 
    	g_CanTransmitState.t500ms=HAL_GetTick();
	 
		cur_total_power = 0;	
		for	(i=0;i<CHARGE_NUM;i++)
		{
			ap_vol =  (g_BmsSysAP[i].TargetAPVoltageH<< 8)| g_BmsSysAP[i].TargetAPVoltageL;
			ap_cur =  (g_BmsSysAP[i].TargetAPCurrentH<< 8)| g_BmsSysAP[i].TargetAPCurrentL;
			
			cur_total_power += ap_vol*ap_cur;			   
		}
		total_power_all  = cur_total_power /100000.0;
		printf("*******AP total_power_all =  %f\r\n" ,total_power_all);  
		
		vcu_canbox_cur();
	 }
		
		
//	print_count++;
//	if(print_count>40)  g_SlotBmsInfor[i].ChargeCurrent
//	{
//		print_count = 0;
//		
//		for(i=0;i<CHARGE_NUM;i++)
//		{
//				printf("CAN BMS data: g_SlotBmsInfor[i].Soc =%d   g_SlotBmsInfor[i].ChargeCurrent = %d  g_SlotBmsInfor[i].RemainCap = %f \r\n" ,g_SlotBmsInfor[i].Soc,g_SlotBmsInfor[i].ChargeCurrent,g_SlotBmsInfor[i].RemainCap);  
//		}
//	}

#if 0	
	if(HAL_GetTick()-g_CanTransmitState.t200ms>=100)
    {
    	g_CanTransmitState.t200ms=HAL_GetTick();
		 
		//count_flag++;
		//if(count_flag >= 10)  // 2s改变一次输出电流
		{
			if(g_AC_ccsinput > 2100) 
			{
				dc_charge_cur += 100;
				if(dc_charge_cur > (g_Battcharge_cur*X2))
				dc_charge_cur = g_Battcharge_cur*X2;
			}
			else if(g_AC_ccsinput > 2000) 
			{
				//if((g_Battcharge_cur*X2) > dc_charge_cur)
				//dc_charge_cur += 50;
				dc_charge_cur = g_Battcharge_cur*0.5;
			}
			else if(g_AC_ccsinput > 1900) 
			{
				//if(((g_Battcharge_cur*X2) > dc_charge_cur)&&(dc_charge_cur > 20))
				//dc_charge_cur -= 20;
				dc_charge_cur = g_Battcharge_cur*0.2;
			}
			else dc_charge_cur  = 0;
		}
		
		
		
			// 2. 判断是否超限
//		if (CcsEnergyLimitReached)
//		{
//			g_Hm7280IotChargeCmd.TargetChargeVoltageH = 0;
//			g_Hm7280IotChargeCmd.TargetChargeVoltageL = 0;
//			g_Hm7280IotChargeCmd.TargetChargeCurrentH = 0;
//			g_Hm7280IotChargeCmd.TargetChargeCurrentL = 0;
//			g_Hm7280IotChargeCmd.ChargeEnable = 0; 
//		}
//		else
		
		{
			if(dc_charge_cur >(g_Battcharge_cur*X2))
			{
				dc_charge_cur = g_Battcharge_cur*X2;
			}
			
			if(g_Battcharge_vol > 0)	 		
				g_Hm7280IotChargeCmd.ChargeEnable = 1;
			else 
			{
				dc_charge_cur = 0;
				g_Hm7280IotChargeCmd.ChargeEnable = 0;
			}
			
			g_Hm7280IotChargeCmd.TargetChargeCurrentH = dc_charge_cur>>8;
			g_Hm7280IotChargeCmd.TargetChargeCurrentL = dc_charge_cur;
			 
			g_Hm7280IotChargeCmd.TargetChargeVoltageH =   >> 8;
			g_Hm7280IotChargeCmd.TargetChargeVoltageL =  ;
		}
		// 3. 发送关闭或正常命令
		CanTransmit(0x1806E640UL,(uint8_t*)&g_Hm7280IotChargeCmd,8); //下发充电电压以及充电电流
//		print_count++;
//		if(print_count>20)
//		{
//			print_count = 0;
//			printf("CAN BMS data: dc_charge_cur =%d   dc_charge_vol = %d g_AC_ccsinput = %d  g_Battcharge_cur = %d\r\n" ,dc_charge_cur,dc_charge_vol,g_AC_ccsinput,g_Battcharge_cur);  
//		}
	}
#endif
	#ifdef E_MOB48V_PROJECT
	if(HAL_GetTick()-g_CanTransmitState.t500ms>=500)
    {
    	temp16=g_UserSet.max_speed_limit;
    	g_CanTransmitState.t500ms=HAL_GetTick();

		GattSetData(LIST_CMD, CMD_TMZS, (uint8_t*)&g_UserSet.timezone);
		GattSetData(LIST_CMD, CMD_MXPS, (uint8_t*)&temp16);

		if(g_UserSet.timezone>=0)
			g_McuRtcInfor.hour=g_gpstime.Hours+g_UserSet.timezone;
		else
			g_McuRtcInfor.hour=(24+g_UserSet.timezone+g_gpstime.Hours)%24;
		
		g_McuRtcInfor.min=g_gpstime.Minutes;

		CanTransmit(0x1806E630,(uint8_t*)&g_McuRtcInfor,8);
		}
    else if(HAL_GetTick()-g_CanTransmitState.t1000ms>=1000)
    {
    	g_CanTransmitState.t1000ms=HAL_GetTick();

		if(PaygGetPayState()||PaygGetFreeState()||GetDemoState())
		{	g_Vcu_0x00633.Wdgrefresh=1;
			//g_Vcu_0x00633.Wdgrefresh=1;
			g_SupPwrrun_protect=TRUE;
		    g_Vcu_0x00633.Heartbeat=1;
		    g_Vcu_0x00633.WdgCounter=0x00ff;

			g_Bms_0x635.Wdgrefresh=1;
			g_Bms_0x635.Heartbeat=1;

			HAL_GPIO_WritePin(GSM_RST_GPIO_Port,GSM_RST_Pin,SET);
		}
		else
		{	
			#ifdef E_MOB48V_PROJECT_CAMP
			if(g_SupPwrrun_protect==FALSE)
			#endif
			{	
				g_Vcu_0x00633.Wdgrefresh=0;
				g_Bms_0x635.Wdgrefresh=0;
				
				g_Vcu_0x00633.WdgCounter=0x00ff;
			}
			g_Vcu_0x00633.Heartbeat=1;
			g_Bms_0x635.Heartbeat=1;
			HAL_GPIO_WritePin(GSM_RST_GPIO_Port,GSM_RST_Pin,RESET);
		}

		g_McuRtcInfor.hour=g_gpstime.Hours;
		g_McuRtcInfor.min=g_gpstime.Minutes;

		CanTransmit(0x1806E633,(uint8_t*)&g_Vcu_0x00633,8);
		HAL_Delay(1);
		CanTransmit(0x1806E635,(uint8_t*)&g_Bms_0x635,8);
    }
    else if(HAL_GetTick()-g_CanTransmitState.t2000ms>=5000) //5s
    {
    	
    	g_CanTransmitState.t2000ms=HAL_GetTick();

		if(PaygGetFreeState())
		{	g_Vcu_0x00632.WatchdogEnable=FALSE;
			#ifndef OLD_CAN_VERSION
			g_Vcu_0x00632.BmsWatchdogEnable=FALSE;
			#endif
			}
		else
		{	g_Vcu_0x00632.WatchdogEnable=TRUE;
			#ifndef OLD_CAN_VERSION
			g_Vcu_0x00632.BmsWatchdogEnable=TRUE;
			#endif
			}

		//g_VcuLimit_0x00634.

		g_VcuLimit_0x00634.PowerLimit=100;
		g_VcuLimit_0x00634.Reserved2[0]=100;
		
		if(PaygGetPayState()||PaygGetFreeState())
		{	
		  // g_VcuLimit_0x00634.SpeedLimit=90;
		   limit_speed=90;
		}
		else
		{	
			limit_speed=g_UserSet.max_speed_limit;
		}

		if(limit_speed!=g_VcuLimit_0x00634.SpeedLimit)
		{	
			g_VcuLimit_0x00634.SpeedLimit=limit_speed;
			CanTransmit(0x1806E634,(uint8_t*)&g_VcuLimit_0x00634,8);
			HAL_Delay(2);
		}
		
		if(g_McuSysInfor2.FunSwtich_WDGSet!=g_Vcu_0x00632.WatchdogEnable\
			||g_BmsRtStatus3[0].BatWatchdogEnable!=g_Vcu_0x00632.BmsWatchdogEnable)
		{	
			#ifndef E_MOB48V_PROJECT_BAT
			extern uint8_t g_TokenValidInput;
			
			if(g_TokenValidInput)
			#endif
			{	
				#ifndef E_MOB48V_PROJECT_BAT
				g_TokenValidInput=FALSE;
				#endif
				CanTransmit(0x1806E632,(uint8_t*)&g_Vcu_0x00632,8);
				}
			}
    	}
	 #endif

	 #ifdef CHARGE_STATION
	 
	 if(HAL_GetTick()-g_CanTransmitState.t1000ms>=1000)
	 {
	 	 uint8_t all_lost=TRUE;
		 g_CanTransmitState.t1000ms=HAL_GetTick();
	 
		 for(i=0;i<CHARGE_NUM;i++)
		 {
			 g_BmsLostTimeCount[i]++;
			 if(g_BmsLostTimeCount[i]<5)
			 	all_lost=FALSE;
			 	
		 }
		 if(all_lost)
		 	BmsRtState1Event=TRUE; //CLEAR ALL	 	
	}
	 
	if(HAL_GetTick()-g_CanTransmitState.t5000ms>=1500)   // ????
	{
		uint8_t buffer[8]={0};
		g_CanTransmitState.t5000ms=HAL_GetTick();
		buffer[0]=0x10;
		CanTransmit(0x1806E110,(uint8_t*)&buffer,8);
	}
	
	 #endif
	

	//SetDashBoardData(LIST_speed,10);
	#ifdef E_MOB48V_PROJECT
	if(McuFaultEvent)
	{
		McuFaultEvent=FALSE;
		tempInt16=g_McuFaultInfor.ContrlTemp-40;
		GattSetData(LIST_DTA,DTA_CTMP,(uint8_t*)&tempInt16);
		
		tempInt16=g_McuFaultInfor.MotorTemp-40;
		GattSetData(LIST_DTA,DTA_MTPM,(uint8_t*)&tempInt16);

		SetDashBoardData(LIST_gearlevel,g_McuFaultInfor.GearState);
		SetDashBoardData(LIST_reverse,g_McuFaultInfor.ReverseState);

		SetDashBoardData(LIST_check,g_McuFaultInfor.BrakeState);
		SetDashBoardData(LIST_electrical,g_McuFaultInfor.MotorBlock_fault|g_McuFaultInfor.MotorHall_fault
			|g_McuFaultInfor.MotorOverTemp_fault|g_McuFaultInfor.MotorPhaseLost_fault);
		SetDashBoardData(LIST_handle,g_McuFaultInfor.Throttle_fault);
		SetDashBoardData(LIST_Ecu,0);
   /* if(g_McuFaultInfor.BrakeValue == 0)
		{
			SetDashBoardData(LIST_check,0);
		}
		else*/
		{
			//SetDashBoardData(LIST_check,g_McuFaultInfor.BrakeState);
		}
		/*if (g_McuFaultInfor.EBSState == 1 && g_McuFaultInfor.HillHolderState == 0)
		{
			SetDashBoardData(LIST_gearlevel, 1);
		}
		if (g_McuFaultInfor.EBSState == 0 && g_McuFaultInfor.HillHolderState == 1)
		{
			SetDashBoardData(LIST_gearlevel, 2);
		}
		if (g_McuFaultInfor.EBSState == 1 && g_McuFaultInfor.HillHolderState == 1)
		{
			SetDashBoardData(LIST_gearlevel, 3);
		}*/

		//SetDashBoardData(LIST_gearlevel, g_McuFaultInfor.GearState);

		if(g_McuFaultInfor.PauseDisableStae)
			SetDashBoardData(LIST_parking,TRUE);
		else
			SetDashBoardData(LIST_parking,FALSE);
		
	}
	
	if(McuRunInforEvent)
	{
		temp16=(g_McuRunInfor.MotorSpeedH<<8)|g_McuRunInfor.MotorSpeedL;
		if(temp16>20000)
			temp16=0;
		GattSetData(LIST_DTA,DTA_MTRD,(uint8_t*)&temp16);
		
		SetDashBoardData(LIST_speed, (uint16_t)temp16*0.0129); // *0.01294
		
		temp16=(g_McuRunInfor.TyreSpeedH<<8)|g_McuRunInfor.TyreSpeedL;
		
		GattSetData(LIST_DTA,DTA_TSPD,(uint8_t*)&temp16);
		

		temp16=(g_McuRunInfor.RtVoltageH<<8)|g_McuRunInfor.RtVoltageL;
		temp16=temp16/10;
		GattSetData(LIST_DTA,DTA_RVLT,(uint8_t*)&temp16);

		tempInt16=(int16_t)((g_McuRunInfor.RtCurrentH<<8)|g_McuRunInfor.RtCurrentL);
		//tempInt16=tempInt16/10;
		//tempInt16-=2000;
		tempInt16*=100;
		GattSetData(LIST_DTA,DTA_RCUR,(uint8_t*)&tempInt16);

		McuRunInforEvent=FALSE;
	}
	if(McuPwrOutEvent)
	{
		McuRunInforEvent=FALSE;
	}
	if(McuSysInfor1Event)
	{
		temp16=(g_McuSysInfor1.RateMaxInputCurtH<<8)|g_McuSysInfor1.RateMaxInputCurtL;
		temp16=temp16/10;
		GattSetData(LIST_DTA,DTA_RMAX,(uint8_t*)&temp16);
		McuSysInfor1Event=FALSE;
		}
	if(McuSysInfor2Event)
	{
		McuSysInfor2Event=FALSE;

		temp16=(g_McuSysInfor2.MaxSpeedSetH<<8)|g_McuSysInfor2.MaxSpeedSetL;
		GattSetData(LIST_DTA,DTA_CMXS,(uint8_t*)&temp16);

		temp16=(g_McuSysInfor2.MaxInputCurtSetH<<8)|g_McuSysInfor2.MaxInputCurtSetL;
		GattSetData(LIST_DTA,DTA_CMXC,(uint8_t*)&temp16);
		}
	

	if(BmsRtState1Event)
	{
		temp32=(g_BmsRtStatus1[0].RealtimeVoltageH<<8)+g_BmsRtStatus1[0].RealtimeVoltageL;
		temp32*=100;

		GattSetData(LIST_DTA,DTA_PCKV,(uint8_t*)&temp32);

		SetDashBoardData(LIST_volts,(uint8_t)(temp32/1000));
		
		tempInt16=(int16_t)((g_BmsRtStatus1[0].RealtimeCurrentH<<8)+g_BmsRtStatus1[0].RealtimeCurrentL);

		tempInt16-=2000;
		tempInt16*=100;
		
		GattSetData(LIST_DTA,DTA_PCKC,(uint8_t*)&tempInt16);
		//GattSetData(LIST_DTA,DTA_RCUR,(uint8_t*)&tempInt16);

		if(PaygGetPayState()==FALSE&&PaygGetFreeState()==FALSE)
		{
			if(tempInt16>500)
			{	
				if(g_SupPwrCharge_Counter<100)
				{	
					g_SupPwrCharge_Counter++;

					if(g_SupPwrCharge_Counter>10)
					{	g_SupPwrrun_protect=FALSE;
						LogPrintf("SupPwr:Protect cancel %d \r\n",g_SupPwrCharge_Counter);
						}
					}
				}
			else
			{
				g_SupPwrCharge_Counter=0;
			}
		}
		else
		{
			g_SupPwrCharge_Counter=0;
		}
		

		if(tempInt16>0)
		{	power=temp32/100*tempInt16/100/100;
			temp16=power;
			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			temp16=0;
			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}
		else if(tempInt16<0)
		{	power=temp32/100*-tempInt16/100/100;
			temp16=0;
			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			temp16=power;
			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}
		else
		{
			temp16=0;
			GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}

		temp16=g_BmsRtStatus1[0].ChargingMosOn;
		GattSetData(LIST_DTA,DTA_CMOS,(uint8_t*)&temp16);
		temp16=g_BmsRtStatus1[0].DisChargeMosOn;
		GattSetData(LIST_DTA,DTA_DMOS,(uint8_t*)&temp16);

		p_u8=(uint8_t*)&g_BmsRtStatus1[0];
		temp16=p_u8[0]|(p_u8[1]<<8);
		temp16&=0x3ffc;
		GattSetData(LIST_DTA,DTA_PPST,(uint8_t*)&temp16);

		temp16=p_u8[0]&0x03;
		GattSetData(LIST_DTA,DTA_PMCS,(uint8_t*)&temp16);

		BmsRtState1Event=FALSE;
	}
	if(BmsRtState2Event)
	{
		temp16=g_BmsRtStatus2[0].Soc;
		GattSetData(LIST_DTA,DTA_RSOC,(uint8_t*)&temp16);

		SetDashBoardData(LIST_soc,(uint8_t)temp16);
		BmsRtState2Event=FALSE;
	}
	if(BmsRtState3Event)
	{
		temp16=(g_BmsRtStatus3[0].CycleNumbH<<8)+g_BmsRtStatus3[0].CycleNumbL;
		GattSetData(LIST_DTA,DTA_ACYC,(uint8_t*)&temp16);

		temp32=temp16;//acyc
		

		temp16=(g_BmsRtStatus3[0].FullBatCapH<<8)+g_BmsRtStatus3[0].FullBatCapL;
		temp16*=10;
		GattSetData(LIST_DTA,DTA_FCCP,(uint8_t*)&temp16);

		temp32=(temp32+1)*temp16; //cyc 
		

		temp16=(g_BmsRtStatus3[0].RemainBatCapH<<8)+g_BmsRtStatus3[0].RemainBatCapL;
		temp16*=10;
		GattSetData(LIST_DTA,DTA_RCAP,(uint8_t*)&temp16);

		temp32-=temp16;

		temp16=temp32/1000;

		GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&temp16);
		BmsRtState3Event=FALSE;
		}
	if(BmsCellVolt1Event)
	{
		LogPrintf("SupPwr:CAN DATA RCV   1111111111%x\r\n",BmsCellVolt1Event);
		temp16=(g_BmsCellVolt1[0].CellVoltage1H<<8)+g_BmsCellVolt1[0].CellVoltage1L;
		GattSetData(LIST_DIA,DIA_CV01,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1[0].CellVoltage2H<<8)+g_BmsCellVolt1[0].CellVoltage2L;
		GattSetData(LIST_DIA,DIA_CV02,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1[0].CellVoltage3H<<8)+g_BmsCellVolt1[0].CellVoltage3L;
		GattSetData(LIST_DIA,DIA_CV03,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt1[0].CellVoltage4H<<8)+g_BmsCellVolt1[0].CellVoltage4L;
		GattSetData(LIST_DIA,DIA_CV04,(uint8_t*)&temp16);
		
		BmsCellVolt1Event=FALSE;
		}
 
 
	if(BmsCellVolt4Event)
	{
		LogPrintf("SupPwr:CAN DATA RCV   1111111111%x\r\n",BmsCellVolt1Event);
		temp16=(g_BmsCellVolt4[0].CellVoltage13H<<8)+g_BmsCellVolt4[0].CellVoltage13L;
		GattSetData(LIST_DIA,DIA_CV13,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4[0].CellVoltage14H<<8)+g_BmsCellVolt4[0].CellVoltage14L;
		GattSetData(LIST_DIA,DIA_CV14,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4[0].CellVoltage15H<<8)+g_BmsCellVolt4[0].CellVoltage15L;
		GattSetData(LIST_DIA,DIA_CV15,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt4[0].CellVoltage16H<<8)+g_BmsCellVolt4[0].CellVoltage16L;
		GattSetData(LIST_DIA,DIA_CV16,(uint8_t*)&temp16);
		BmsCellVolt4Event=FALSE;
	}
	if(BmsCellVolt5Event)
	{
		LogPrintf("SupPwr:CAN DATA RCV   1111111111%x\r\n",BmsCellVolt1Event);
		temp16=(g_BmsCellVolt5[0].CellVoltage17H<<8)+g_BmsCellVolt5[0].CellVoltage17L;
		GattSetData(LIST_DIA,DIA_CV17,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5[0].CellVoltage18H<<8)+g_BmsCellVolt5[0].CellVoltage18L;
		GattSetData(LIST_DIA,DIA_CV18,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5[0].CellVoltage19H<<8)+g_BmsCellVolt5[0].CellVoltage19L;
		GattSetData(LIST_DIA,DIA_CV19,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt5[0].CellVoltage20H<<8)+g_BmsCellVolt5[0].CellVoltage20L;
		GattSetData(LIST_DIA,DIA_CV20,(uint8_t*)&temp16);
		BmsCellVolt5Event=FALSE;
	}

	if(BmsCellVolt6Event)
	{
		temp16=(g_BmsCellVolt6[0].CellVoltage21H<<8)+g_BmsCellVolt6[0].CellVoltage21L;
		GattSetData(LIST_DIA,DIA_CV21,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt6[0].CellVoltage22H<<8)+g_BmsCellVolt6[0].CellVoltage22L;
		GattSetData(LIST_DIA,DIA_CV22,(uint8_t*)&temp16);
		temp16=(g_BmsCellVolt6[0].CellVoltage23H<<8)+g_BmsCellVolt6[0].CellVoltage23L;
		GattSetData(LIST_DIA,DIA_CV23,(uint8_t*)&temp16);
		//temp16=(g_BmsCellVolt6.CellVoltage24H<<8)+g_BmsCellVolt6.CellVoltage24L;
		//GattSetData(LIST_DIA,DIA_CV23,(uint8_t*)&temp16);
		BmsCellVolt6Event=FALSE;
		}
	if(BmsRtTempEvent)
	{
		int rt_temp=0;
		BmsRtTempEvent=FALSE;

		rt_temp=g_BmsRtTemp[0].BmsTemperature1-40;
		GattSetData(LIST_DIA,DIA_TEMP1,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp[0].BmsTemperature2-40;
		GattSetData(LIST_DIA,DIA_TEMP2,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp[0].BmsTemperature3-40;
		GattSetData(LIST_DIA,DIA_TEMP3,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp[0].BmsTemperature4-40;
		GattSetData(LIST_DIA,DIA_TEMP4,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp[0].BmsTemperature7-40;
		GattSetData(LIST_DIA,DIA_TEMP5,(uint8_t*)&rt_temp);
		rt_temp=g_BmsRtTemp[0].BmsTemperature8-40;
		GattSetData(LIST_DIA,DIA_TEMP6,(uint8_t*)&rt_temp);
	}
	#endif
	
    #ifdef CHARGE_STATION
	if(BmsRtState1Event)
	{
		temp32=(g_BmsRtStatus1[0].RealtimeVoltageH<<8)+g_BmsRtStatus1[0].RealtimeVoltageL;
		temp32*=100;
		
		tempInt16=(int16_t)((g_BmsRtStatus1[0].RealtimeCurrentH<<8)+g_BmsRtStatus1[0].RealtimeCurrentL);

		tempInt16-=2000;
		tempInt16*=100;
		
	//	LogPrintf("SupPwr:CAN DATA RCV   1111111111  %d\r\n",temp32);

		if(PaygGetPayState()==FALSE&&PaygGetFreeState()==FALSE)
		{
			if(tempInt16>500)
			{	
				if(g_SupPwrCharge_Counter<100)
				{	
					g_SupPwrCharge_Counter++;

					if(g_SupPwrCharge_Counter>10)
					{	g_SupPwrrun_protect=FALSE;
						LogPrintf("SupPwr:Protect cancel %d \r\n",g_SupPwrCharge_Counter);
					}
				}
			}
			else
			{
				g_SupPwrCharge_Counter=0;
			}
		}
		else
		{
			g_SupPwrCharge_Counter=0;
		}
		

		if(tempInt16>0)
		{	power=temp32/100*tempInt16/100/100;
			temp16=power;
			//GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			//GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			temp16=0;
			//GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}
		else if(tempInt16<0)
		{	power=temp32/100*-tempInt16/100/100;
			temp16=0;
			///GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			//GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			temp16=power;
			///GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}
		else
		{
			temp16=0;
			//GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&temp16);
			//GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&temp16);
			//GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&temp16);
		}

		temp16=g_BmsRtStatus1[0].ChargingMosOn;
		//GattSetData(LIST_DTA,DTA_CMOS,(uint8_t*)&temp16);
		temp16=g_BmsRtStatus1[0].DisChargeMosOn;
		//GattSetData(LIST_DTA,DTA_DMOS,(uint8_t*)&temp16);

		p_u8=(uint8_t*)&g_BmsRtStatus1[0];
		temp16=p_u8[0]|(p_u8[1]<<8);
		temp16&=0x3ffc;
		//GattSetData(LIST_DTA,DTA_PPST,(uint8_t*)&temp16);

		temp16=p_u8[0]&0x03;
		//GattSetData(LIST_DTA,DTA_PMCS,(uint8_t*)&temp16);

		for(i=0;i<CHARGE_NUM;i++)
		{
			PLC_BATINFOR_TypeDef  tempPlcbatInfor;
			uint8_t buffer[32];
			uint32_t serialno=0;

			memset((uint8_t*)&tempPlcbatInfor,0x00,sizeof(tempPlcbatInfor));

			temp32=(g_BmsRtStatus1[i].RealtimeVoltageH<<8)+g_BmsRtStatus1[i].RealtimeVoltageL;
			temp32*=100;
			//tempPlcbatInfor.ChargeVoltage=temp32;
			
			g_SlotBmsInfor[i].ChargeVoltage = temp32;
			memcpy((uint8_t*)&tempPlcbatInfor.ChargeVoltage,(uint8_t*)&temp32,4);

			tempInt16=(int16_t)((g_BmsRtStatus1[i].RealtimeCurrentH<<8)+g_BmsRtStatus1[i].RealtimeCurrentL);
			tempInt16-=2000;
			//tempInt16*=100;

			g_SlotBmsInfor[i].ChargeCurrent = tempInt16;
			
			 

			if(tempInt16<0)
				tempInt16*=-1;
			//tempPlcbatInfor.ChargeCurrent=tempInt16;
			memcpy((uint8_t*)&tempPlcbatInfor.ChargeCurrent,(uint8_t*)&tempInt16,2);
			
			tempPlcbatInfor.Soc = g_BmsRtStatus2[i].Soc;

			g_SlotBmsInfor[i].Soc = g_BmsRtStatus2[i].Soc;

			temp16=(g_BmsRtStatus3[i].RemainBatCapH<<8)+g_BmsRtStatus3[i].RemainBatCapL;
		 //  printf("CAN BMS data: [%d].temp16 =  %d\r\n" ,i ,g_SlotBmsInfor[i].ChargeCurrent);  

			temp32=temp16*10*temp32/1000;
			//temp32=temp16*10/1000;
			if(g_BmsLostTimeCount[i]>=4)
			{	
				memset((uint8_t*)&g_BmsRtStatus1[i],0x00,8);
				memset((uint8_t*)&g_BmsRtStatus2[i],0x00,8);
				memset((uint8_t*)&g_BmsRtStatus3[i],0x00,8);
				memset((uint8_t*)&tempPlcbatInfor,0x00,sizeof(tempPlcbatInfor));
			}
			else
				memcpy((uint8_t*)&tempPlcbatInfor.RemainCap,(uint8_t*)&temp32,4);

		
			
			bms_vol[i] = (g_BmsCellVolt4[i].CellVoltage14H<<8)+g_BmsCellVolt4[i].CellVoltage14L;;
			if(bms_vol[i] > 3000) bms_num[i] =14;
			bms_vol[i] = (g_BmsCellVolt4[i].CellVoltage16H<<8)+g_BmsCellVolt4[i].CellVoltage16L;;
			if(bms_vol[i] > 3000) bms_num[i] =16;
			bms_vol[i] = (g_BmsCellVolt5[i].CellVoltage20H<<8)+g_BmsCellVolt5[i].CellVoltage20L;
			if(bms_vol[i] > 3000) bms_num[i] =20;
			bms_vol[i] = (g_BmsCellVolt6[i].CellVoltage23H<<8)+g_BmsCellVolt6[i].CellVoltage23L;
			if(bms_vol[i] > 3000) bms_num[i] =23;
			bms_vol[i] = (g_BmsCellVolt6[i].CellVoltage24H<<8)+g_BmsCellVolt6[i].CellVoltage24L;
			if(bms_vol[i] > 3000) bms_num[i] =24;
		
			if(bms_num[i] == 0) bms_num[i] = 20;
			tempbms_vol_act[i] = (int16_t)((g_BmsRtStatus2[i].MaxChargeVoltageH<<8)+g_BmsRtStatus2[i].MaxChargeVoltageL);
			
			
			bat_per_vol[i] = tempbms_vol_act[i]/bms_num[i]*0.1;
			if(bat_per_vol[i] > 3.8)  bat_all_vol[i] = 3.6 * bms_num[i];
			else  bat_all_vol[i] = 3.2 * bms_num[i];

			g_SlotBmsInfor[i].RemainCap = bat_all_vol[i]*temp16;
			
 
		//	g_SlotBmsInfor[i].RemainCap = temp32/1000.0f/1000.0f;;
	
			PlcBmsInforUpdate(i,&tempPlcbatInfor);

			memset(buffer,00,32);

			memcpy(buffer,g_BmSerialNo[i][2].serialNo,6);
			memcpy(buffer+6,g_BmSerialNo[i][3].serialNo,2);

			if(g_BmsLostTimeCount[i]>=6)
			{	
				serialno=0;
				memset((uint8_t*)g_BmSerialNo[i],0x00,8*4);
				memset((uint8_t*)&g_SlotBmsInfor[i],0x00,sizeof(SLOT_BATINFOR_TypeDef));
			}
			else	
				serialno=atoll(buffer);
			 
			//g_SlotBmsInfor[i].BatteryID=serialno;
			memcpy(g_SlotBmsInfor[i].BatteryID,g_BmSerialNo[i][0].serialNo,6);
			memcpy(g_SlotBmsInfor[i].BatteryID+6,g_BmSerialNo[i][1].serialNo,6);
			memcpy(g_SlotBmsInfor[i].BatteryID+12,g_BmSerialNo[i][2].serialNo,6);
			memcpy(g_SlotBmsInfor[i].BatteryID+18,g_BmSerialNo[i][3].serialNo,4);

			g_SlotBmsInfor[i].BatteryID[g_BmSerialNo[i][0].size]=0x00;
			PlcBmsBatIdUpdate(i,serialno);

		//	printf("EmeterInfor  CAN BatteryID\r\n");
	  //my_printf_hex(g_SlotBmsInfor[i].BatteryID,32);

			//if(BmsRtState3Event)
			
			temp16=(g_BmsRtStatus3[i].CycleNumbH<<8)+g_BmsRtStatus3[i].CycleNumbL;
			//GattSetData(LIST_DTA,DTA_ACYC,(uint8_t*)&temp16);
			temp32=temp16;//acyc
			temp16=(g_BmsRtStatus3[i].FullBatCapH<<8)+g_BmsRtStatus3[i].FullBatCapL;
			temp16*=10;
			//GattSetData(LIST_DTA,DTA_FCCP,(uint8_t*)&temp16);
			temp32=(temp32+1)*temp16; //cyc 
			temp16=(g_BmsRtStatus3[i].RemainBatCapH<<8)+g_BmsRtStatus3[i].RemainBatCapL;
			temp16*=10;
			//GattSetData(LIST_DTA,DTA_RCAP,(uint8_t*)&temp16);
			temp32-=temp16;
			temp16=temp32/1000;
			//GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&temp16);
			//BmsRtState3Event=FALSE;
			
			g_SlotBmsInfor[i].cellVoltage[0]=(g_BmsCellVolt1[i].CellVoltage1H<<8)+g_BmsCellVolt1[i].CellVoltage1L;
			g_SlotBmsInfor[i].cellVoltage[1]=(g_BmsCellVolt1[i].CellVoltage2H<<8)+g_BmsCellVolt1[i].CellVoltage2L;
			g_SlotBmsInfor[i].cellVoltage[2]=(g_BmsCellVolt1[i].CellVoltage3H<<8)+g_BmsCellVolt1[i].CellVoltage3L;
			g_SlotBmsInfor[i].cellVoltage[3]=(g_BmsCellVolt1[i].CellVoltage4H<<8)+g_BmsCellVolt1[i].CellVoltage4L;

			g_SlotBmsInfor[i].cellVoltage[4]=(g_BmsCellVolt2[i].CellVoltage5H<<8)+g_BmsCellVolt2[i].CellVoltage5L;
			g_SlotBmsInfor[i].cellVoltage[5]=(g_BmsCellVolt2[i].CellVoltage6H<<8)+g_BmsCellVolt2[i].CellVoltage6L;
			g_SlotBmsInfor[i].cellVoltage[6]=(g_BmsCellVolt2[i].CellVoltage7H<<8)+g_BmsCellVolt2[i].CellVoltage7L;
			g_SlotBmsInfor[i].cellVoltage[7]=(g_BmsCellVolt2[i].CellVoltage8H<<8)+g_BmsCellVolt2[i].CellVoltage8L;

			g_SlotBmsInfor[i].cellVoltage[8]=(g_BmsCellVolt3[i].CellVoltage9H<<8)+g_BmsCellVolt3[i].CellVoltage9L;
			g_SlotBmsInfor[i].cellVoltage[9]=(g_BmsCellVolt3[i].CellVoltage10H<<8)+g_BmsCellVolt3[i].CellVoltage10L;
			g_SlotBmsInfor[i].cellVoltage[10]=(g_BmsCellVolt3[i].CellVoltage11H<<8)+g_BmsCellVolt3[i].CellVoltage11L;
			g_SlotBmsInfor[i].cellVoltage[11]=(g_BmsCellVolt3[i].CellVoltage12H<<8)+g_BmsCellVolt3[i].CellVoltage12L;
			
			g_SlotBmsInfor[i].cellVoltage[12]=(g_BmsCellVolt4[i].CellVoltage13H<<8)+g_BmsCellVolt4[i].CellVoltage13L;
			g_SlotBmsInfor[i].cellVoltage[13]=(g_BmsCellVolt4[i].CellVoltage14H<<8)+g_BmsCellVolt4[i].CellVoltage14L;
			g_SlotBmsInfor[i].cellVoltage[14]=(g_BmsCellVolt4[i].CellVoltage15H<<8)+g_BmsCellVolt4[i].CellVoltage15L;
			g_SlotBmsInfor[i].cellVoltage[15]=(g_BmsCellVolt4[i].CellVoltage16H<<8)+g_BmsCellVolt4[i].CellVoltage16L;
			
			g_SlotBmsInfor[i].cellVoltage[16]=(g_BmsCellVolt5[i].CellVoltage17H<<8)+g_BmsCellVolt5[i].CellVoltage17L;
			g_SlotBmsInfor[i].cellVoltage[17]=(g_BmsCellVolt5[i].CellVoltage18H<<8)+g_BmsCellVolt5[i].CellVoltage18L;
			g_SlotBmsInfor[i].cellVoltage[18]=(g_BmsCellVolt5[i].CellVoltage19H<<8)+g_BmsCellVolt5[i].CellVoltage19L;
			g_SlotBmsInfor[i].cellVoltage[19]=(g_BmsCellVolt5[i].CellVoltage20H<<8)+g_BmsCellVolt5[i].CellVoltage20L;

			g_SlotBmsInfor[i].cellVoltage[20]=(g_BmsCellVolt6[i].CellVoltage21H<<8)+g_BmsCellVolt6[i].CellVoltage21L;
			g_SlotBmsInfor[i].cellVoltage[21]=(g_BmsCellVolt6[i].CellVoltage22H<<8)+g_BmsCellVolt6[i].CellVoltage22L;
			g_SlotBmsInfor[i].cellVoltage[22]=(g_BmsCellVolt6[i].CellVoltage23H<<8)+g_BmsCellVolt6[i].CellVoltage23L;

			g_SlotBmsInfor[i].temp[0]=g_BmsRtTemp[i].BmsTemperature1-40;
			g_SlotBmsInfor[i].temp[1]=g_BmsRtTemp[i].BmsTemperature2-40;
			g_SlotBmsInfor[i].temp[2]=g_BmsRtTemp[i].BmsTemperature3-40;
			g_SlotBmsInfor[i].temp[3]=g_BmsRtTemp[i].BmsTemperature4-40;
			g_SlotBmsInfor[i].temp[4]=g_BmsRtTemp[i].BmsTemperature7-40;
			g_SlotBmsInfor[i].temp[5]=g_BmsRtTemp[i].BmsTemperature8-40;

		}
 
		BmsRtState1Event=FALSE;
	}
	#endif
	if(BmsSysInforEvent)
	{
		BmsSysInforEvent=FALSE;
	} 
	#endif
	#endif
	#endif
}




