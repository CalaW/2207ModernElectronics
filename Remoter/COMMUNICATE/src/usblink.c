#include <string.h>
#include "usblink.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "remoter_ctrl.h"
/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/********************************************************************************	 
 * ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
 * ALIENTEK MiniFly_Remotor
 * USBͨ����������	
 * ����ԭ��@ALIENTEK
 * ������̳:www.openedv.com
 * ��������:2018/6/1
 * �汾��V1.0
 * ��Ȩ���У�����ؾ���
 * Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
 * All rights reserved
********************************************************************************/

#define USBLINK_TX_QUEUE_SIZE 16
#define USBLINK_RX_QUEUE_SIZE 16

static enum
{
	waitForStartByte1,
	waitForStartByte2,
	waitForMsgID,
	waitForDataLength,
	waitForData,
	waitForChksum1,
}rxState;

static bool isInit;
static atkp_t rxPacket;
static xQueueHandle  txQueue;
static xQueueHandle  rxQueue;


/*usb���ӳ�ʼ��*/
void usblinkInit(void)
{
	if (isInit) return;
	txQueue = xQueueCreate(USBLINK_TX_QUEUE_SIZE, sizeof(atkp_t));
	ASSERT(txQueue);
	rxQueue = xQueueCreate(USBLINK_RX_QUEUE_SIZE, sizeof(atkp_t));
	ASSERT(rxQueue);
	isInit = true;
}

/*usb���ӷ���atkpPacket*/
bool usblinkSendPacket(const atkp_t *p)
{
	ASSERT(p);
	ASSERT(p->dataLen <= ATKP_MAX_DATA_SIZE);
	return xQueueSend(txQueue, p, 0);
}
bool usblinkSendPacketBlocking(const atkp_t *p)
{
	ASSERT(p);
	ASSERT(p->dataLen <= ATKP_MAX_DATA_SIZE);
	return xQueueSend(txQueue, p, portMAX_DELAY);
}

/*usb���ӽ���atkpPacket*/
bool usblinkReceivePacket(atkp_t *p)
{
	ASSERT(p);
	return xQueueReceive(rxQueue, p, 0);
}
bool usblinkReceivePacketBlocking(atkp_t *p)
{
	ASSERT(p);
	return xQueueReceive(rxQueue, p, portMAX_DELAY);
}

/*usb���ӷ�������*/
void usblinkTxTask(void* param)
{
	atkp_t p;
	u8 sendBuffer[64];
	u8 cksum;
	u8 dataLen;
	while(bDeviceState != CONFIGURED)//��usb���óɹ�
	{
		vTaskDelay(1000);
	}
	while(1)
	{
		xQueueReceive(txQueue, &p, portMAX_DELAY);
		if(p.msgID != UP_RADIO)/*NRF51822�����ݰ����ϴ�*/
		{
			if(p.msgID == UP_PRINTF)/*��ӡ���ݰ�ȥ��֡ͷ*/
			{
				memcpy(&sendBuffer, p.data, p.dataLen);
				dataLen = p.dataLen;
			}
			else
			{
				sendBuffer[0] = UP_BYTE1;
				sendBuffer[1] = UP_BYTE2;
				sendBuffer[2] = p.msgID;
				sendBuffer[3] = p.dataLen;
				memcpy(&sendBuffer[4], p.data, p.dataLen);
				cksum = 0;
				for (int i=0; i<p.dataLen+4; i++)
				{
					cksum += sendBuffer[i];
				}
				dataLen = p.dataLen+5;
				sendBuffer[dataLen - 1] = cksum;
			}
			usbsendData(sendBuffer, dataLen);
		}		
	}
}

/*usb���ӽ�������*/
//������Ҫ���Ĵ���
void usblinkRxTask(void *param)
{
	u8 c;
	u8 dataIndex = 0;
	u8 cksum = 0;
	rxState = waitForStartByte1;
	
	while(1)
	{
		if (usbGetDataWithTimout(&c))
		{
			switch(rxState)
			{
				case waitForStartByte1:
					//USB_SendData(c);
					if(c == '1'){
						rxState = waitForStartByte2;
						//USB_SendData('T');
					}
					else{
						rxState = waitForStartByte1;
						//USB_SendData('F');
					}
					//rxState = (c == '1') ? waitForStartByte2 : waitForStartByte1;
					cksum = c;
					//USB_SendData('1');
					break;
				case waitForStartByte2:
					//USB_SendData(c);
					if(c == '2'){
						rxState = waitForData;
						//USB_SendData('M');
					}
					else{
						rxState = waitForStartByte1;
						//USB_SendData('N');
					}
					//rxState = (c == '2') ? waitForMsgID : waitForStartByte1;
					cksum += c;
					///USB_SendData('2');
					break;
				//case waitForMsgID:
				//	USB_SendData(c);
				  	//rxPacket.msgID = c;
				  	//rxState = waitForData;
				//	cksum += c;
				//	USB_SendData('3');
				  break;
				//case waitForDataLength:
				//	rxPacket.dataLen = c;
				//	rxState =waitForData;
				//	USB_SendData(c);
				//	USB_SendData('4');
				//	break;
				case waitForData:
					rxPacket.data[dataIndex] = c;
					//USB_SendData(c);
					//USB_SendData('5');
					dataIndex++;
					if(dataIndex==3)
					{
						rxState = waitForChksum1;
						dataIndex=0;
					}		
					break;
				case waitForChksum1:
					//if (cksum == c)/*����У����ȷ*/
					//USB_SendData(c);
					//USB_SendData('6');
					if(c == '0')
					{
						//xQueueSend(rxQueue, &rxPacket, 0);
						upAnalyse(&rxPacket);
						//USB_SendData('7');
					} 
					else
					{
						rxState = waitForStartByte1;
					}
					rxState = waitForStartByte1;
					break;
				default:
					break;
			}
		}
		//else	/*��ʱ����*/
		//{
			//rxState = waitForStartByte1;
			//USB_SendData('S');
		//}
	}
}

