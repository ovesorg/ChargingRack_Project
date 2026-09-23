#include"main.h"


OTA_COM_TypeDef g_OtaInfor;
OTA_BIN_AUTH_TypeDef g_OtaAuth;

extern USER_SET_TypeDef g_UserSet;

const uint32_t Crc32Table[256]=
{
	0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
	0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD,
	0x4C11DB70,0x48D0C6C7,0x4593E01E,0x4152FDA9,0x5F15ADAC,0x5BD4B01B,0x569796C2,0x52568B75,
	0x6A1936C8,0x6ED82B7F,0x639B0DA6,0x675A1011,0x791D4014,0x7DDC5DA3,0x709F7B7A,0x745E66CD,
	0x9823B6E0,0x9CE2AB57,0x91A18D8E,0x95609039,0x8B27C03C,0x8FE6DD8B,0x82A5FB52,0x8664E6E5,
	0xBE2B5B58,0xBAEA46EF,0xB7A96036,0xB3687D81,0xAD2F2D84,0xA9EE3033,0xA4AD16EA,0xA06C0B5D,
	0xD4326D90,0xD0F37027,0xDDB056FE,0xD9714B49,0xC7361B4C,0xC3F706FB,0xCEB42022,0xCA753D95,
	0xF23A8028,0xF6FB9D9F,0xFBB8BB46,0xFF79A6F1,0xE13EF6F4,0xE5FFEB43,0xE8BCCD9A,0xEC7DD02D,
	0x34867077,0x30476DC0,0x3D044B19,0x39C556AE,0x278206AB,0x23431B1C,0x2E003DC5,0x2AC12072,
	0x128E9DCF,0x164F8078,0x1B0CA6A1,0x1FCDBB16,0x018AEB13,0x054BF6A4,0x0808D07D,0x0CC9CDCA,
	0x7897AB07,0x7C56B6B0,0x71159069,0x75D48DDE,0x6B93DDDB,0x6F52C06C,0x6211E6B5,0x66D0FB02,
	0x5E9F46BF,0x5A5E5B08,0x571D7DD1,0x53DC6066,0x4D9B3063,0x495A2DD4,0x44190B0D,0x40D816BA,
	0xACA5C697,0xA864DB20,0xA527FDF9,0xA1E6E04E,0xBFA1B04B,0xBB60ADFC,0xB6238B25,0xB2E29692,
	0x8AAD2B2F,0x8E6C3698,0x832F1041,0x87EE0DF6,0x99A95DF3,0x9D684044,0x902B669D,0x94EA7B2A,
	0xE0B41DE7,0xE4750050,0xE9362689,0xEDF73B3E,0xF3B06B3B,0xF771768C,0xFA325055,0xFEF34DE2,
	0xC6BCF05F,0xC27DEDE8,0xCF3ECB31,0xCBFFD686,0xD5B88683,0xD1799B34,0xDC3ABDED,0xD8FBA05A,
	0x690CE0EE,0x6DCDFD59,0x608EDB80,0x644FC637,0x7A089632,0x7EC98B85,0x738AAD5C,0x774BB0EB,
	0x4F040D56,0x4BC510E1,0x46863638,0x42472B8F,0x5C007B8A,0x58C1663D,0x558240E4,0x51435D53,
	0x251D3B9E,0x21DC2629,0x2C9F00F0,0x285E1D47,0x36194D42,0x32D850F5,0x3F9B762C,0x3B5A6B9B,
	0x0315D626,0x07D4CB91,0x0A97ED48,0x0E56F0FF,0x1011A0FA,0x14D0BD4D,0x19939B94,0x1D528623,
	0xF12F560E,0xF5EE4BB9,0xF8AD6D60,0xFC6C70D7,0xE22B20D2,0xE6EA3D65,0xEBA91BBC,0xEF68060B,
	0xD727BBB6,0xD3E6A601,0xDEA580D8,0xDA649D6F,0xC423CD6A,0xC0E2D0DD,0xCDA1F604,0xC960EBB3,
	0xBD3E8D7E,0xB9FF90C9,0xB4BCB610,0xB07DABA7,0xAE3AFBA2,0xAAFBE615,0xA7B8C0CC,0xA379DD7B,
	0x9B3660C6,0x9FF77D71,0x92B45BA8,0x9675461F,0x8832161A,0x8CF30BAD,0x81B02D74,0x857130C3,
	0x5D8A9099,0x594B8D2E,0x5408ABF7,0x50C9B640,0x4E8EE645,0x4A4FFBF2,0x470CDD2B,0x43CDC09C,
	0x7B827D21,0x7F436096,0x7200464F,0x76C15BF8,0x68860BFD,0x6C47164A,0x61043093,0x65C52D24,
	0x119B4BE9,0x155A565E,0x18197087,0x1CD86D30,0x029F3D35,0x065E2082,0x0B1D065B,0x0FDC1BEC,
	0x3793A651,0x3352BBE6,0x3E119D3F,0x3AD08088,0x2497D08D,0x2056CD3A,0x2D15EBE3,0x29D4F654,
	0xC5A92679,0xC1683BCE,0xCC2B1D17,0xC8EA00A0,0xD6AD50A5,0xD26C4D12,0xDF2F6BCB,0xDBEE767C,
	0xE3A1CBC1,0xE760D676,0xEA23F0AF,0xEEE2ED18,0xF0A5BD1D,0xF464A0AA,0xF9278673,0xFDE69BC4,
	0x89B8FD09,0x8D79E0BE,0x803AC667,0x84FBDBD0,0x9ABC8BD5,0x9E7D9662,0x933EB0BB,0x97FFAD0C,
	0xAFB010B1,0xAB710D06,0xA6322BDF,0xA2F33668,0xBCB4666D,0xB8757BDA,0xB5365D03,0xB1F740B4
};
//查表法
uint32_t CRC32(uint8_t *pData,uint32_t Length)
{
	uint32_t nReg;//CRC寄存器
	uint32_t nTemp=0;
	uint32_t i, n;
	nReg = 0xFFFFFFFF;//
	for(n=0; n<Length; n++)
	{
		nReg ^= (uint32_t)pData[n];
		for(i=0; i<4; i++)
		{
			nTemp = Crc32Table[(uint8_t)(( nReg >> 24 ) & 0xff)]; //取一个字节，查表
			nReg <<= 8; //丢掉计算过的头一个BYTE
			nReg ^= nTemp; //与前一个BYTE的计算结果异或
		}
	}
	return nReg;
}


