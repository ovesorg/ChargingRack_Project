#include"main.h"
#include"string.h"
#include <stdio.h>
#include <stdarg.h>

MQTT_STATE_DEF g_Mqtt_State=MQTT_STATE_IDLE;
SMS_STATE_DEF g_Sms_State=SMS_STATE_IDLE;
uint8_t  g_Mqtt_Request=MQTT_REQ_NONE;
uint8_t  g_Mqtt_ClinetConnect=0;

uint8_t  g_Mqtt_GattReported=0;

uint8_t g_Mqtt_hold=FALSE;
uint8_t g_SimCard_State=TRUE;
uint8_t g_NetConnect_State=FALSE;
__IO uint8_t g_GsmWkup_Event=FALSE;
uint8_t g_MqttReconnect=FALSE;


extern uint8_t g_AtAckState;
extern TIMER_TypeDef g_AtTimeout_timer;

extern UART_HandleTypeDef huart1;
extern uint8_t g_AtCmdState;
extern uint8_t g_AtAckState;

extern TIMER_TypeDef g_AtAckTout_timer;


extern USER_SET_TypeDef g_UserSet;

extern uint8_t g_GsmExist;

extern uint8_t g_mqtt_broker_index;

uint8_t GmsGetSimcardState(void)
{
	return g_SimCard_State;
}

void  GmsSetSimcardState(uint8_t state)
{
	 g_SimCard_State=state;
}

uint8_t  GmsNetConnectState(void)
{
	return  g_NetConnect_State;
}

void  GmsSetNetConnectState(uint8_t state)
{
	  g_NetConnect_State=state;
}


void  GmsWakeup(void)
{
	  g_GsmWkup_Event=TRUE;
}


void GsmComIdle(void)
{
	g_Mqtt_State=MQTT_STATE_IDLE;
	g_Sms_State=SMS_STATE_IDLE;
	TimerAtTOutStop();
}

void GsmComInit(void)
{
	AtCmdInit();
	MqttInit();
	//#ifndef MODULE_4G
	#ifdef SMS_4G_SUPPORT
	SmsInit();
	#endif
	//#endif
}

void GsmComProc(void)
{
	//#ifndef MODULE_4G
	#ifdef SMS_4G_SUPPORT
	SmsProc();
	#endif
	//#endif
	MqttProc();
	AtCmdProc();

	if(g_GsmWkup_Event)
	{
		g_GsmWkup_Event=FALSE;
		HAL_GPIO_WritePin(GSM_EN_GPIO_Port, GSM_EN_Pin, GPIO_PIN_SET);
		#ifdef GD32F10X_MD
		g_Mqtt_GattReported=FALSE;
		HAL_GPIO_WritePin(POWER_4G_CTRL_GPIO_Port, POWER_4G_CTRL_Pin, GPIO_PIN_SET);
	    #endif
		}

	if(GetTimerGsmComEvent())
	{
		GsmComInit();
		//HAL_Delay(2000); 
		TimerSleepSet();

		TimerEventClear(TIMER_GSMCOM);
		}
}

void MqttInit(void)
{
	g_Mqtt_State=MQTT_STATE_IDLE;
	#ifdef MODULE_4G
	#ifdef SMS_4G_SUPPORT
	//g_Mqtt_State=MQTT_STATE_INIT;
	#else
	g_Mqtt_State=MQTT_STATE_INIT;
	#endif
	HAL_GPIO_WritePin(GSM_EN_GPIO_Port, GSM_EN_Pin, GPIO_PIN_SET);

	#ifdef GD32F10X_MD
	g_Mqtt_GattReported=FALSE;
	HAL_GPIO_WritePin(POWER_4G_CTRL_GPIO_Port, POWER_4G_CTRL_Pin, GPIO_PIN_SET);
    #endif
	#endif
}

void SmsInit(void)
{
	#ifdef CAMP_PROJECT
	if(EEpGetLowBatState()==FALSE)
	#endif
	{	g_Sms_State=SMS_STATE_INIT;
		HAL_GPIO_WritePin(GSM_EN_GPIO_Port, GSM_EN_Pin, GPIO_PIN_SET);
		#ifdef GD32F10X_MD
		HAL_GPIO_WritePin(POWER_4G_CTRL_GPIO_Port, POWER_4G_CTRL_Pin, GPIO_PIN_SET);
	    #endif
		}
}

