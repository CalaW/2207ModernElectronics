#ifndef __COMMANDER_H
#define __COMMANDER_H
#include <stdbool.h>
#include "atkp.h"
#include "joystick.h"
/********************************************************************************	 
 * ������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
 * ALIENTEK MiniFly_Remotor
 * �ɿ�ָ����������	
 * ����ԭ��@ALIENTEK
 * ������̳:www.openedv.com
 * ��������:2018/6/1
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
 * All rights reserved
********************************************************************************/

/*ң���������*/
typedef enum 
{
	REMOTOR_CMD,
	REMOTOR_DATA,
}remoterType_e;

/*��������*/
#define  CMD_GET_MSG		0x01	/*��ȡ������Ϣ���Լ죩*/
#define  CMD_GET_CANFLY		0x02	/*��ȡ�����Ƿ��ܷ�*/
#define  CMD_FLIGHT_LAND	0x03	/*��ɡ�����*/
#define  CMD_EMER_STOP		0x04	/*����ͣ��*/
#define  CMD_FLIP			0x05	/*4D����*/
#define  CMD_POWER_MODULE	0x06	/*�򿪹ر���չģ���Դ*/
#define  CMD_LEDRING_EFFECT	0x07	/*����RGB�ƻ�Ч��*/
#define  CMD_POWER_VL53LXX	0x08	/*�򿪹رռ���*/

/*���б���*/
#define  ACK_MSG			0x01	/*���᷵�ص���Ϣ*/

/*��չģ������ö��*/
enum expModuleID
{
	NO_MODULE,
	LED_RING,
	WIFI_CAMERA,
	OPTICAL_FLOW,
	MODULE1,
};

/*ң�����ݽṹ*/
//�¼���x,y,depth
typedef __packed struct
{
	float x;
	float y;
	float depth;
	float roll;      
	float pitch;  
	float yaw;	
	float thrust;
	float trimPitch;
	float trimRoll;
	u8 ctrlMode;
	bool flightMode;
	bool RCLock;
} remoterData_t;

typedef __packed struct
{
	u8 version;
	bool mpu_selfTest;
	bool baro_slfTest;
	bool isCanFly;
	bool isLowpower;
	enum expModuleID moduleID;
	
	float trimRoll;		/*roll΢��*/
	float trimPitch;	/*pitch΢��*/
} MiniFlyMsg_t;

void commanderTask(void* param);
joystickFlyf_t getFlyControlData(void);
float limit(float value,float min, float max);
void sendRmotorCmd(u8 cmd, u8 data);
void sendRmotorData(u8 *data, u8 len);
void upAnalyse(atkp_t *p);
#endif /*__COMMANDER_H*/