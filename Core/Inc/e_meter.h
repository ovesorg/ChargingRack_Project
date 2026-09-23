
#ifndef __E_METER_H__

#define __E_METER_H__

#define EMETER_ADDR_COUNT 14

enum
{
	METERADDR_CURRENT_ENERGY=0x001d,
	METERADDR_PHASE_A_VOLT=0x0100,
	METERADDR_PHASE_B_VOLT=0x0102,
	METERADDR_PHASE_C_VOLT=0x0104,
	METERADDR_PHASE_A_CURRENT=0x0106,
	METERADDR_PHASE_B_CURRENT=0x0108,
	METERADDR_PHASE_C_CURRENT=0x010A,

	METERADDR_TOTAL_POWER=0x010E,
	
//	METERADDR_PHASE_A_VOLT=0x012c,
//	METERADDR_TOTAL_POWER =0x0136,

};

enum
{
	EMETER_READ_IDLE,
	EMETER_READ_POWER,
	EMETER_READ_ENERGY	
};

/*typedef struct
{
	uint8_t bus_addr;
	uint8_t cmd;
	uint8_t regH;
	uint8_t regL;
	
	uint8_t numH;
	uint8_t numL;
	uint16_t crc;
}MODBUS_READ_TypeDef;*/

typedef struct
{
	uint8_t bus_addr;
	uint8_t func;
	uint8_t regH;
	uint8_t regL;
	
	uint8_t dataH;
	uint8_t dataL;

	uint16_t crc;
}MODBUS_WRITE_TypeDef;

typedef struct
{
	uint8_t bus_addr;
	uint8_t cmd;
	uint8_t regH;
	uint8_t regL;
	
	uint8_t numH;
	uint8_t numL;
	uint8_t datalen;
	
	uint8_t data[65+128];
}MODBUS_WRITE_MULTI_TypeDef;


typedef struct
{
	uint32_t phase_a_voltage;// 0.1v
	uint32_t phase_b_voltage;
	uint32_t phase_c_voltage;
	uint32_t phase_a_current;//0.001A
	uint32_t phase_b_current;
	uint32_t phase_c_current;
	uint32_t reserved;
	uint32_t total_power;  //0.1W
	uint32_t current_energy;
}EMETER_INFOR_TypeDef;


uint32_t MbUint8ToUint32(uint8_t *buffer);
void  FloatToMbUint8(float src,uint8_t *buffer);
float MbUint8ToFloat(uint8_t *buffer);
uint16_t Uint16Uint8ToMb(uint8_t *buffer);

void ModbusMultipWrite(uint8_t addr,uint8_t cmd,uint16_t reg,uint8_t *Data,uint8_t regnum);
void ModbusWrite(uint8_t addr,uint8_t cmd,uint16_t reg,uint16_t Data);
void ModbusRead(uint8_t addr,uint8_t cmd,uint16_t reg,uint8_t size);
void EmeterReadEnable(void);
void EmeterReadModeSet(uint8_t read);
uint8_t EmeterReadModeGet(void);
uint32_t Uint32Uint8ToMb(uint8_t *buffer);
uint32_t MbUint32ToUint32(uint8_t *buffer);

void EmeterInit(void);
void EmeterProc(void);
#endif