void MqttSetRequest(uint8_t request)
{
	g_Mqtt_Request=request;
}

uint8_t MqttGetRequest(void)
{
	return g_Mqtt_Request;
}

uint8_t MqttGetState(void)
{
	return g_Mqtt_State;
}

static uint8_t ftp_statt = 0,g_Mqtt_State_ftp=FTP_STATE_IDLE;

void set_ftpota_flag(uint8_t flag)
{
	ftp_statt = flag;
}

void MqttProc(void)
{
	uint8_t i=0;
	
	if(g_Mqtt_hold)
		return ;
	
	if(ftp_statt == 0)
	{	
		g_Mqtt_State_ftp = FTP_STATE_IDLE;
		switch(g_Mqtt_State)
		{
			case MQTT_STATE_IDLE:  
				break;
			case MQTT_STATE_INIT:
				AtCmdMerge(AT_CMD_AT);
				g_Mqtt_State=MQTT_STATE_AT;

				g_Mqtt_ClinetConnect=0;
				g_mqtt_broker_index=0;
				
				TimerAtTOutStart(5000U,TRUE);
				break;
			case MQTT_STATE_AT:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CPIN;
					}
				break;
			case MQTT_STATE_CPIN:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CPIN);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CSQ;
					}
				break;
			case MQTT_STATE_CSQ: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CSQ);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CICCID;
					
					CampStart();
					}
				break;
			case MQTT_STATE_CICCID:	
			if(g_AtAckState==AT_ACK_OK||g_AtAckTout_timer.retry)
				{	AtCmdMerge(AT_CMD_CICCID);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CEMODE;
					}
				break;		
			case MQTT_STATE_CEMODE:	
			if(g_AtAckState==AT_ACK_OK||g_AtAckTout_timer.retry)
				{	AtCmdMerge(AT_CMD_CEMODE);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CREG;
					}
				break;	
				
			case MQTT_STATE_CREG:
				if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge(AT_CMD_CREG);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_COPS_Q;
					
					}
				break;
			case MQTT_STATE_COPS_Q:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_COPS_Q);
					TimerAtTOutStart(5000U,TRUE);

					//g_Mqtt_State=MQTT_STATE_COPS;
					g_Mqtt_State=MQTT_STATE_CGREG;
					
					}
				break;	
			case MQTT_STATE_COPS:
				if(g_AtAckState==AT_ACK_OK||g_AtAckTout_timer.retry)
				{	AtCmdMerge(AT_CMD_COPS);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CGREG;
					}
				break;		
			case MQTT_STATE_CGREG:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CGREG);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CGDCONT;//MQTT_STATE_CPSI;
					
					}
				break;	
			case MQTT_STATE_CGDCONT: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CGDCOUNT);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CPSI;//MQTT_STATE_CGACT;
					}
				break;
			case MQTT_STATE_CFUN:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CFUN);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CPSI;
					}	
			case MQTT_STATE_CPSI:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CPSI);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CPSI_CHCK;
					}
				break;	
			case MQTT_STATE_CPSI_CHCK:
				if(g_AtAckTout_timer.retry/*g_AtAckState==AT_ACK_OK&&*/)
				{	
					//HAL_Delay(500);
					AtCmdMerge(AT_CMD_CPSI);
					TimerAtTOutStart(2000U,TRUE);

					/*if(g_GsmExist)
						g_Mqtt_State=MQTT_STATE_CGDCONT;
					else
						g_Mqtt_State=MQTT_STATE_MQTTSTART;*/
					}
				break;
			case MQTT_STATE_CONNECT_CSQ:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CSQ);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CGACT;//MQTT_STATE_CGDCONT;
					}
				break;
			/*case MQTT_STATE_CGDCONT: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CGDCOUNT);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_CGACT;
					}
				break;*/
			case MQTT_STATE_CGACT: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CGACT);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_MQTTSTART;
					}
				break;	
			case MQTT_STATE_MQTTSTART: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTSTART);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTACCQ;

					AtSetSubscribeId();
					AtSetTopicId(NULL);
					}
				break;
			case MQTT_STATE_MQTTACCQ: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTACCQ);
					TimerAtTOutStart(6000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTCONNECT;
					}
				break;	
			case MQTT_STATE_MQTTCONNECT: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTCONNECT);
					TimerAtTOutStart(15000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTSUB;
					}
				if(g_AtAckTout_timer.retry>=AT_RETRY_CNT)//error counter
				{	
					if(g_SimCard_State)
					{	g_UserSet.report_fail_cnt++;
						EEpUpdateEnable();
						}
	
					AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(6000U,TRUE);
					g_Mqtt_State=MQTT_STATE_INIT;
					}
				
				break;		
			case MQTT_STATE_PUB_CSQ:
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CSQ);
					TimerAtTOutStart(5000U,TRUE);

					g_Mqtt_State=MQTT_STATE_MQTTSUB;
					}
				break;	
			case MQTT_STATE_MQTTSUB: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTSUB);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTSUBACK;
					}
				break;
			case MQTT_STATE_MQTTSUBACK:	
			    
				if(g_AtAckState==AT_ACK_OK)
				{
					//AtCmdMerge(AT_CMD_CGNSSPWR);
					//HAL_Delay(500);
					g_NetConnect_State=TRUE;
					LogPrintf("MQTTSUBACK reportt_auto= %d g_MqttReconnect=%d g_Mqtt_ClinetConnect=%d g_Mqtt_Request=%d\r\n",g_UserSet.reportt_auto,g_MqttReconnect,g_Mqtt_ClinetConnect,g_Mqtt_Request);
					
					if(g_UserSet.reportt_auto&&g_MqttReconnect==FALSE&&g_Mqtt_ClinetConnect>=1)
					{	
						AtCmdMerge(AT_CMD_CMQTTTOPIC);
						TimerAtTOutStart(5000U,TRUE);
						g_Mqtt_State=MQTT_STATE_MQTTPAYLOAD;
						g_Mqtt_Request=MQTT_REQ_ALLFIELD;
						#ifdef MODULE_4G_USB_TEST
						g_Mqtt_Request=MQTT_REQ_RAML;
						#endif
						
						LogPrintf("Report data \r\n");
						}
					else
					{	
						if((EEpGetWakeupCnt()>=EEpGetHeartbeat())&&g_MqttReconnect==FALSE)
						{	
							GattSingleFieldMerge("ppid");
							MqttSetRequest(MQTT_REQ_SINGLEFIELD);
							EEpSetWakeupCnt(0);

							LogPrintf("Report heartbeat \r\n");
							}
						else
						{	
							if(g_MqttReconnect)
							{
								#ifdef GD32F10X_MD
								if(g_Mqtt_GattReported==FALSE)
								{
									MqttSetRequest(MQTT_REQ_ALLFIELD);
									LogPrintf("Report all field \r\n");
									}
								else
								#endif
								{
									//do nothing
									//GattSingleFieldMerge("ppid");
									//MqttSetRequest(MQTT_REQ_SINGLEFIELD);
									//g_Mqtt_Request=MQTT_REQ_ALLFIELD;
									LogPrintf("Report none \r\n");
									}
								}
							else
							{
								g_Mqtt_Request=MQTT_REQ_NONE;
								LogPrintf("Report none \r\n");
								}
							}
						
						AtCmdMerge(AT_CMD_NONE);
						TimerAtTOutStart(5000U,TRUE);
						
						g_Mqtt_State=MQTT_STATE_CHK_REQUEST;
						g_MqttReconnect=FALSE;
						
					}
					}
				break;		
			case MQTT_STATE_MQTTTOPIC: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTTOPIC);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTPAYLOAD;
					}
				break;		
            case MQTT_STATE_MQTTPAYLOAD: 
				if(g_AtAckState==AT_ACK_OK)
				{	
					switch(MqttGetRequest())
					{
						case MQTT_REQ_NONE:
							GattAllFieldJsonMerge();	
							break;
						case MQTT_REQ_ALLFIELD:
							GattAllFieldJsonMerge();
							#ifdef GD32F10X_MD
							g_Mqtt_GattReported=TRUE;
							#endif
							break;
						case MQTT_REQ_DTTYPE:
							break;
						case MQTT_REQ_SINGLEFIELD:
							break;
						case MQTT_REQ_CMD:
							break;	
						case MQTT_REQ_RAML:
							GattMultiFieldMerge(); 
							break;
						#ifdef ABACUSLEDER_SUPPORT
						case MQTT_REQ_ABAC:
							GattAbacFieldMerge(); 
							break;
						#endif
						case MQTT_REQ_SLOT_BMS:
							#ifdef CHARGE_STATION
							GattSlotBmsFieldMerge();
							#endif
							break;
						case MQTT_REQ_OTA:
							break;
						case MQTT_REQ_OTA_DATA:
							break;
						case MQTT_REQ_OTA_SUCC:
							break;
						}

					AtCmdMerge(AT_CMD_CMQTTPAYLOAD);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTPUB;
					}
				break;	 
			case MQTT_STATE_MQTTPUB: 
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CMQTTPUB);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_Request=MQTT_REQ_NONE;
					g_Mqtt_State=MQTT_STATE_CHK_REQUEST;
					}
				break;		

			case MQTT_STATE_CHK_REQUEST:
				if(g_AtAckTout_timer.retry)
				{
					//TimerAtTOutStop();
					AtCmdMerge(AT_CMD_AT);
					//AtCmdMerge(AT_CMD_SIMSWITCH);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CLBS4G;
				}
				
				if(g_Mqtt_Request)
				{
					AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTTOPIC;
					break;
					}

				if(g_Mqtt_ClinetConnect==0)
				{
					AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTACCQ;
					g_mqtt_broker_index=1;
					g_Mqtt_ClinetConnect=1;
					}
				else
				{
					/*g_mqtt_broker_index=1;
					
					if(EEpGetWakeupCnt()>=EEpGetHeartbeat())
					{	
						GattSingleFieldMerge("ppid");
						MqttSetRequest(MQTT_REQ_SINGLEFIELD);
						EEpSetWakeupCnt(0);
						g_mqtt_broker_index=0;

						AtCmdMerge(AT_CMD_AT);
						TimerAtTOutStart(5000U,TRUE);
						g_Mqtt_State=MQTT_STATE_MQTTTOPIC;
						}*/
					}
	
				if(g_MqttReconnect)
				{
					AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_MQTTSTART;

					g_Mqtt_ClinetConnect=0;
					}
				break;	
			case MQTT_STATE_CGPSINFO:	
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CGPSINFOR);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CLBS4G;
					}
				break;
			case MQTT_STATE_CLBS4G:	
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CLBS);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_RT_CSQ;
					}
				break;	
			case MQTT_STATE_RT_CSQ:	
				if(g_AtAckState==AT_ACK_OK)
				{	AtCmdMerge(AT_CMD_CSQ);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State=MQTT_STATE_CHK_REQUEST;
					}
				break;		
				
			case MQTT_STATE_END:
				g_Mqtt_State=MQTT_STATE_IDLE;
				break;
			}
		}

	if(ftp_statt == 1)	
	{
	 //g_Mqtt_State_ftp = FTP_STATE_OPEN;
		while(1)
		{fwdgt_counter_reload();
			switch(g_Mqtt_State_ftp)
			{
				case FTP_STATE_IDLE:
			 
					printf("Report ppid 1111111111111\r\n");				
					g_Mqtt_State_ftp = FTP_STATE_INIT;
					HAL_Delay(5000);
					g_Mqtt_Request=MQTT_REQ_NONE;
					TimerAtTOutStart(15000U,TRUE);
				break;
				
				case FTP_STATE_INIT:
					printf("Report ppid 22222222222222\r\n");
					AtCmdMerge_ftp(AT_CMD_FTPSTART);
					g_Mqtt_State_ftp=FTP_STATE_LOGIN;

					TimerAtTOutStart(150000U,TRUE);
					HAL_Delay(5000);
				break;
				
				case FTP_STATE_LOGIN:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					printf("Report ppid 3333333333333333333\r\n");
					AtCmdMerge_ftp(AT_CMD_FTPLOGIN);
					TimerAtTOutStart(150000U,TRUE);
					g_Mqtt_State_ftp=FTP_STATE_FILELS; 
					HAL_Delay(15000);
				}
				break;
				
				case FTP_STATE_FILELS:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					clear_filels();
					AtCmdMerge_ftp(AT_CMD_FTPFSLS);
					TimerAtTOutStart(150000U,TRUE);
					g_Mqtt_State_ftp=FTP_STATE_DEL; 
					HAL_Delay(5000);
				}
				break;
				
				case FTP_STATE_DEL:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					if(getlen_filels()> 0)
					{	
						 AtCmdMerge_ftp(AT_CMD_FTPFILEDEL);
						
						TimerAtTOutStart(150000U,TRUE);
						
						HAL_Delay(8000);
					}
					g_Mqtt_State_ftp=FTP_STATE_GET; 
				}
				break;

				case FTP_STATE_GET:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					
					clear_filels();
					AtCmdMerge_ftp(AT_CMD_FTPGETFILE);
					//LogPrintf("Report ppid 444444444444444444\r\n");
					
					g_FtpAtAckState = AT_ACK_NONE;
					TimerAtTOutStart(15000U,TRUE);
					g_Mqtt_State_ftp=FTP_STATE_LOGOUT;
					HAL_Delay(15000);
				}
				break;
				
				case FTP_STATE_LOGOUT:
				//if(g_FtpAtAckState == AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPLOGOUT);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State_ftp=FTP_STATE_STOP;
					HAL_Delay(12000);
				}
				break;
				case FTP_STATE_STOP:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPSTOP);
					TimerAtTOutStart(5000U,TRUE);
					HAL_Delay(5000);
					g_Mqtt_State_ftp=FTP_STATE_OPEN;
				}
				break;
				case FTP_STATE_OPEN:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPFILEOPEN);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State_ftp= FTP_STATE_SEEK;
					HAL_Delay(5000);
					offcount = 0;
					
					for(i=0;i<OTA_PAGE_NUM/2;i++)
					{	
						FlashPageErase(OTA_START_ADDR+i*PAGE_SIZE);
					}
				}
				break;
				
				case FTP_STATE_SEEK:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPFILESLEEK);
					//TimerAtTOutStart(500U,TRUE);
					g_Mqtt_State_ftp= FTP_STATE_READ;
					HAL_Delay(500);
				}
				break;
				
				case FTP_STATE_READ:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPFILEREAD);   

					//TimerAtTOutStart(500U,TRUE);
					if(ota_finsh == 0)
					g_Mqtt_State_ftp=FTP_STATE_SEEK;
					else
					g_Mqtt_State_ftp=FTP_STATE_CLOSE;
					
					HAL_Delay(500);
				}
				break;
				case FTP_STATE_CLOSE:
				//if(g_AtAckState==AT_ACK_OK)
				{	
					AtCmdMerge_ftp(AT_CMD_FTPFILECLOSE);
					TimerAtTOutStart(5000U,TRUE);
					g_Mqtt_State_ftp= FTP_STATE_END;
				}
				break;
