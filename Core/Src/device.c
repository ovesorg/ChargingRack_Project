#include "main.h"


#ifdef SUPERPOWER_PROJECT


// Bms* p_bms = NULL;
// ChargeController* p_chargeController = NULL;


// 定义设备初始化函数指针数组，用于存放不同类型设备的初始化函数
DeviceInitializer deviceInitializers[] = {
    initializeBms,              // 初始化 Bms 设备的函数指针
    initializeChargeController, // 初始化 ChargeController 设备的函数指针
    // 添加其他设备类型的初始化函数指针...
};

DeviceDestroyer deviceDestroyers[] = {
    destroyBms,              // 销毁 Bms 设备的函数指针
    destroyChargeController, // 销毁 ChargeController 设备的函数指针
    // 添加其他设备类型的销毁函数指针...
};



uint16_t extract_uint16(uint8_t* data, uint8_t index) 
{
    return (uint16_t)((data[index] << 8) | data[index + 1]);
}

int16_t extract_int16(uint8_t* data, uint8_t index) 
{
    return (int16_t)((data[index] << 8) | data[index + 1]);
}

uint8_t extract_uint8(uint8_t* data, uint8_t index) 
{
    return data[index];
}

uint8_t get_high_byte(uint16_t value) 
{
    return (value >> 8) & 0xFF;
}

uint8_t get_low_byte(uint16_t value) 
{
    return value & 0xFF;
}





// Bms设备的初始化函数
void initializeBms(void* device,uint8_t device_id) 
{
    Bms* bms = (Bms*)device;

    if (device == NULL) 
    {
        LOG("pointer is null. Initialization failed. %s-%u\n", __FUNCTION__, __LINE__);
        return;
    }
    // 使用 memset 函数将整个结构体的值设置为零
    memset(bms, 0, sizeof(Bms));
    // 初始化 CanBms 的属性...
    bms->device.device_id = device_id; 
    bms->device.device_type = DEVICE_TYPE_BMS; // 设备类型赋值

    // 其他初始化操作...
}

// ChargeController设备的初始化函数
void initializeChargeController(void* device,uint8_t device_id) 
{
    ChargeController* chargeController = (ChargeController*)device;

    if (device == NULL) 
    {
        LOG("pointer is null. Initialization failed. %s-%u\n", __FUNCTION__, __LINE__);
        return;
    }
    memset(chargeController, 0, sizeof(ChargeController));
    // 初始化 ChargeController 的属性...
    chargeController->device.device_id = device_id; 
    chargeController->device.device_type = DEVICE_TYPE_CHARGE_CONTROLLER; // 设备类型赋值
    // 其他初始化操作...
}

// Bms设备的销毁函数
void destroyBms(void* device) 
{
    Bms* bms = (Bms*)device;
    // 执行 CanBms 设备的销毁操作...
    memset(bms, 0, sizeof(Bms));
    // 释放 Bms 设备动态分配的内存

}

// ChargeController设备的销毁函数
void destroyChargeController(void* device) 
{
    ChargeController* chargeController = (ChargeController*)device;
    // 执行 ChargeController 设备的销毁操作...
    memset(chargeController, 0, sizeof(ChargeController));

}

// 通用结构体反序列化函数
void deserializeStruct(void* struct_ptr, const uint8_t* buffer, size_t struct_size) 
{
    size_t offset = 0;
    size_t bytes_to_copy = struct_size - sizeof(((Bms*)0)->device);

    // 跳过device成员并将其他成员从buffer中复制回结构体
    memcpy(((uint8_t*)struct_ptr) + sizeof(((Bms*)0)->device), buffer, bytes_to_copy);
}

// 通用结构体序列化函数
void serializeStruct(const void* struct_ptr, uint8_t* buffer, size_t struct_size, size_t buffer_size) 
{
    size_t offset = 0;
    size_t bytes_to_copy = struct_size - sizeof(((Bms*)0)->device);

    if (buffer_size < bytes_to_copy) {
        LOG("Error: Buffer too small to hold the serialized data.\n");
        return;
    }

    // 跳过device成员并将其他成员复制到buffer中
    memcpy(buffer, ((const uint8_t*)struct_ptr) + sizeof(((Bms*)0)->device), bytes_to_copy);
}

#endif // SUPERPOWER_PROJECT




