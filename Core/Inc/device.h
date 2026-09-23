#ifndef __device_H__
#define __device_H__



// 定义设备类型的枚举
typedef enum {
    DEVICE_TYPE_BMS,
    DEVICE_TYPE_CHARGE_CONTROLLER,
    // 添加其他设备类型...
} DeviceType;


// 定义设备状态枚举
typedef enum {
    STATUS_NORMAL = 0x01,
    STATUS_SLEEP,
    STATUS_WAITING,
    STATUS_PROTECT,
    STATUS_ERROR,
} Status;


// 定义设备初始化函数指针类型
typedef void (*DeviceInitializer)(void*,uint8_t device_id);


//定义销毁函数指针类型
typedef void (*DeviceDestroyer)(void*);


/*定义设备的结构体*/
typedef struct {
    uint8_t device_id;          // 设备ID
    uint8_t status;             // 设备系统状态
    DeviceType device_type;            // 设备类型
    uint16_t error_counter_rx;  // 接收错误计数器
    uint16_t error_counter_tx;  // 发送错误计数器
    // void (*initialize)(void* device); // 设备初始化函数指针
    DeviceInitializer initialize; // 设备初始化函数指针
    DeviceDestroyer destroy;    // 设备销毁函数指针

} Device;


// Bms结构体
typedef struct {
    Device device;
    uint16_t total_voltage;           
    uint32_t u32_total_voltage;       
    int16_t current;                 
    uint16_t u16_current;             
    uint16_t remaining_capacity;      
    uint8_t charging_flag;            
    uint16_t full_capacity;           
    uint16_t discharge_cycles;        
    uint16_t rsoc;                    
    uint16_t acyc;
    uint16_t batp;
    uint16_t inpp;
    uint16_t outp;
    uint16_t aeng;
    uint16_t cmos;  
    uint16_t dmos;
    uint16_t cell_balance_status_1;    
    uint16_t cell_balance_status_2;    
    uint16_t protection_flags;         
    uint16_t mos_status;              
    uint16_t production_date;         
    uint16_t software_version;        
    uint8_t num_battery_strings;     
    uint8_t num_ntc_probes;           
    uint16_t ntc_temperatures[10];        
    uint16_t cell_voltages[30];
    int8_t highest_temperature;
    uint16_t ppst;
    uint16_t pckt;
} Bms;


// CanChargeController
typedef struct {
    Device device;
    uint16_t output_voltage;//100mV
    uint16_t output_current;//100mA
    uint8_t CanChargeController_status;
} ChargeController;





typedef struct {
    uint16_t maxVoltage;    // 最高允许充电端电压
    uint16_t maxCurrent;    // 最高允许充电电流
    uint8_t chargerControl;    // 充电机控制
    uint8_t stopReason;     // BMS 发送停止命令原因
    uint16_t highestCellVoltage;    // 单体最高电压
    uint16_t lowestCellVoltage;     // 单体最低电压
    uint8_t stateOfCharge;          // SOC
    int8_t highestTemperature;      // 最高温度
    uint16_t batteryVoltage;        // 电池组电压
} ChargingProcess;


typedef struct LinkedList {
    Device* head;
} LinkedList;






uint16_t extract_uint16(uint8_t* data, uint8_t index);
int16_t extract_int16(uint8_t* data, uint8_t index);
uint8_t extract_uint8(uint8_t* data, uint8_t index); 
uint8_t get_high_byte(uint16_t value); 
uint8_t get_low_byte(uint16_t value);


// Bms设备的初始化函数
void initializeBms(void* device,uint8_t device_id); 
// ChargeController设备的初始化函数
void initializeChargeController(void* device,uint8_t device_id);
// Bms设备的销毁函数
void destroyBms(void* device);
// ChargeController设备的销毁函数
void destroyChargeController(void* device);
// 通用结构体序列化函数
void serializeStruct(const void* struct_ptr, uint8_t* buffer, size_t struct_size, size_t buffer_size);
// 通用结构体反序列化函数
void deserializeStruct(void* struct_ptr, const uint8_t* buffer, size_t struct_size); 



#endif /* __device_H__ */

