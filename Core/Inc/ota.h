
#ifndef __OTA_H__

#define __OTA_H__


#define  OtaPrintf(...)  printf(__VA_ARGS__)


#define OTA_START_ADDR 0x8000000+1024*128
#define OTA_PAGE_NUM  110

enum
{
	OTA_STATE_IDLE,
	OTA_STATE_UPGRADE,
	OTA_STATE_RESET
};

typedef struct
{
	uint8_t devinfor[16];
	uint8_t time[16];
	uint8_t version[4];
	uint32_t length;
	uint8_t auth[16];
	uint8_t reserved[4];
	uint32_t crc32;// 升级固件CRC
}OTA_BIN_AUTH_TypeDef;

typedef struct
{
	uint8_t ver[8];
	uint8_t key[32];
	uint32_t filesize;
	uint32_t addr;
	uint32_t crc32;
	uint32_t time;
	uint8_t breakcontinue;
	uint8_t state;
	uint8_t reserved;
	uint8_t succ;
	uint32_t rebootTimer;

	uint32_t sleep;
	uint32_t online;
}OTA_COM_TypeDef;





void OtaInit(void);
void OtaProc(void);
void OtaAckPublishPayload(uint8_t *buf);
uint32_t CRC32(uint8_t *pData,uint32_t Length);
void OtaParse(uint8_t * buf);

#endif


