#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef SUPERPOWER_PROJECT

#define BUP_data_uart_rx_size()  huart5.RxXferCount

//extern PAYG_TypeDef payg;
//extern uint8_t g_GattMem[MEM_GATT_SIZE];
__IO uint8_t g_superPowerGetInfor_Enable;
uint8_t* g_UartSuperPowerBuf;
uint8_t g_superpowercount=0;
uint8_t g_superpowercount2=0;
#ifdef LOWPOWER_SUPPORT
uint16_t g_LowPowerCounter=0;
uint16_t g_LowPowerMode=0;
uint16_t g_LowPowerHold=0;
uint16_t g_LowPowerSleepTime=0;
#endif
// Bms* g_p_bms1 = (Bms* )malloc(sizeof(Bms));
// ChargeController* g_p_chargeController1 = (ChargeController* )malloc(sizeof(ChargeController));
Bms* g_p_bms1 = NULL;
ChargeController* g_p_chargeController1 = NULL;

Bms g_bms1;
ChargeController g_chargeController1;

//抽象值
static uint16_t last_remaining_capacity = 0;
static uint32_t last_total_voltage = 0;
static int16_t last_current = 0;
static uint16_t last_rsoc = 0;
static uint16_t last_acyc = 0;
static uint16_t last_full_capacity = 0;
static uint16_t last_batp = 0;
static uint16_t last_outp = 0;
static uint16_t last_inpp = 0;
static uint16_t last_dmos = 0;
static uint16_t last_cmos = 0;
static uint16_t last_pckt = 0;
static uint16_t last_ppst = 0;
static uint16_t last_cell_voltages[30] = {0};
static uint16_t last_ntc_temperatures[10] = {0};

// 将指令以十六进制数组的形式定义
uint8_t command_voltage[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x02, 0xF9, 0xF5};
uint8_t command_current[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x03, 0xF8, 0xF5};
uint8_t command_battery[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x04, 0xFF, 0xF5};
uint8_t command_code[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x11, 0xEA, 0xF5};
uint8_t command_discharge_allow[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x19, 0xE2, 0xF5};
uint8_t command_discharge_deny[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x1A, 0xE1, 0xF5};
uint8_t command_charge_allow[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x1B, 0xE0, 0xF5};
uint8_t command_charge_deny[] = {0xEA, 0xD1, 0x01, 0x04, 0xFF, 0x1C, 0xE7, 0xF5};
uint8_t command_heartbeat[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x20, 0xDA, 0xF5};
uint8_t command_close_watchdog[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x21, 0xDB, 0xF5};
uint8_t command_open_watchdog[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x22, 0xD8, 0xF5};
uint8_t command_alarm[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x23, 0xD9, 0xF5};
//uint8_t command_feeddog[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x24, 0xDE, 0xF5};
uint8_t command_feeddog[] = {0xEA, 0xD1, 0x01, 0x04, 0xFE, 0x18, 0xE2, 0xF5};


extern UART_HandleTypeDef huart5;


// 创建一个指令数组的数组
uint8_t* commands[] = {
    command_voltage,
    command_current,
    command_battery,
    command_code,
    command_discharge_allow,
    command_discharge_deny,
    command_charge_allow,
    command_charge_deny,
    command_heartbeat,
    command_close_watchdog,
    command_open_watchdog,
    command_alarm,
    command_feeddog
    // ... 在这里添加其他指令
};

// 函数用于通过索引获取指令
uint8_t* getCommand(uint8_t index) 
{
    if (index >= 0 && index < COMMOND_COUNT)
    {
        return commands[index];
    }
    LOG("getCommand index error\n");
    return NULL;  // 如果索引越界，返回 NULL
}

//函数发送指令
void sendCommand(uint8_t index) 
{
    uint8_t* command = getCommand(index);
    //BUP_disconnect_handler();
    //BUP_connect_handler();
    if (command != NULL) 
    {
        superPoweruartSend(command, 8);
    }
}

void superPower_timeout_timer_start(void)
{
  LOG("BLE start SBP_SAMPLE_TIMER Timer\n");	
  //osal_start_timerEx(bleuart_TaskID, SBP_SAMPLE_TIMER, 1000);
}


void superPoweruartSend(uint8_t *buffer,uint16_t size)
{
		//Uart3Send(buffer,size);
   LOG("buff 0:%x size %d \r\n",buffer[0],size);
  // hal_uart_send_buff(UART1,(uint8_t*)buffer,size);
  Uart5Send(buffer,size);
}

