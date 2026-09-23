#include"main.h"

uint8_t g_EmeterComEnable=0;
uint8_t g_EmeterReadMode=EMETER_READ_POWER;
uint8_t g_EmeterComErrorCnt=0;

EMETER_INFOR_TypeDef EmeterInfor;
extern __IO uint8_t g_Uart485Buf[UART3_RX_BUF_SIZE];
extern UART_HandleTypeDef huart3;//485	DI

uint32_t   set_total_power=0;


uint32_t get_set_power(void)
{
	return set_total_power;
}
uint32_t MbUint8ToUint32(uint8_t *buffer)
{	
	uint32_t temp;

	temp=buffer[0]<<24;
	temp|=buffer[1]<<16;
	temp|=buffer[2]<<8;
	temp|=buffer[3];


	return temp;
}

uint32_t MbUint32ToUint32(uint8_t *buffer)
{	
	uint32_t temp;

	temp=buffer[1]<<24;
	temp|=buffer[0]<<16;
	temp|=buffer[3]<<8;
	temp|=buffer[2];

	return temp;
}


float bytes_to_float_big_endian( uint8_t bytes[4]) {
    union {
        uint32_t u32;
        float    f;
    } converter;
    converter.u32 = ((uint32_t)bytes[0] << 24) |
                    ((uint32_t)bytes[1] << 16) |
                    ((uint32_t)bytes[2] << 8)  |
                    ((uint32_t)bytes[3]);
    return converter.f;   // 直接使用联合体（大多数编译器支持）
}

float bytes_to_float_little_endian( uint8_t *bytes) {
    uint32_t as_int = ((uint32_t)bytes[3] << 24) |
                      ((uint32_t)bytes[2] << 16) |
                      ((uint32_t)bytes[1] << 8)  |
                      ((uint32_t)bytes[0]);
    float result;
    memcpy(&result, &as_int, sizeof(result));
    return result;
}


uint32_t Uint32Uint8ToMb(uint8_t *buffer)
{	
	uint32_t temp;

	temp=buffer[3]<<16;
	temp|=buffer[2]<<24;
	temp|=buffer[0]<<8;
	temp|=buffer[1];

	return temp;
}

uint16_t Uint16Uint8ToMb(uint8_t *buffer)
{	
	uint16_t temp=0;
	temp|=buffer[0]<<8;
	temp|=buffer[1];

	return temp;
}

void  FloatToMbUint8(float src,uint8_t *buffer)
{	
	uint16_t *p=(uint16_t*)&src;
	uint16_t temp=*p;

	buffer[0]=temp>>8;
	buffer[1]=temp;
	p++;

	temp=*p;
	buffer[2]=temp>>8;
	buffer[3]=temp;
}

float MbUint8ToFloat(uint8_t *buffer)
{	
	float *p;
	uint8_t temp[4];

	temp[0]=buffer[1];
	temp[1]=buffer[0];
	temp[2]=buffer[3];
	temp[3]=buffer[2];

	p=(float*)&temp;

	return *p;
}


void ModbusWrite(uint8_t addr,uint8_t cmd,uint16_t reg,uint16_t Data)
{
	MODBUS_WRITE_TypeDef temp;

	temp.bus_addr=addr;
	temp.func=cmd;
	temp.regH=reg>>8;
	temp.regL=reg;
	temp.dataH=Data>>8;
	temp.dataL=Data;
	temp.crc=CRC16(&temp.bus_addr, sizeof(MODBUS_WRITE_TypeDef)-2);
	Uart3Send(&temp.bus_addr,sizeof(MODBUS_READ_TypeDef));
	
//	printf("  TFT tx tx_size = %d\r\n",sizeof(MODBUS_READ_TypeDef));	
//	my_printf_hex(&temp.bus_addr,sizeof(MODBUS_READ_TypeDef));
}

void ModbusMultipWrite(uint8_t addr,uint8_t cmd,uint16_t reg,uint8_t *Data,uint8_t regnum)
{
	MODBUS_WRITE_MULTI_TypeDef temp;
	uint16_t crc;

	temp.bus_addr=addr;
	temp.cmd=cmd;
	temp.regH=reg>>8;
	temp.regL=reg;
	temp.numH=regnum>>8;
    temp.numL=regnum;

	temp.datalen=regnum*2;
	
	memcpy(temp.data,Data,temp.datalen);

	crc=CRC16(&temp.bus_addr, 7+temp.datalen);

	temp.data[regnum*2]=crc;
	temp.data[regnum*2+1]=crc>>8;


	Uart3Send(&temp.bus_addr, 7+temp.datalen+2);
	// printf("  TFT MUT tx tx_size = %d\r\n",7+temp.datalen+2);	  	
	// my_printf_hex(&temp.bus_addr, (7+temp.datalen+2));
}