const uint8_t g_OvesVcu103Mark[] __attribute__((at(0x8008000)))="OVES-VCU-103\0";

#define  VCU_SPEC_ADDR 0x8008000

const uint8_t g_OvesCmpVcu103Mark[]="OVES-VCU-103\0";


uint8_t UpgradeBinCheck(void)
{
    uint8_t *pmark=(__IO uint8_t*)(VCU_SPEC_ADDR);

	if(strstr(g_OvesCmpVcu103Mark,pmark)!=NULL)
		return TRUE;
	else 
		return FALSE;
}


void OtaInit(void)
{
	memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));

	g_OtaInfor.sleep=g_UserSet.sleeptime; 
	g_OtaInfor.online=g_UserSet.onlinetime;

	g_OtaInfor.state=OTA_STATE_IDLE;
	
}


void OtaAckPublishPayload(uint8_t *buf)
{
	uint8_t*json=GattGetJsonBuff();
	
	memset(json,0x00,JSON_LEN);

	memcpy(json,"{\"cmd\":{",8);
	json+=strlen((char*)json);

	if(strlen((char*)buf)<JSON_LEN-10)
		memcpy(json,buf,strlen((char*)buf));

	json+=strlen((char*)json);

	memcpy(json,"}}",2);
	/*
	
	memcpy(json,"{\"ota\":\"ready\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d,\"size\":%d",g_OtaInfor.time,g_OtaInfor.filesize);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"ack\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d,\"addr\":%d",g_OtaInfor.time,g_OtaInfor.addr);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"boot\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"time\":%d",g_OtaInfor.time);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"succ\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"ver\":%s",g_OtaInfor.time);
	json+=strlen((char*)json);
	memcpy(json,"}}",2);

	memcpy(json,"{\"ota\":\"err\",",8);
	json+=strlen((char*)json);
	spirntf(json,"\"code\":%s","crc");
	json+=strlen((char*)json);
	memcpy(json,"}}",2);*/

}

