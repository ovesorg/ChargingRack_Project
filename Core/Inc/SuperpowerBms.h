#ifndef __SUPERPOWER_H__
#define __SUPERPOWER_H__


enum 
{
    COMMOND_VOLTAGE=0,
    COMMOND_CURRENT,
    COMMOND_BATTERY,
    COMMOND_CODE,
    COMMOND_DISCHARGE_ALLOW,
    COMMOND_DISCHARGE_DENY,
    COMMOND_CHARGE_ALLOW,
    COMMOND_CHARGE_DENY,
    COMMOND_HEARTBEAT,
    COMMOND_CLOSE_WATCHDOG,
    COMMOND_OPEN_WATCHDOG,
    COMMOND_ALARM,
    COMMOND_FEEDDOG,
    COMMOND_COUNT
};
	
#define BKU_COUNTER_REG BKP_DATA_0
#define BKU_MODE_REG BKP_DATA_1
#define BKU_HOLD_REG BKP_DATA_2
#define BKU_SLEEPTIME_REG BKP_DATA_3
#define BKU_PORST_REG BKP_DATA_4


#define LOG(...)

uint8_t* getCommand(uint8_t index);
void superPower_timeout_timer_start(void);
void superPoweruartSend(uint8_t *buffer,uint16_t size);
void sendCommand(uint8_t index);
void superPowerGetEnable(void);
void GetUartInfo(void* p_bms,uint8_t* data);
void GetallsuperpowerUartInfo(void* p_dev);
void publish_battery_data(void* p_bms);
void superPowerPublish(void* p_dev);
void printUartInfo(uint8_t* Buffer, uint16_t size);
void printSuperPowerBmsInfo(void* p_bms);
void superPowerInit(void);
void superPowerTask(void);
#ifdef LOWPOWER_SUPPORT
void LowPowerHold(uint8_t hold);
void superPowerBkpInit(void);
uint8_t superPowerLowpowerMode(void);

#endif
#endif /* __SUPERPOWER_H__ */