void ModbusRead(uint8_t addr,uint8_t cmd,uint16_t reg,uint8_t size)
{
	MODBUS_READ_TypeDef temp;

	temp.bus_addr=addr;
	temp.func=cmd;
	temp.regH=reg>>8;
	temp.regL=reg;
	temp.numH=size>>8;
    temp.numL=size;

	temp.crc=CRC16(&temp.bus_addr, sizeof(MODBUS_READ_TypeDef)-2);
	Uart3Send(&temp.bus_addr,sizeof(MODBUS_READ_TypeDef));
	
	//printf("  485 tx tx_size = %d\r\n",sizeof(MODBUS_READ_TypeDef));	
	//my_printf_hex(&temp.bus_addr,sizeof(MODBUS_READ_TypeDef));
	
	HAL_Delay(10);
}




void EmeterInit(void)
{
	g_EmeterComEnable=0;
	
}

void EmeterReadModeSet(uint8_t read)
{
	g_EmeterReadMode=read;
	
}

uint8_t EmeterReadModeGet(void)
{
	return g_EmeterReadMode;
}


void EmeterReadEnable(void)
{
	g_EmeterComEnable=TRUE;
}


#if 0

void EmeterProc(void)
{
	uint16_t rx_size,j=0,crc=0;
	uint8_t valid;
	uint8_t size=0;
	static uint32_t temp_energy_a =0 ,temp_power_a= 0,temp_energy_b =0 ,temp_power_b= 0,temp_energy_c=0 ,temp_power_c= 0;
	float f = 0.0;
	static uint8_t send_count =0;
	 
	//	g_EmeterComEnable =1;
	//HAL_Delay(100);
	uint16_t METERADDR_PHASE_A_VOLT_3=0x012c,
//	METERADDR_TOTAL_POWER =0x0136,
	
	if(g_EmeterComEnable)   
	{	
		  g_EmeterComEnable=FALSE;
		
		
		if(getModbusState()==MODBUS_EMETER)
		{
			send_count++;
		
		 if(send_count == 15)
		 { memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
		   huart3.RxXferCount=0;
			ModbusRead(0x05,0x04,METERADDR_PHASE_A_VOLT_3,0x0e);
			
		 }
		else if(send_count == 30)
		{memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
		   huart3.RxXferCount=0;
			ModbusRead(0x06,0x04,METERADDR_PHASE_A_VOLT_3,0x0e); 
			
		}
		 else if(send_count == 45)
		 {
			 send_count = 0;
			 memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
		   huart3.RxXferCount=0;
			 ModbusRead(0x07,0x04,METERADDR_PHASE_A_VOLT_3,0x0e);
			 
		 }
		 
	  	g_EmeterComErrorCnt++;

	
		if(g_EmeterComErrorCnt>50)
		{
			g_EmeterReadMode=EMETER_READ_IDLE;//release bus
			g_EmeterComErrorCnt=0;
			setModbusState(MODBUS_PLC);
			printf("EmeterInfor  485 SEND OVER ...................\r\n");
		}
	 
//HAL_Delay(10);
//0110000800020440000000E7C9
	//	memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
		//huart3.RxXferCount=0;
	}
		
	}

	valid=FALSE;
	rx_size=huart3.RxXferCount;


	
	//g_EmeterReadMode=EMETER_READ_ENERGY;
	
	//for(j=0;j<UART485_RX_BUF_SIZE/2;j++)
	//if(huart3.rcv_flag ==1)
	{
		huart3.rcv_flag = 0;
		size = EMETER_ADDR_COUNT*2;  //28  g_Uart485Buf  UART3_RX_BUF_SIZE

		for(j=0;j<128;j++)  
		{
			if((g_Uart485Buf[j]==5) || (g_Uart485Buf[j]==6) || (g_Uart485Buf[j]==7))
			{
				if((g_Uart485Buf[j+1]==0x04) && (rx_size>=size) && (g_Uart485Buf[j+2]==size))
				{	
					valid=TRUE;
					 
					break;
				}
			}
	   }	
	}

	if(valid) 
	{
//		HAL_Delay(10);
//		for(j=0;j<(size+5);j++)  
//		{
//			rs485_data[j] = g_Uart485Buf[j];
//		}
//		
//			if(rx_size > 0)
//			{
//				printf("EmeterInfor  485 RX rx_size = %d\r\n",rx_size);
//				my_printf_hex(g_Uart485Buf,rx_size);
//			}

		//HAL_Delay(2);

//		crc=(g_Uart485Buf[g_Uart485Buf[j+2]+3+j]|(g_Uart485Buf[g_Uart485Buf[j+2]+3+1+j]<<8));

//		if(crc==CRC16((uint8_t*)&g_Uart485Buf[j], g_Uart485Buf[j+2]+3)&&g_Uart485Buf[j+1]==0x04)
			
//		crc=(g_Uart485Buf[size+3+j]|(g_Uart485Buf[size+3+1+j]<<8));

// 		if(crc==CRC16((uint8_t*)&g_Uart485Buf[j], size+3) )	
		{
			//g_EmeterComErrorCnt=0; 0K
			
				if(g_Uart485Buf[j]==5)  
				{
					f = bytes_to_float_big_endian(&g_Uart485Buf[j+3]);
					EmeterInfor.phase_a_voltage = f*10;
					//printf("EmeterInfor  485 RX rx_size A xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_voltage);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+7]);
					EmeterInfor.phase_a_current = f*10;
					//printf("EmeterInfor  485 RX rx_size A xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_current);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+11]);
					// EmeterInfor.current_energy = f*10;
					temp_energy_a =  f*10;
					//printf("EmeterInfor  485 RX rx_size A xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.current_energy);


					f = bytes_to_float_big_endian(&g_Uart485Buf[j+23]);
					//EmeterInfor.total_power = f*10;
					temp_power_a =  f*10;
				//	printf("EmeterInfor  485 RX rx_size  A xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.total_power);
				}
				else if(g_Uart485Buf[j]==6)
				{
					f = bytes_to_float_big_endian(&g_Uart485Buf[j+3]);
					EmeterInfor.phase_b_voltage = f*10;
				//	printf("EmeterInfor  485 RX rx_size B xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_voltage);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+7]);
					EmeterInfor.phase_b_current = f*10;
				//	printf("EmeterInfor  485 RX rx_size B xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_current);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+11]);
					// EmeterInfor.current_energy = f*10;
					temp_energy_b = f*10;
				//	printf("EmeterInfor  485 RX rx_size  B xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.current_energy);


					f = bytes_to_float_big_endian(&g_Uart485Buf[j+23]);
					//EmeterInfor.total_power = f*10;
					temp_power_b = f*10;
					//printf("EmeterInfor  485 RX rx_size B xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.total_power);
				}
				else if(g_Uart485Buf[j]==7)
				{
					f = bytes_to_float_big_endian(&g_Uart485Buf[j+3]);
					EmeterInfor.phase_c_voltage = f*10;
					//printf("EmeterInfor  485 RX rx_size C xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_voltage);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+7]);
					EmeterInfor.phase_c_current = f*10;
				//	printf("EmeterInfor  485 RX rx_size C xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.phase_a_current);

					f = bytes_to_float_big_endian(&g_Uart485Buf[j+11]);
					// EmeterInfor.current_energy = f*10;
					temp_energy_c = f*10;
				//	printf("EmeterInfor  485 RX rx_size C xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.current_energy);


					f = bytes_to_float_big_endian(&g_Uart485Buf[j+23]);
					//EmeterInfor.total_power = f*10;
					temp_power_c = f*10;
				//	printf("EmeterInfor  485 RX rx_size C xxxxxxxx = %f     %d  \r\n",f,EmeterInfor.total_power);
				}
				
				EmeterInfor.total_power = temp_power_a+temp_power_b+temp_power_c;
				EmeterInfor.current_energy = temp_energy_a+temp_energy_b+temp_energy_c;
				
				g_EmeterReadMode=EMETER_READ_ENERGY;

			    PlcEmeterInforUpdate(&EmeterInfor);
				
				memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
			huart3.RxXferCount=0;
			huart3.RxXferSize=UART3_RX_BUF_SIZE;
			huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
				
		}
			
	 }
}

