#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ucos_ii.h"
#include "stdlib.h"
#include "TempTask.h"
#include "defines.h"
#include "global.h"
#include "macro_def.h"
#include "stm32f4xx.h"
#include "tm_stm32f4_fatfs.h"
#include "Device.h"
#include "W5100.h"						/* W5100���� */
#include "Conmunication.h"

/*****************************************************************
������: W5100_Initialization
����: ��
���: ��
����: ��
˵�����ȶ�W5100��ʼ����Ȼ�������أ����ֱ��ʼ��4���˿�
*****************************************************************/
void W5100_Initialization(void)
{
	W5100_Init();

	/* ������ط����� */
	Detect_Gateway();

	/* �˿�0 */
	Socket_Init(0);
}

/*****************************************************************
������: W5100_Socket_Set
����: ��
���: �˿�״̬Socket_Statex
����: ��
˵�����ֱ�����4���˿ڣ����ݶ˿ڹ���ģʽ�����˿�����TCP��������TCP�ͻ���
      ��UDPģʽ��
      �Ӷ˿�״̬�ֽ�Socket_Statex�����ж϶˿ڵĹ������
*****************************************************************/
void W5100_Socket_Set(void)
{
	/* �˿� 0 */
	if(S0_State==0)
	{
		if(S0_Mode==TCP_SERVER)			/* TCP������ģʽ */
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
	}
}


/*********************************************************************
������: Process_Socket_Data
����: �˿ں�
���: ��
����:
˵�����������ȵ���S_rx_process()��W5100�Ķ˿ڽ������ݻ�������ȡ���ݣ�
	Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���

	������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
	�������ݡ�
*********************************************************************/
void Process_Socket_Data1(SOCKET s)
{
	unsigned short size;

	size=S_rx_process(s);
	memcpy(Temp_Buffer, Rx_Buffer, size);

	//size=Temp_Buffer[2]+3;
	memcpy(Tx_Buffer, Temp_Buffer, size);
	S_tx_process(s, size);
}

DIR w5100_SdInsDir;
FILINFO w5100_SdFilInfo;
char Infor[128] = {0};
TCHAR PackName[13] = {0};
char filename[0x60] = {0};

//��ѯ�������
void SearchPackName(SOCKET s,unsigned char pStep)
{
	u16 PackLength;

	if (isSdCardOk())
	{
		//����zip��    ����ʾ��С
		strcpy(filename, SD_ROOT_PATH);
		strcat(filename, "\\");
		
		while(!(f_findfirst(&w5100_SdInsDir,&w5100_SdFilInfo,filename,"*.zip")))
		{
			if(strcmp(PackName,w5100_SdFilInfo.fname))
			{
				strcpy(Infor, "Zip File list:");
				strcat(Infor,w5100_SdFilInfo.fname);
				PackLength = w5100_SdFilInfo.fsize;
				strcpy(PackName,w5100_SdFilInfo.fname);
			}
			else
			{
				break;
			}
		}
		f_closedir (&w5100_SdInsDir);
	}
	memcpy(Tx_Buffer, Infor, 128);
	S_tx_process(s, 128);
}

//��ѯ�������
void PackBinData(SOCKET s)
{
	strcpy(filename, SD_ROOT_PATH);
	strcat(filename, "\\");
	
	if(isSdCardOk)
	{
		while(!(f_findfirst(&w5100_SdInsDir,&w5100_SdFilInfo,filename,"*bin")))
		{
				if(strcmp(PackName,w5100_SdFilInfo.fname))
				{
					strcpy(Infor, "Bin File list:");
					strcat(Infor,w5100_SdFilInfo.fname);
					strcpy(PackName,w5100_SdFilInfo.fname);
				}
				else
				{
					break;
				}
		}
		f_closedir (&w5100_SdInsDir);
	}
	memcpy(Tx_Buffer, Infor, 128);
	S_tx_process(s, 128);
}

void SearchPackProgress(unsigned char pStep)
{
	if (isSdCardOk())
	{
		
		strcpy(Infor, "Package Begin!");
	
	
	}
}
//��ѯ�������
void SearchPackData(SOCKET s,unsigned char pStep)
{
	char fileData[256];
	u32 u32_WRDataNum;
	if (isSdCardOk())
	{
		strcpy(filename, SD_ROOT_PATH);
		strcat(filename, "\\");
		strcat(filename, "001.zip");
		if(f_open(&gtag_SdInsfile, (const TCHAR*)filename, FA_OPEN_EXISTING|FA_READ) == FR_OK)
		{
			if(f_read(&gtag_SdInsfile, fileData, 256, &u32_WRDataNum) == FR_OK)
			{
				strcpy(Infor, fileData);
			}
		}
		f_closedir (&w5100_SdInsDir);
	}
	memcpy(Tx_Buffer, Infor, 128);
	S_tx_process(s, 128);
}

void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	
	//�����buffer
	memset(Temp_Buffer,0,sizeof(Temp_Buffer));
	//��ֵbuffer
	size = S_rx_process(s);
	memcpy(Temp_Buffer,Rx_Buffer,size);
	if((Temp_Buffer[0] == 'A')&&(Temp_Buffer[1] == 'A')&&(Temp_Buffer[3] == 'A')&&(Temp_Buffer[4] == 'A'))  //��ѯ֡ͷ
	{
		if(Temp_Buffer[size+1] == CrcCheck(Temp_Buffer,size)) //��ѯУ���
		{
			size = (Temp_Buffer[2]<<8)|Temp_Buffer[3];
		
			switch(Temp_Buffer[4])
			{
				case 0x43:																		 //��ѯ������ȱ���
					SearchPackProgress(Temp_Buffer[5]);
					break;
				
				case 0x44:                                     //��ѯ������Ʊ���
					SearchPackName(s,Temp_Buffer[5]);
					break;
				
				case 0x45:                                      //��ѯ������ݱ���
					SearchPackData(s,Temp_Buffer[5]);
					break;
				default:
					break;	
			}
		}
	}
}



void W5100_LoopWork1(void)
{
    /* ����W5100�˿� */
    W5100_Socket_Set();

    /* ����W5100�ж� */
    if(W5100_Interrupt)
    W5100_Interrupt_Process();
		
    /* ���Socket0���յ����� */
    if((S0_Data & S_RECEIVE) == S_RECEIVE)
    {
        S0_Data&=~S_RECEIVE;
        Process_Socket_Data(0);
    }
}