//				case FTP_STATE_DEL:
//				//if(g_AtAckState==AT_ACK_OK)
//				{	
////					AtCmdMerge_ftp(AT_CMD_FTPFILEDEL);
//					TimerAtTOutStart(5000U,TRUE);
//					g_Mqtt_State_ftp= FTP_STATE_END;
//				}
//				break;
				
				default :break;
			}
		
			AtCmdProc_ftp();
			
			 if(g_Mqtt_State_ftp==FTP_STATE_OPEN)
			{
				if(ftp_statt == 0)  break;
			}
	  }
	}	
}



void SmsProc(void)
{

	if(ftp_statt == 0)
	{
	switch(g_Sms_State)
	{
		case SMS_STATE_IDLE:  
			break;
		case SMS_STATE_INIT:
			AtCmdMerge(AT_CMD_AT);
			g_Sms_State=SMS_STATE_AT;
			TimerAtTOutStart(10000U,TRUE);
			break;	
		case SMS_STATE_AT:
			if(/*g_AtAckState==AT_ACK_OK&&*/g_AtAckTout_timer.retry)
			{
				AtCmdMerge(AT_CMD_AT);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_ATE0;
			}
			break;
		case SMS_STATE_ATE0:
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_ATE0);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CIPCLOSE;
			}
			break;
		case SMS_STATE_CIPCLOSE:
			//if(g_AtAckState==AT_ACK_OK)
			{
				//AtCmdMerge(AT_CMD_CIPCLOSE);
				TimerAtTOutStart(5000U,TRUE);
				//g_Sms_State=MQTT_STATE_CIPSHUT;
				g_Sms_State=SMS_STATE_CPIN;
			}
			break;
		case SMS_STATE_CIPSHUT: 
			//if(g_AtAckState==AT_ACK_OK||g_AtAckTout_timer.retry>=AT_RETRY_CNT)
			{
				//AtCmdMerge(AT_CMD_CIPSHUT);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CPIN;
			}
			break;
		case SMS_STATE_CPIN:  
			if(g_AtAckState==AT_ACK_OK||g_AtAckTout_timer.retry>=AT_RETRY_CNT)
			{
				AtCmdMerge(AT_CMD_CPIN);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CSQ;
			}
			break;
		case SMS_STATE_CSQ: 
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CSQ);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CREG;
			}
			break;
		case SMS_STATE_CREG:  
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CREG);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CGAT;
			}
			break;
		case SMS_STATE_CGAT:  
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CGATT);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGF;
			}
			break;	
		case SMS_STATE_CMGF:  
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGF);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_WAIT;
			}
			break;
		case SMS_STATE_WAIT:   
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_NONE);
				TimerAtTOutStart(10000U,TRUE);
				g_Sms_State=SMS_STATE_JUMP;
				}
			break;
		case SMS_STATE_JUMP:      // NORMAL JUMP TO MQTT INIT
			if(g_AtAckTout_timer.retry)
			{
				TimerAtTOutStop();
				g_Sms_State=SMS_STATE_IDLE;//SMS_STATE_IDLE;
				LogPrintf("GSM:Jump to Mqtt connect \r\n");
				g_Mqtt_State=MQTT_STATE_INIT;  //ENTER TO MQTT SEND  PUBLISH
				}
			break;			
		case SMS_STATE_CPMS:                       //CHECK SMS
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CPMS);
				TimerAtTOutStart(30000U,TRUE);
				g_Sms_State=SMS_STATE_CMGR;
			}
			break;
		case SMS_STATE_CMGR: 				//READ AND PARSE SMS
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGR);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGR_PARSE;//SMS_STATE_CMGS;    //ENTER SMS SEND MODE
			}
			break;
		case SMS_STATE_CMGR_PARSE:
			{
				if(g_AtAckState==AT_ACK_OK)
				{
					AtCmdMerge(AT_CMD_NONE);
					TimerAtTOutStart(3000U,TRUE);
					}
				
				if(g_AtAckTout_timer.retry)
				{
					AtCmdMerge(AT_CMD_AT);
					TimerAtTOutStart(5000U,TRUE);
					g_Sms_State=SMS_STATE_CMGS;    //ENTER SMS SEND MODE
					}
			}
			break;
		case SMS_STATE_CMGS:    //SEND  SMS START
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGS); //SMS ADDR
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGS_SMS;
			}
			break;
		case SMS_STATE_CMGS_SMS:  // SMS  TXT
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGS_SMS);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGF_0;
			}
			break;	
		case SMS_STATE_READ:    // READ SMS START
			{
				AtCmdMerge(AT_CMD_AT);
				TimerAtTOutStart(5000U,TRUE);
			    g_Sms_State=SMS_STATE_CPMS;
			}
			break;
		case SMS_STATE_CMGF_0:
			if(g_AtAckState==AT_ACK_OK)
			{
				#ifdef MODULE_4G
				AtCmdMerge(AT_CMD_CMGF);
				#else
				AtCmdMerge(AT_CMD_CMGF_0);
				#endif
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGDA;  	//ENTER DELAY MODE
			}
			break;
		case SMS_STATE_CMGDA:  //DEL  ALL SMS
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGDA);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_CMGF_1;	
			}
			break;	
		case SMS_STATE_CMGF_1:
			if(g_AtAckState==AT_ACK_OK)
			{
				AtCmdMerge(AT_CMD_CMGF);
				TimerAtTOutStart(5000U,TRUE);
				g_Sms_State=SMS_STATE_END;
				}
			break;	
		case SMS_STATE_END:
			if(g_AtAckState==AT_ACK_OK)
			{
				TimerAtTOutStop();
				g_Sms_State=SMS_STATE_IDLE;
				g_Mqtt_hold=FALSE;
				}
			break;		
		}
	}	
}