#endif

void EmeterProc(void)
{
	uint16_t rx_size,j=0,crc=0;
	uint8_t valid;
	uint8_t size=0;
	static uint8_t send_count =0;
	/*if(g_EmeterComEnable)
	{	
		g_EmeterComEnable=FALSE;
		
		if(g_EmeterReadMode==EMETER_READ_ENERGY)
			ModbusRead(0x01,0x04,METERADDR_CURRENT_ENERGY,2);
		else
			ModbusRead(0x01,0x04,METERADDR_PHASE_A_VOLT,16);

		g_EmeterComErrorCnt++;

		if(g_EmeterComErrorCnt>20)
		{	g_EmeterReadMode=EMETER_READ_IDLE;//release bus
			g_EmeterComErrorCnt=0;
		}	
	}*/
	
	if(g_EmeterComEnable)   
	{	
		  g_EmeterComEnable=FALSE;
		if(getModbusState()==MODBUS_EMETER)
		{
			send_count++;
		
			if(send_count == 1)
			{ 
				 memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
			     huart3.RxXferCount=0;
				 ModbusRead(0x02,0x03,0x26,2); //  读取屏幕设置功率
				 g_EmeterReadMode=EMETER_READ_ENERGY;			 
			}
			else if(send_count == 10)
			{ 
				 memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
			   huart3.RxXferCount=0;
				 ModbusRead(0x01,0x04,METERADDR_CURRENT_ENERGY,2);
				 g_EmeterReadMode=EMETER_READ_ENERGY;			 
			}
			else if(send_count == 20)
			{
				g_EmeterReadMode=EMETER_READ_POWER;
				memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
				huart3.RxXferCount=0;
				ModbusRead(0x01,0x04,METERADDR_PHASE_A_VOLT,16);
				//send_count = 0;
			}
			if(send_count >= 30) send_count = 0;
			 
			g_EmeterComErrorCnt++;

		
			if(g_EmeterComErrorCnt>30)
			{
				g_EmeterReadMode=EMETER_READ_IDLE;//release bus
				g_EmeterComErrorCnt=0;
				setModbusState(MODBUS_PLC);
				printf("EmeterInfor  485 SEND OVER ...................\r\n");
			}
		}
		
	}

	valid=FALSE;
	rx_size=huart3.RxXferCount;

	
	
	
	
	for(j=0;j<128;j++)
	{
		if(g_EmeterReadMode==EMETER_READ_ENERGY)
		{	
			size=4;
		}
		else
		{	
			size=16*2;
		}
		
		
		if(g_Uart485Buf[j]==2&&g_Uart485Buf[j+1]==0x03&&g_Uart485Buf[j+2]==0x04&&rx_size==9)
		{	//valid=TRUE;
			
			set_total_power = (g_Uart485Buf[j+5]<<24)|(g_Uart485Buf[j+6]<<16)|(g_Uart485Buf[j+3]<<8)|(g_Uart485Buf[j+4]);
			
			memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
			huart3.RxXferCount=0;
			huart3.RxXferSize=UART485_RX_BUF_SIZE;
    		huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
			break;
		}
		
		if(g_Uart485Buf[j]>=1&&g_Uart485Buf[j+1]>=0x04&&rx_size>=size+3&&g_Uart485Buf[j+2]==size)
		{	
			valid=TRUE;
			break;
		}
	}

	if(valid)
	{
		HAL_Delay(2);

		crc=(g_Uart485Buf[g_Uart485Buf[j+2]+3+j]|(g_Uart485Buf[g_Uart485Buf[j+2]+3+1+j]<<8));

//		 	if(rx_size > 0)
//			{
//				printf("EmeterInfor  485 RX rx_size = %d\r\n",rx_size);
//				my_printf_hex(g_Uart485Buf,rx_size);
//			}
		
		if(crc==CRC16((uint8_t*)&g_Uart485Buf[j], g_Uart485Buf[j+2]+3)&&g_Uart485Buf[j+1]==0x04)
		{

			//g_EmeterComErrorCnt=0;
			
			if(g_EmeterReadMode==EMETER_READ_POWER)
			{
				memcpy((uint8_t*)&EmeterInfor,(uint8_t*)&g_Uart485Buf[j+3],sizeof(EmeterInfor)-4);
				EmeterInfor.phase_a_voltage=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_a_voltage);
				EmeterInfor.phase_b_voltage=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_b_voltage);	
				EmeterInfor.phase_c_voltage=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_c_voltage);	
				EmeterInfor.phase_a_current=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_a_current);	
				EmeterInfor.phase_b_current=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_b_current);	
				EmeterInfor.phase_c_current=MbUint8ToUint32((uint8_t*)&EmeterInfor.phase_c_current);	
				EmeterInfor.total_power=MbUint8ToUint32((uint8_t*)&EmeterInfor.total_power);	
				//g_EmeterReadMode=EMETER_READ_ENERGY;
			}
			else if(g_EmeterReadMode==EMETER_READ_ENERGY)
			{
				EmeterInfor.current_energy=MbUint8ToUint32((uint8_t*)&g_Uart485Buf[j+3]);	
				//g_EmeterReadMode=EMETER_READ_IDLE;
			}
			//printf("EmeterInfor  485 RX rx_size A xxxxxxxx = %d     %d  \r\n",EmeterInfor.phase_a_voltage,EmeterInfor.phase_a_current);
			//	printf("EmeterInfor  485 RX rx_size A xxxxtotal_power = %d  current_energy %d  \r\n",EmeterInfor.total_power,EmeterInfor.current_energy);
			
			
			PlcEmeterInforUpdate(&EmeterInfor);

			memset((uint8_t*)g_Uart485Buf,0x00,UART3_RX_BUF_SIZE);
			huart3.RxXferCount=0;
			huart3.RxXferSize=UART485_RX_BUF_SIZE;
    		huart3.pRxBuffPtr=(uint8_t*)g_Uart485Buf; 
		}

	 }

}