void OtaParse(uint8_t * buf)
{
	uint32_t size,time,crc;
	uint32_t addr,len;
	uint8_t i,ver[8]={0},key[16]={0};
	uint8_t DecodePack[1024]={0};
	uint8_t tempBuff[128]={0};
	uint32_t Flash_data=0;
	uint8_t EncodePack[1024]={0};
	uint8_t *pTemp=NULL;
	
	if(buf!=NULL)
	{
		uint8_t *p=NULL;
		p=strstr(buf,"ota\":");

		if(p!=NULL)
		{
			TimerSet(TIMER_OTA_EXIT,300000U*10);

			g_UserSet.sleeptime=30; 
			g_UserSet.onlinetime=29;

			p=strstr(buf,"time");
			if(p!=NULL)
			{
				time=atol(p+7);

				g_OtaInfor.time=time;
				}
			
			p=strstr(buf,"upgrade");

			if(p!=NULL)
			{
				 memcpy((uint8_t*)&g_OtaAuth,(uint8_t*)(OTA_START_ADDR),sizeof(OTA_BIN_AUTH_TypeDef));

				 p=strstr(buf,"auth");  //文件大小
				 if(p!=NULL)
				 {

					memcpy(EncodePack,p+7,AtCmdGetValueLen(p+7,'\"'));

					len=Base64Decode(EncodePack,DecodePack);
					pTemp=(uint8_t*)&g_OtaAuth;

					for(i=0;i<sizeof(OTA_BIN_AUTH_TypeDef);i++)
					{
						if(DecodePack[i]!=pTemp[i])
							break;
						}

					if(i<sizeof(OTA_BIN_AUTH_TypeDef)||g_OtaInfor.addr==0)//new bin file
					{
						memcpy((uint8_t*)&g_OtaAuth,DecodePack,sizeof(OTA_BIN_AUTH_TypeDef));

						memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));
					
						for(i=0;i<OTA_PAGE_NUM/2;i++)
						{	
							FlashPageErase(OTA_START_ADDR+i*PAGE_SIZE);
							}
						OtaPrintf("--ota auth len=%d   ,new file\r\n",len);	
						}
					else
					{
						OtaPrintf("--ota auth len=%d   ,re-download file\r\n",len);
						g_OtaInfor.breakcontinue=TRUE;
						}

					if(strstr(g_OtaAuth.devinfor,"vcu_ota_bin\0")!=NULL)
					{
						g_OtaInfor.state=OTA_STATE_UPGRADE;
						}
										
					}
				 
				//memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));
				/*if(g_OtaInfor.addr==0) //no ota or re-poweron
				{
					memset((uint8_t*)&g_OtaInfor,0x00,sizeof(OTA_COM_TypeDef));
					
					for(i=0;i<OTA_PAGE_NUM/2;i++)
					{	
						FlashPageErase(OTA_START_ADDR+i*PAGE_SIZE);
						}
					}*/
				
				p=strstr(buf,"ver");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(g_OtaInfor.ver,p+6,4);
					}

				p=strstr(buf,"key");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(g_OtaInfor.key,p+6,16);
					}

				p=strstr(buf,"size");  //文件大小
				if(p!=NULL)
				{
					g_OtaInfor.filesize=atol(p+6);
					}
				
				p=strstr(buf,"time");  //文件大小
				if(p!=NULL)
				{
					g_OtaInfor.time=atol(p+6);
					}

				memset(tempBuff,0x00,128);

				if(g_OtaInfor.state==OTA_STATE_UPGRADE)
				{
					if(g_OtaInfor.breakcontinue)
						sprintf((char*)tempBuff,"\"ota\":\"ready\",\"time\":%ld,\"size\":%d,\"addr\":%d",g_OtaInfor.time,g_OtaInfor.filesize,g_OtaInfor.addr);
					else	
						sprintf((char*)tempBuff,"\"ota\":\"ready\",\"time\":%ld,\"size\":%d",g_OtaInfor.time,g_OtaInfor.filesize);
					}
				else
				{
					sprintf((char*)tempBuff,"\"ota\":\"errbin\",\"time\":%ld,\"size\":%d",g_OtaInfor.time,g_OtaInfor.filesize);
					}
				
				OtaAckPublishPayload(tempBuff);

				g_OtaInfor.breakcontinue=FALSE;

				/*p=strstr(buf,"crc");  //文件CRC32 
				if(p!=NULL)
				{
					g_OtaInfor.crc32=atol(p+5);
					}*/
				}


			p=strstr(buf,"firmware");

			if(p!=NULL)
			{
				p=strstr(buf,"addr");//数据包固件起始地址
				if(p!=NULL)
				{
					addr=atol(p+6);
					}

				p=strstr(buf,"len");//DATA数据包长度
				if(p!=NULL)
				{
					len=atol(p+5);
					}

				p=strstr(buf,"crc"); //base64 数据包 CRC
				if(p!=NULL)
				{
					crc=atol(p+5);
					}
				
				p=strstr(buf,"time");  //文件大小
				if(p!=NULL)
				{
					time=atol(p+6);
					}

				p=strstr(buf,"data");//base64 数据包
				if(p!=NULL&&g_OtaInfor.state==OTA_STATE_UPGRADE)
				{
					
					uint32_t cmpcrc=0;

					OtaPrintf("--ota addr=%d   \r\n",addr);	
					
					if(g_OtaInfor.addr==addr)
					{
						memcpy(EncodePack,p+7,len);
						len=Base64Decode(EncodePack,DecodePack);

						cmpcrc=CRC16(DecodePack, len);

						OtaPrintf("--ota len=%d crc1=%d crc2=%d \r\n",len,CRC16(DecodePack, len),crc);	
						
						if(cmpcrc==crc)
						{
							OtaPrintf("---------crc %X----------- \r\n",crc);		
							FlashPageProgram(OTA_START_ADDR+g_OtaInfor.addr,len/4,(uint32_t*)DecodePack);
							g_OtaInfor.addr+=len;
							//g_OtaAddr
							}
						}
					
					}

				memset(tempBuff,0x00,128);
				if(g_OtaInfor.state==OTA_STATE_UPGRADE)
				{	sprintf((char*)tempBuff,"\"ota\":\"ack\",\"time\":%ld,\"addr\":%d",time,g_OtaInfor.addr);}
				else
				{	sprintf((char*)tempBuff,"\"ota\":\"invalid request\",\"time\":%ld",time);}
				
				OtaAckPublishPayload(tempBuff);
				}

			p=strstr(buf,"complete");

			if(p!=NULL)
			{
				p=strstr(buf,"ver");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(ver,p+6,4);
					}

				p=strstr(buf,"key");// 更新软件版本号
				if(p!=NULL)
				{
					memcpy(key,p+6,16);
					}

				/*p=strstr(buf,"size");  //文件大小
				if(p!=NULL)
				{
					size=atol(p+6);
					}*/

				memset(tempBuff,0x00,128);

				OtaPrintf("---------size:%d -----auth size:%d------ \r\n",g_OtaInfor.filesize,g_OtaInfor.addr-sizeof(OTA_BIN_AUTH_TypeDef));

				if(g_OtaInfor.filesize==g_OtaInfor.addr-sizeof(OTA_BIN_AUTH_TypeDef)&&(g_OtaInfor.state==OTA_STATE_UPGRADE))
				{
					OTA_BIN_AUTH_TypeDef auth;
					uint32_t verifyCrc32=0;

					memcpy((uint8_t*)&auth,(uint8_t*)(OTA_START_ADDR),sizeof(OTA_BIN_AUTH_TypeDef));

					verifyCrc32=CRC32(( uint8_t*)(OTA_START_ADDR+sizeof(OTA_BIN_AUTH_TypeDef)),g_OtaInfor.filesize);

					OtaPrintf("---------verifyCrc32:%x -----crc32:%x------ \r\n",verifyCrc32,auth.crc32);	
					OtaPrintf("---------size:%d -----auth size:%d------ \r\n",g_OtaInfor.filesize,auth.length);
					if(verifyCrc32==auth.crc32)//校验正确
					{
						
						OtaPrintf("---------checksum succ ----------- \r\n");	
						g_OtaInfor.rebootTimer=HAL_GetTick();
						g_OtaInfor.succ=TRUE;
						g_OtaInfor.state=OTA_STATE_RESET;
						sprintf((char*)tempBuff,"\"ota\":\"reboot\",\"time\":%d",g_OtaInfor.time);
						//HAL_Delay(2);
						}
					else
					{	sprintf((char*)tempBuff,"\"ota\":\"crcerr\",\"time\":%d",g_OtaInfor.time);
						}

					}
				else
				{
					if(g_OtaInfor.state==OTA_STATE_UPGRADE)
						sprintf((char*)tempBuff,"\"ota\":\"sizeerr\",\"time\":%d",g_OtaInfor.time);
					else
						sprintf((char*)tempBuff,"\"ota\":\"invalid request\",\"time\":%d",g_OtaInfor.time);
					}

				g_OtaInfor.addr=0;
				
				OtaAckPublishPayload(tempBuff);
				}
			
			}
		}
}

