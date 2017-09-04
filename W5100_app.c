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
#include "W5100.h"						/* W5100定义 */
#include "Conmunication.h"

/*****************************************************************
程序名: W5100_Initialization
输入: 无
输出: 无
返回: 无
说明：先对W5100初始化，然后检查网关，最后分别初始化4个端口
*****************************************************************/
void W5100_Initialization(void)
{
	W5100_Init();

	/* 检查网关服务器 */
	Detect_Gateway();

	/* 端口0 */
	Socket_Init(0);
}

/*****************************************************************
程序名: W5100_Socket_Set
输入: 无
输出: 端口状态Socket_Statex
返回: 无
说明：分别设置4个端口，根据端口工作模式，将端口置于TCP服务器、TCP客户端
      或UDP模式。
      从端口状态字节Socket_Statex可以判断端口的工作情况
*****************************************************************/
void W5100_Socket_Set(void)
{
	/* 端口 0 */
	if(S0_State==0)
	{
		if(S0_Mode==TCP_SERVER)			/* TCP服务器模式 */
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
	}
}


/*********************************************************************
程序名: Process_Socket_Data
输入: 端口号
输出: 无
返回:
说明：本过程先调用S_rx_process()从W5100的端口接收数据缓冲区读取数据，
	然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。

	处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
	发送数据。
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

//查询打包名称
void SearchPackName(SOCKET s,unsigned char pStep)
{
	u16 PackLength;

	if (isSdCardOk())
	{
		//查找zip包    并提示大小
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

//查询打包进度
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
//查询打包数据
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
	
	//先清除buffer
	memset(Temp_Buffer,0,sizeof(Temp_Buffer));
	//赋值buffer
	size = S_rx_process(s);
	memcpy(Temp_Buffer,Rx_Buffer,size);
	if((Temp_Buffer[0] == 'A')&&(Temp_Buffer[1] == 'A')&&(Temp_Buffer[3] == 'A')&&(Temp_Buffer[4] == 'A'))  //查询帧头
	{
		if(Temp_Buffer[size+1] == CrcCheck(Temp_Buffer,size)) //查询校验和
		{
			size = (Temp_Buffer[2]<<8)|Temp_Buffer[3];
		
			switch(Temp_Buffer[4])
			{
				case 0x43:																		 //查询打包进度报文
					SearchPackProgress(Temp_Buffer[5]);
					break;
				
				case 0x44:                                     //查询打包名称报文
					SearchPackName(s,Temp_Buffer[5]);
					break;
				
				case 0x45:                                      //查询打包数据报文
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
    /* 设置W5100端口 */
    W5100_Socket_Set();

    /* 处理W5100中断 */
    if(W5100_Interrupt)
    W5100_Interrupt_Process();
		
    /* 如果Socket0接收到数据 */
    if((S0_Data & S_RECEIVE) == S_RECEIVE)
    {
        S0_Data&=~S_RECEIVE;
        Process_Socket_Data(0);
    }
}