uint8_t superPowerLowpowerMode(void)
{
	return g_LowPowerMode;
}
void superPowerGetEnable(void)
{
	g_superPowerGetInfor_Enable=TRUE;
}


/*
// 通用结构体序列化函数
void serializeStruct(const void* struct_ptr, uint8_t* buffer, size_t struct_size, size_t buffer_size) {
    size_t offset = 0;
    size_t bytes_to_copy = struct_size - sizeof(((Bms*)0)->device);

    if (buffer_size < bytes_to_copy) {
        printf("Error: Buffer too small to hold the serialized data.\n");
        return;
    }

    // 跳过device成员并将其他成员复制到buffer中
    memcpy(buffer, ((const uint8_t*)struct_ptr) + sizeof(((Bms*)0)->device), bytes_to_copy);
}
*/
void GetUartInfo(void* p_dev,uint8_t* data)
{
    uint8_t i;
    uint8_t* buffer;
    uint16_t size;
    Bms* p_bms;
    ChargeController* p_cc;

    
    //检测p_dev是否为空
    if(p_dev == NULL)
    {
        LOG("p_dev is NULL %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    

    if(BUP_data_uart_rx_size() == 0)
    {
        LOG("g_UartSuperPowerBuf is empty %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    
    //buffer = g_UartSuperPowerBuf;
    buffer = data;
    size = huart5.RxXferCount;//BUP_data_uart_rx_size();

    //LOG("GetUartInfo size:%d\n", size);

    //printUartInfo(buffer, size);
    //printUartInfo(g_UartSuperPowerBuf, size);
    if(buffer[0]==0xEA&&buffer[1]==0xD1&&buffer[2]==0x01&&(buffer[4]==0xFF||buffer[4]==0xFE))
    {
    p_bms = (Bms*)p_dev;
    switch (buffer[5])
    {
    case 0x02:
    {
        LOG("voltage:\n");
        p_bms->num_ntc_probes = buffer[7];
        p_bms->num_battery_strings = buffer[8];
        for(i = 0; i < p_bms->num_battery_strings; i++)
        {
            p_bms->cell_voltages[i] = (buffer[9 + i * 2] << 8) + buffer[10 + i * 2];
        }        
        break;
    }
    case 0x03:
    {
        LOG("current:\n");
        p_bms->device.status = buffer[6];
        p_bms->u16_current = ((buffer[7] << 8) + buffer[8])*10;
        //p_bms->current = (int16_t)p_bms->u16_current;
        
        p_bms->discharge_cycles = (buffer[9] << 8) + buffer[10];
        p_bms->ppst = buffer[12];
        p_bms->num_ntc_probes = buffer[13];
        for(i = 0; i < p_bms->num_ntc_probes; i++)
        {
            p_bms->ntc_temperatures[i] = buffer[14 + i]-40u;
        }
        p_bms->pckt = (p_bms->ntc_temperatures[0]+p_bms->ntc_temperatures[1]+p_bms->ntc_temperatures[2]+p_bms->ntc_temperatures[3])/4;
        p_bms->mos_status = buffer[20 + p_bms->num_ntc_probes];
        if(p_bms->mos_status&0x02 != 0)
        {
            p_bms->dmos = 1;
        }
        else
        {
            p_bms->dmos = 0;
        }
        //检测p_bms->mos_status的第三位
        if(p_bms->mos_status&0x04 != 0)
        {
            p_bms->cmos = 1;
        }
        else
        {
            p_bms->cmos = 0;
        }

        if(((p_bms->device.status>>0)&0x01)==1||p_bms->device.status==0x31)//放电||p_bms->device.status==0x31
        {
            LOG("discharge\n");
            p_bms->current = -1*p_bms->u16_current;
            p_bms->outp = p_bms->u16_current*p_bms->u32_total_voltage/1000000;
            p_bms->batp = 0;
            p_bms->inpp = 0;
        }
        else if(((p_bms->device.status>>1)&0x01)==1||p_bms->device.status==0x32)//充电||p_bms->device.status==0x32
        {
            LOG("charge\n");
            p_bms->current = p_bms->u16_current;
            p_bms->outp = 0;
            p_bms->batp = p_bms->u16_current*p_bms->u32_total_voltage/1000000;
            p_bms->inpp = p_bms->u16_current*p_bms->u32_total_voltage/1000000;
        }
        else
        {
            LOG("no discharge and no charge%s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
            //打印p_bms->device.status 16进制
            LOG("p_bms->device.status:%x\n", p_bms->device.status);
            p_bms->outp = 0;
            p_bms->batp = 0;
            p_bms->inpp = 0;
            p_bms->u16_current=0;
            p_bms->current=0;
        }
        break;
    }
    case 0x04:
    {
        LOG("battery:\n");
        p_bms->rsoc = buffer[7];
        p_bms->acyc = (buffer[9] << 8) + buffer[10];
        p_bms->full_capacity = (buffer[21] << 8) + buffer[22];
        p_bms->remaining_capacity = (buffer[27] << 8) + buffer[28];
        p_bms->u32_total_voltage = ((buffer[47] << 8) + buffer[48])*10;
        p_bms->total_voltage = ((buffer[47] << 8) + buffer[48])*10;
        break;
    }


    default:
    {
        LOG("default %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        break;
    }
    }
    //return;
    }

    if(buffer[0]==0x00&&buffer[1]==0x01&&buffer[2]==0x02&&buffer[3]==0x03&&buffer[i+3]==0x04&&buffer[i+3]==0x05)
    {        
        switch (buffer[4])
        {
        case 0x01:
        { 
            LOG("my p_bms\n");  
            p_bms = (Bms*)p_dev;
            deserializeStruct(p_bms, &buffer[6], sizeof(Bms)-6);
            break;
        }
        case 0x02:
        {
            LOG("my p_cc\n");
            p_cc = (ChargeController*)p_dev;
            deserializeStruct(p_cc, &buffer[6], sizeof(Bms)-6);
            break;
        }    
        default:
            break;
        }
    }
    //BUP_disconnect_handler();


}


void GetallsuperpowerUartInfo(void* p_dev)
{
    uint16_t i;
    uint8_t* buffer;
    uint16_t size;
    Bms* p_bms;

    
    //检测p_dev是否为空
    if(p_dev == NULL)
    {
        LOG("p_dev is NULL %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    

    if(BUP_data_uart_rx_size() == 0)
    {
        LOG("g_UartSuperPowerBuf is empty %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    p_bms = (Bms*)p_dev;
    buffer = g_UartSuperPowerBuf;
    size = BUP_data_uart_rx_size();

    //printUartInfo(g_UartSuperPowerBuf, size);

    for(i = 0;i < size; i++ )
    {
        if(buffer[i]==0xEA&&buffer[i+1]==0xD1&&buffer[i+2]==0x01&&(buffer[i+4]==0xFF||buffer[i+4]==0xFE))
        {
            GetUartInfo(p_dev,&buffer[i]);
        }
        if(buffer[i]==0x00&&buffer[i+1]==0x01&&buffer[i+2]==0x02&&buffer[i+3]==0x03&&buffer[i+3]==0x04&&buffer[i+3]==0x05)
        {
            GetUartInfo(p_dev,&buffer[i]);
        }
    }
    //BUP_disconnect_handler();
}

void publish_battery_data(void* p_dev)
{
    uint8_t i;
	uint16_t temp16;
	
    Bms* p_bms = (Bms*)p_dev;
    //发布电池剩余容量
    if(p_bms->remaining_capacity != last_remaining_capacity)
    {
        //OvDtaProfile_Notify(BLE_DTA_RCAP-BLE_DTA_BATP,MEM_SIZE_RCAP,(uint8_t*)&(p_bms->remaining_capacity));
        last_remaining_capacity = p_bms->remaining_capacity;
		GattSetData(LIST_DTA,DTA_RCAP,(uint8_t*)&last_remaining_capacity);
    }

    //发布电池总电压
    if(p_bms->u32_total_voltage != last_total_voltage)
    {
        //OvDtaProfile_Notify(BLE_DTA_PCKV-BLE_DTA_BATP,MEM_SIZE_PCKV,(uint8_t*)&(p_bms->u32_total_voltage));
        last_total_voltage = p_bms->u32_total_voltage;
		GattSetData(LIST_DTA,DTA_PCKV,(uint8_t*)&last_total_voltage);
    }

    //发布电池电流
    if(p_bms->current != last_current)
    {
        //OvDtaProfile_Notify(BLE_DTA_PCKC-BLE_DTA_BATP,MEM_SIZE_PCKC,(uint8_t*)&(p_bms->current));
        last_current = p_bms->current;
		GattSetData(LIST_DTA,DTA_PCKC,(uint8_t*)&last_current);
    }

    //发布电池RSOC
    if(p_bms->rsoc != last_rsoc)
    {
        //OvDtaProfile_Notify(BLE_DTA_RSOC-BLE_DTA_BATP,MEM_SIZE_RSOC,(uint8_t*)&(p_bms->rsoc));

        last_rsoc = p_bms->rsoc;
		GattSetData(LIST_DTA,DTA_RSOC,(uint8_t*)&last_rsoc);
    }

    //发布电池ACYC
    if(p_bms->acyc != last_acyc)
    {
        //OvDtaProfile_Notify(BLE_DTA_ACYC-BLE_DTA_BATP,MEM_SIZE_ACYC,(uint8_t*)&(p_bms->acyc));
        last_acyc = p_bms->acyc;
		GattSetData(LIST_DTA,DTA_ACYC,(uint8_t*)&last_acyc);
    }

    //发布电池FCCP
    if(p_bms->full_capacity != last_full_capacity)
    {
    	
        //OvDtaProfile_Notify(BLE_DTA_FCCP-BLE_DTA_BATP,MEM_SIZE_FCCP,(uint8_t*)&(p_bms->full_capacity));
        last_full_capacity = p_bms->full_capacity;
		GattSetData(LIST_DTA,DTA_FCCP,(uint8_t*)&last_full_capacity);

		GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&last_acyc);
    }

	temp16=last_acyc*last_full_capacity/1000;
	GattSetData(LIST_DTA,DTA_AENG,(uint8_t*)&temp16);

    //发布电池batp
    if(p_bms->batp != last_batp)
    {
       // OvDtaProfile_Notify(BLE_DTA_BATP-BLE_DTA_BATP,MEM_SIZE_BATP,(uint8_t*)&(p_bms->batp));
        last_batp = p_bms->batp;
	    GattSetData(LIST_DTA,DTA_BATP,(uint8_t*)&last_batp);
    }

    //发布电池outp
    if(p_bms->outp != last_outp)
    {
        //OvDtaProfile_Notify(BLE_DTA_OUTP-BLE_DTA_BATP,MEM_SIZE_OUTP,(uint8_t*)&(p_bms->outp));
        last_outp = p_bms->outp;
		GattSetData(LIST_DTA,DTA_OUTP,(uint8_t*)&last_outp);
    }

    //发布电池inpp
    if(p_bms->inpp != last_inpp)
    {
       // OvDtaProfile_Notify(BLE_DTA_INPP-BLE_DTA_BATP,MEM_SIZE_INPP,(uint8_t*)&(p_bms->inpp));
        last_inpp = p_bms->inpp;
		GattSetData(LIST_DTA,DTA_INPP,(uint8_t*)&last_inpp);
    }

    //发布电池dmos
    if(p_bms->dmos != last_dmos)
    {
        //OvDtaProfile_Notify(BLE_DTA_DMOS-BLE_DTA_BATP,MEM_SIZE_DMOS,(uint8_t*)&(p_bms->dmos));
        last_dmos = p_bms->dmos;
		GattSetData(LIST_DTA,DTA_DMOS,(uint8_t*)&last_dmos);
    }

    //发布电池cmos
    if(p_bms->cmos != last_cmos)
    {
        //OvDtaProfile_Notify(BLE_DTA_CMOS-BLE_DTA_BATP,MEM_SIZE_CMOS,(uint8_t*)&(p_bms->cmos));
        last_cmos = p_bms->cmos;
		GattSetData(LIST_DTA,DTA_CMOS,(uint8_t*)&last_cmos);
    }

    //发布电池pckt
    if(p_bms->pckt != last_pckt)
    {
        //OvDtaProfile_Notify(BLE_DTA_PCKT-BLE_DTA_BATP,MEM_SIZE_PCKT,(uint8_t*)&(p_bms->pckt));
        last_pckt = p_bms->pckt;
		GattSetData(LIST_DTA,DTA_PCKT,(uint8_t*)&last_pckt);
    }

    //发布电池ppst
    if(p_bms->ppst != last_ppst)
    {
        //OvDtaProfile_Notify(BLE_DTA_PPST-BLE_DTA_BATP,MEM_SIZE_PPST,(uint8_t*)&(p_bms->ppst));
        last_ppst = p_bms->ppst;
		GattSetData(LIST_DTA,DTA_PPST,(uint8_t*)&last_ppst);
    }

    //发布电池dia
    for(i=0;i<p_bms->num_battery_strings;i++)
    {
        //LOG("no publish battery cell_voltages\n");
        if(p_bms->cell_voltages[i] != last_cell_voltages[i])
        {
           // OvDiaProfile_Notify(i/*BLE_DIA_CV01-BLE_DIA_CV01*/,MEM_SIZE_CV01+i,(uint8_t*)&(p_bms->cell_voltages[i]));
           
            last_cell_voltages[i] = p_bms->cell_voltages[i];
		   GattSetData(LIST_DIA,DIA_CV01+i,(uint8_t*)&last_cell_voltages[i]);
        }
    }
    //发布电池ntc
    // for(i=p_bms->num_battery_strings;i<p_bms->num_ntc_probes+p_bms->num_battery_strings;i++)
    // {
    //     LOG("no publish battery ntc_temperatures\n");
    //     if(p_bms->ntc_temperatures[i-p_bms->num_battery_strings] != last_ntc_temperatures[i-p_bms->num_battery_strings])
    //     {
    //         OvDiaProfile_Notify(i,MEM_SIZE_NTC1+i,(uint8_t*)&(p_bms->ntc_temperatures[i-p_bms->num_battery_strings]));
    //         last_ntc_temperatures[i-p_bms->num_battery_strings] = p_bms->ntc_temperatures[i-p_bms->num_battery_strings];
    //         //LOG("last_ntc_temperatures[%d]:%d\n", i-p_bms->num_battery_strings, last_ntc_temperatures[i-p_bms->num_battery_strings]);
        
    //     }
    // }
    for(i=0;i<6;i++)
    {
        //OvDiaProfile_Notify(i,MEM_SIZE_TEMP1+i,(uint8_t*)&(p_bms->ntc_temperatures[i-p_bms->num_battery_strings]));

		if(last_ntc_temperatures[i] != p_bms->ntc_temperatures[i])
		{	last_ntc_temperatures[i] = p_bms->ntc_temperatures[i];
       		 GattSetData(LIST_DIA,DIA_TEMP1+i,(uint8_t*)&(last_ntc_temperatures[i]));
			}
    }
}

//蓝牙发布指针设备的数据
void superPowerPublish(void* p_dev)
{
    uint8_t i=0;
    Bms* p_bms;
    //ChargeController* p_chargeController;

    //检测p_dev是否为空
    if(p_dev == NULL)
    {
        LOG("p_dev is NULL %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }


    p_bms = (Bms*)p_dev;
    //p_chargeController = (ChargeController*)p_dev;

    // if(p_bms->full_capacity == 0&&p_bms->u32_total_voltage == 0)
    // {
    //     LOG("everything is 0  %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
    //     return;
    // }
    // else
    // {
    //     LOG("everything is not 0  %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
    // }

    
    // //发布电池剩余容量
    // OvDtaProfile_Notify(BLE_DTA_RCAP-BLE_DTA_BATP,MEM_SIZE_RCAP,(uint8_t*)&(p_bms->remaining_capacity));
    // //发布电池总电压
    // OvDtaProfile_Notify(BLE_DTA_PCKV-BLE_DTA_BATP,MEM_SIZE_PCKV,(uint8_t*)&(p_bms->u32_total_voltage));
    // //发布电池温度
    // OvDtaProfile_Notify(BLE_DTA_PCKT-BLE_DTA_BATP,MEM_SIZE_PCKT,(uint8_t*)&(p_bms->ntc_temperatures));
    // //发布电池电流
    // OvDtaProfile_Notify(BLE_DTA_PCKC-BLE_DTA_BATP,MEM_SIZE_PCKC,(uint8_t*)&(p_bms->current));
    // //发布电池RSOC
    // OvDtaProfile_Notify(BLE_DTA_RSOC-BLE_DTA_BATP,MEM_SIZE_RSOC,(uint8_t*)&(p_bms->rsoc));
    // //发布电池ACYC
    // OvDtaProfile_Notify(BLE_DTA_ACYC-BLE_DTA_BATP,MEM_SIZE_ACYC,(uint8_t*)&(p_bms->acyc));
    // //发布电池FCCP
    // OvDtaProfile_Notify(BLE_DTA_FCCP-BLE_DTA_BATP,MEM_SIZE_FCCP,(uint8_t*)&(p_bms->full_capacity));
    // //发布电池batp
    // OvDtaProfile_Notify(BLE_DTA_BATP-BLE_DTA_BATP,MEM_SIZE_BATP,(uint8_t*)&(p_bms->batp));
    // //发布电池outp
    // OvDtaProfile_Notify(BLE_DTA_OUTP-BLE_DTA_BATP,MEM_SIZE_OUTP,(uint8_t*)&(p_bms->outp));
    // //发布电池inpp
    // OvDtaProfile_Notify(BLE_DTA_INPP-BLE_DTA_BATP,MEM_SIZE_INPP,(uint8_t*)&(p_bms->inpp));
    // //发布电池dmos
    // OvDtaProfile_Notify(BLE_DTA_DMOS-BLE_DTA_BATP,MEM_SIZE_DMOS,(uint8_t*)&(p_bms->dmos));
    // //发布电池cmos
    // OvDtaProfile_Notify(BLE_DTA_CMOS-BLE_DTA_BATP,MEM_SIZE_CMOS,(uint8_t*)&(p_bms->cmos));
    // //发布电池pckt
    // OvDtaProfile_Notify(BLE_DTA_PCKT-BLE_DTA_BATP,MEM_SIZE_PCKT,(uint8_t*)&(p_bms->pckt));
    // //发布电池ppst
    // OvDtaProfile_Notify(BLE_DTA_PPST-BLE_DTA_BATP,MEM_SIZE_PPST,(uint8_t*)&(p_bms->ppst));

    // //发布电池dia
    // for(i=0;i<p_bms->num_battery_strings;i++)
    // {
    //     OvDiaProfile_Notify(i/*BLE_DIA_CV01-BLE_DIA_CV01*/,MEM_SIZE_CV01+i,(uint8_t*)&(p_bms->cell_voltages[i]));
    // }
    // for(i=p_bms->num_battery_strings;i<p_bms->num_ntc_probes+p_bms->num_battery_strings;i++)
    // {
    //     OvDiaProfile_Notify(i,MEM_SIZE_NTC1+i,(uint8_t*)&(p_bms->ntc_temperatures[i-p_bms->num_battery_strings]));
    // }

    publish_battery_data(p_bms);

}

void printUartInfo(uint8_t* Buffer, uint16_t size)
{
    uint8_t i;
    if(size == 0)
    {
        LOG("Buffer is NULL %s-%s-%u\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    LOG("size:%u %s-%s-%u\n", size, __FILE__, __FUNCTION__, __LINE__);
    for(i = 0; i < size; i++)
    {
        LOG("%x ", Buffer[i]);
    }
    LOG("\n");
}



//打印superpowerbms的各项数据
void printSuperPowerBmsInfo(void* p_dev)
{
    uint8_t i;
    Bms* p_bms = (Bms*)p_dev;
    //检测指针是否为空
    if (p_bms == NULL)
    {
        LOG("printSuperPowerBmsInfo p_bms is NULL\n");
        return;
    }
    LOG("device status:%x\n", p_bms->device.status);
    LOG("rsoc:%u\n", p_bms->rsoc);
    LOG("acyc:%u\n", p_bms->acyc);
    LOG("full capacity:%u\n", p_bms->full_capacity);
    LOG("remaining capacity:%u\n", p_bms->remaining_capacity);
    LOG("total voltage:%u\n", p_bms->total_voltage);
    LOG("U32_total voltage:%u\n", p_bms->u32_total_voltage);
    LOG("u16_current:%u\n", p_bms->u16_current);
    LOG("current:%d\n", p_bms->current);
    LOG("discharge cycles:%u\n", p_bms->discharge_cycles);
    LOG("num ntc probes:%u\n", p_bms->num_ntc_probes);
    for(i = 0; i < p_bms->num_ntc_probes; i++)
    {
        LOG("temperature %u:%u\n", i, p_bms->ntc_temperatures[i]);
        LOG("last_ntc_temperatures[%d]:%d\n", i, last_ntc_temperatures[i]);

    }
    LOG("num battery strings:%u\n", p_bms->num_battery_strings);
    for(i = 0; i < p_bms->num_battery_strings; i++)
    {
        LOG("cell voltage %u:%u\n", i, p_bms->cell_voltages[i]);
    }
    LOG("mos status:%u\n", p_bms->mos_status);
    LOG("dmos:%u\n", p_bms->dmos);
    LOG("cmos:%u\n", p_bms->cmos);
    LOG("pckt:%u\n", p_bms->pckt);
    LOG("ppst:%u\n", p_bms->ppst);
    LOG("batp:%u\n", p_bms->batp);
    LOG("outp:%u\n", p_bms->outp);
    LOG("inpp:%u\n", p_bms->inpp);

}
#ifdef LOWPOWER_SUPPORT

void LowPowerHold(uint8_t hold)
{
	g_LowPowerHold=hold;
}
void superPowerBkpInit(void)
{
	/* BKP clock enable */
    rcu_periph_clock_enable(RCU_BKPI);
	g_LowPowerCounter=bkp_data_read(BKU_COUNTER_REG);
	g_LowPowerMode=bkp_data_read(BKU_MODE_REG);
	g_LowPowerHold=bkp_data_read(BKU_HOLD_REG);
	g_LowPowerSleepTime=bkp_data_read(BKU_SLEEPTIME_REG);

	if(bkp_data_read(BKU_PORST_REG)==0)
	{
		//bkp_deinit();
		pmu_backup_write_enable();
		bkp_data_write(BKU_PORST_REG,1);
		g_LowPowerHold=TRUE;
		/* clear the RCU all reset flags */
       // rcu_all_reset_flag_clear();
		
		}
}
#endif
void superPowerInit(void)
{
    // g_p_bms1 = (Bms*)malloc(sizeof(Bms));
    //initializeBms(g_p_bms1,1);

    initializeBms(&g_bms1,1);
	g_UartSuperPowerBuf=(uint8_t*)huart5.pRxBuffPtr;
}




__IO uint16_t g_cv_01_addr=MEM_ADDR_CV01;
__IO uint16_t g_ble_addr=BLE_DIA_CV01;

void superPowerTask(void)
{ 
    uint8_t i=0;
	uint16_t temp16=3330;
    // sendCommand(COMMOND_VOLTAGE);
    // WaitMs(10);
    // // GetUartInfo(g_p_bms1);
    // GetUartInfo(&g_bms1);

    // sendCommand(COMMOND_CURRENT);
    // WaitMs(10);
    
    // // GetUartInfo(g_p_bms1);
    // GetUartInfo(&g_bms1);


    // sendCommand(COMMOND_BATTERY);
    // WaitMs(10);
    
    // // GetUartInfo(g_p_bms1);
    // GetUartInfo(&g_bms1);

	//GattSetData(LIST_DIA,DIA_CV01,(uint8_t*)&temp16);


    if(g_superPowerGetInfor_Enable!=TRUE)
		return ;

	g_superPowerGetInfor_Enable=FALSE;
    // for(i=0;i<COMMOND_BATTERY+1;i++)
    // {
    //     WaitUs(20);
    //     sendCommand(i);
    //     WaitMs(100);
    //     GetUartInfo(&g_bms1);
    //     WaitMs(20);
    // }




    // for(i=0;i<COMMOND_BATTERY+1;i++)
    // {
    //     //WaitMs(100);
    //     sendCommand(i);
    //     WaitMs(50);
    // }
    // printUartInfo(g_UartSuperPowerBuf, BUP_data_uart_rx_size());
    // GetallsuperpowerUartInfo(&g_bms1);  //使用时要改GetUartInfo()关闭disconnect
    // superPowerPublish(&g_bms1);
    // printSuperPowerBmsInfo(&g_bms1);
    LOG("PayState %02X  Free state %d  \r\n",PaygGetPayState(),PaygGetFreeState());
    // if(PaygGetPayState())
    // {
    //     sendCommand(COMMOND_OPEN_WATCHDOG);
    //     WaitMs(100);
    // }

    // if(PaygGetFreeState())
    // {
    //     sendCommand(COMMOND_CLOSE_WATCHDOG);
    //     WaitMs(100);
    // }
    // if(PaygGetRemainDays()>0&&PaygGetPayState())
    // {
    //     sendCommand(COMMOND_FEEDDOG);
    //     WaitMs(100);
    // }
    // if(PaygGetRemainDays()<=3)
    // {
    //     sendCommand(COMMOND_ALARM);
    //     WaitMs(100);
    // }
   #ifdef LOWPOWER_SUPPORT
	/*if(g_bms1.current<=500&&g_bms1.current>=-500)
	{	
		if(g_LowPowerCounter<=60)
		     g_LowPowerCounter++;
		}
	else*/
	{
		g_LowPowerCounter=0;
		if(g_LowPowerMode)
		{
			bkp_data_write(BKU_MODE_REG,0);
			g_LowPowerMode=0;
			}
		}

	LogPrintf("cnt:%d current: %d mode %d  hold %d\r\n",g_LowPowerCounter,g_bms1.current,g_LowPowerMode,g_LowPowerHold);
	
    if((g_LowPowerCounter>=60||g_LowPowerMode)&&g_LowPowerHold==FALSE)
	{
		switch(g_superpowercount2)
		{
			case 0:
				//if(PaygGetPayState())
					sendCommand(COMMOND_HEARTBEAT);
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;
			case 1:
				if(PaygGetPayState())
		        {
		            sendCommand(COMMOND_FEEDDOG);
		        }
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;
			case 2:
				if(PaygGetPayState())
		        {
		            sendCommand(COMMOND_VOLTAGE);
		        }
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;
			case 3:
				if(PaygGetPayState())
		        {
		            sendCommand(COMMOND_BATTERY);
		        }
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;		
			case 4:
				if(PaygGetPayState())
		        {
		            sendCommand(COMMOND_CURRENT);
		        }
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;
			case 5:	
				sendCommand(COMMOND_HEARTBEAT);
				g_superpowercount2++;
				g_superpowercount2%=7;
				break;		
			case 6:	
				superPower_timeout_timer_start();
				g_superpowercount2++;
				g_superpowercount2%=7;

		   		rtc_configuration(53);//for 1 min
				
				bkp_data_write(BKU_COUNTER_REG,g_LowPowerCounter);
				bkp_data_write(BKU_MODE_REG,1);
				bkp_data_write(BKU_PORST_REG,1);

				if(g_LowPowerSleepTime)
				{    g_LowPowerSleepTime--;
					 bkp_data_write(BKU_HOLD_REG,FALSE);
					}
				else
				{
					bkp_data_write(BKU_HOLD_REG,TRUE);
					}
				
				bkp_data_write(BKU_SLEEPTIME_REG,g_LowPowerSleepTime);
				
				pmu_to_standbymode();
				break;
			
			default:
				g_superpowercount2=0;
				break;
			}

		//printUartInfo(g_UartSuperPowerBuf, BUP_data_uart_rx_size());
        GetallsuperpowerUartInfo(&g_bms1);  //使用时要改GetUartInfo()关闭disconnect
        superPowerPublish(&g_bms1);
        printSuperPowerBmsInfo(&g_bms1);
		return ;
		}

     #endif
    if(g_superpowercount2==5)
    {
        sendCommand(COMMOND_HEARTBEAT);
        g_superpowercount2++;
        g_superpowercount2%=60;
        return;
    }
    if(g_superpowercount2==6)
    {
    	extern uint8_t g_TokenValidInput;
		
    	if(g_TokenValidInput)
    	{
	        if(PaygGetFreeState())
	        {
	            sendCommand(COMMOND_CLOSE_WATCHDOG);
	        }
	        else
	        {
	            sendCommand(COMMOND_OPEN_WATCHDOG);
	        }
    	}
		g_TokenValidInput=FALSE;
        g_superpowercount2++;
        g_superpowercount2%=60;
        return;
    }
    if(g_superpowercount2==7)
    {
        if(PaygGetPayState())
        {
            sendCommand(COMMOND_FEEDDOG);
        }
        
        g_superpowercount2++;
        g_superpowercount2%=60;
        return;
    }
    if(g_superpowercount2==8)
    {
        if(PaygGetPayRemainDays()<=3&&PaygGetPayState())
        {
            sendCommand(COMMOND_ALARM);
        }
        g_superpowercount2++;
        g_superpowercount2%=60;
        return;
    }
    else
    {
        g_superpowercount2++;
        g_superpowercount2%=60;
        //return;
    }


    if(g_superpowercount%4==0)
    {
        sendCommand(COMMOND_VOLTAGE);
        g_superpowercount++;
    }
    else if(g_superpowercount%4==1)
    {
        sendCommand(COMMOND_CURRENT);
        g_superpowercount++;
        g_superpowercount%=4;
    }
    else if(g_superpowercount==2)
    {
        sendCommand(COMMOND_BATTERY);
        g_superpowercount++;
        g_superpowercount%=4;
    }
    else if(g_superpowercount==3)
    {
        //printUartInfo(g_UartSuperPowerBuf, BUP_data_uart_rx_size());
        GetallsuperpowerUartInfo(&g_bms1);  //使用时要改GetUartInfo()关闭disconnect
        superPowerPublish(&g_bms1);
        printSuperPowerBmsInfo(&g_bms1);
        g_superpowercount++;
        g_superpowercount%=4;
    }
    else
    {
        LOG("error g_superpowercount");
        g_superpowercount++;
        g_superpowercount%=4;
    }
    
}




#endif /* SUPERPOWER_PROJECT */