//uint32_t verifyCrc32;
void OtaProc(void)
{

	if(TimerGetEventState(TIMER_OTA_EXIT))
	{	
		g_UserSet.sleeptime=g_OtaInfor.sleep; 
		g_UserSet.onlinetime=g_OtaInfor.online;
		EEpUpdateEnable();
		TimerEventClear(TIMER_OTA_EXIT);
		g_OtaInfor.state=OTA_STATE_IDLE;
		OtaPrintf("---------OTA TIMEOUT----------- \r\n");
		}

	//verifyCrc32=CRC32(( __IO uint8_t*)(ApplicationAddress/*+sizeof(OTA_BIN_AUTH_TypeDef)*/),/*78552+sizeof(OTA_BIN_AUTH_TypeDef)*/(uint32_t)79472);
	if(g_OtaInfor.succ)
	{
		if(HAL_GetTick()-g_OtaInfor.rebootTimer>10000u)
		{
			uint32_t Flash_data;
			OtaPrintf("---------system reset----------- \r\n");

			FlashPageErase(ApplicationAddress+PAGE_SIZE*127-0X2000);
			HAL_Delay(2);
			Flash_data=BootOtaModeflag;
			FlashPageProgram(UpgradeOtaflagAddress,4,&Flash_data);
			__disable_fault_irq(); 
			NVIC_SystemReset();
			}
		}

}




