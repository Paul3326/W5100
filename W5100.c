/********************************************************************************
	提供商：成都浩然电子有限公司
	网  址：http://www.hschip.com

    时  间: 2007-11-30

    本软件包括5个部分：
    	1. W5100初始化
    	2. W5100的Socket初始化
    	3. Socket连接
    	   如果Socket设置为TCP服务器模式，则调用Socket_Listen()函数,W5100处于侦听状态，直到远程客户端与它连接。
    	   如果Socket设置为TCP客户端模式，则调用Socket_Connect()函数，
    	                                  每调用一次Socket_Connect(s)函数，产生一次连接，
    	                                  如果连接不成功，则产生超时中断，然后可以再调用该函数进行连接。
    	   如果Socket设置为UDP模式,则调用Socket_UDP函数
    	4. Socket数据接收和发送
    	5. W5100中断处理

    置W5100为服务器模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)，设置过程即完成，等待客户端的连接。
    置W5100为客户端模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)，设置过程即完成，并与远程服务器连接。
    置W5100为UDP模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)，设置过程即完成，可以与远程主机UDP通信。

    W5100产生的连接成功、终止连接、接收数据、发送数据、超时等事件，都可以从中断状态中获得。
********************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ucos_ii.h"

#include "defines.h"
#include "global.h"
#include "macro_def.h"

#include "stm32f4xx.h"
#include"W5100.h"					/* 定义W5100的寄存器地址、状态 */

#define TRUE	0xff
#define FALSE	0x00

typedef unsigned char SOCKET;

/* 端口数据缓冲区 */
extern unsigned char Rx_Buffer[1460];			/* 端口接收数据缓冲区 */
extern unsigned char Tx_Buffer[1460];			/* 端口发送数据缓冲区 */

/* 网络参数寄存器 */
extern unsigned char Gateway_IP[4];	     		/* Gateway IP Address */
extern unsigned char Sub_Mask[4];				/* Subnet Mask */
extern unsigned char Phy_Addr[6];  			/* Physical Address */
extern unsigned char IP_Addr[4];				/* Loacal IP Address */

extern unsigned char S0_Port[2];   			/* Socket0 Port number */
extern unsigned char S0_DIP[4];				/* Socket0 Destination IP Address */
extern unsigned char S0_DPort[2];				/* Socket0 Destination Port number */

extern unsigned char S1_Port[2];   			/* Socket1 Port number */
extern unsigned char S1_DIP[4];   			/* Socket1 Destination IP Address */
extern unsigned char S1_DPort[2];				/* Socket1 Destination Port number */

extern unsigned char S2_Port[2];				/* Socket2 Port number */
extern unsigned char S2_DIP[4];				/* Socket2 Destination IP Address */
extern unsigned char S2_DPort[2];				/* Socket2 Destination Port number */

extern unsigned char S3_Port[2];				/* Socket3 Port number */
extern unsigned char S3_DIP[4];				/* Socket3 Destination IP Address */
extern unsigned char S3_DPort[2];				/* Socket3 Destination Port number */

extern unsigned char S0_State;				/* Socket0 state recorder */
extern unsigned char S1_State;				/* Socket1 state recorder */
extern unsigned char S2_State;				/* Socket2 state recorder */
extern unsigned char S3_State;				/* Socket3 state recorder */
	#define S_INIT	0x01
	#define S_CONN	0x02

extern unsigned char S0_Data;			/* Socket0 receive data and transmit OK */
extern unsigned char S1_Data;			/* Socket1 receive data and transmit OK */
extern unsigned char S2_Data;			/* Socket2 receive data and transmit OK */
extern unsigned char S3_Data;			/* Socket3 receive data and transmit OK*/
	#define S_RECEIVE		0x01
	#define S_TRANSMITOK	0x02

extern unsigned char W5100_Interrupt;

/* UDP Destionation IP address and Port number */
extern unsigned char UDP_DIPR[4];
extern unsigned char UDP_DPORT[2];


#define  BANK1_SRAM1_STADDR    (0x60000000)
#define  BANK1_SRAM2_STADDR    (BANK1_SRAM1_STADDR + 64*1024*1024)
#define  BANK1_SRAM3_STADDR    (BANK1_SRAM2_STADDR + 64*1024*1024)
#define  BANK1_SRAM4_STADDR    (BANK1_SRAM3_STADDR + 64*1024*1024)

#define  W5100_BANK_INDX     (FSMC_Bank1_NORSRAM1)

#define  W5100_STADDR     (BANK1_SRAM1_STADDR)

#define W5100_RST_1()    GPIO_SetBits(GPIOG, GPIO_Pin_7)
#define W5100_RST_0()    GPIO_ResetBits(GPIOG, GPIO_Pin_7)


void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line8))
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        W5100_Interrupt=1;
    }	
}

void W5100_Ctrl_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;    
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
                         
    W5100_RST_1();
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;      
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOG, &GPIO_InitStruct);

    // INT
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOG, &GPIO_InitStruct);    

    /* Connect EXTI Line to INT Pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource8);

    /* Configure EXTI line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set the EXTI interrupt to the highest priority */
    //NVIC_PriorityGroupConfig(CPU_NVIC_GROUP);  
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CPU_NVIC_PREEMP_PRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void W5100_FsmcInit(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;

    GPIO_InitTypeDef GPIO_InitStructure;
    /* Enable GPIOs clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG, ENABLE);

    /* Enable FSMC clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE); 
  
    /*-- GPIOs Configuration -----------------------------------------------------*/
    /*
    +-------------------+--------------------+------------------+------------------+
    | PD0  <-> FSMC_D2  | PF0 <-> FSMC_A0  | 
    | PD1  <-> FSMC_D3  | PF1 <-> FSMC_A1  |
    | PD4  <-> FSMC_NOE |
    | PD5  <-> FSMC_NWE | PD7  <-> FSMC_NE1
    | PE7  <-> FSMC_D4   |
    | PE8  <-> FSMC_D5   |
    | PE9  <-> FSMC_D6   | 
    | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
    | PD15 <-> FSMC_D1    
  */

    /* GPIOD configuration */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_FSMC);     // CLK
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);

    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_FSMC); //NWAIT
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC); //NE1
    
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1 |  GPIO_Pin_4  | GPIO_Pin_5   
                                | GPIO_Pin_7  | GPIO_Pin_14 | GPIO_Pin_15;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);


    /* GPIOE configuration */
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource2 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource3 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource4 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource5 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource6 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
    //GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7 | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 ;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* GPIOF configuration */
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /*-- FSMC Configuration ------------------------------------------------------*/
#if 0
    p.FSMC_AddressSetupTime = 6;
    p.FSMC_AddressHoldTime = 3;
    p.FSMC_DataSetupTime = 6;
    p.FSMC_BusTurnAroundDuration = 1;
    p.FSMC_CLKDivision = 10;    
    p.FSMC_DataLatency = 6;
    p.FSMC_AccessMode = FSMC_AccessMode_B;
//#else       // org
    p.FSMC_AddressSetupTime = 6;
    p.FSMC_AddressHoldTime = 3;
    p.FSMC_DataSetupTime = 6;
    p.FSMC_BusTurnAroundDuration = 1;
    p.FSMC_CLKDivision = 0;    
    p.FSMC_DataLatency = 0;
    p.FSMC_AccessMode = FSMC_AccessMode_A;
   

    FSMC_NORSRAMInitStructure.FSMC_Bank = W5100_BANK_INDX;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

    /*!< Enable FSMC  Bank */
    FSMC_NORSRAMCmd(W5100_BANK_INDX, ENABLE); 
#endif 

    //FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    //FSMC_NORSRAMTimingInitTypeDef  p; 

//    p.FSMC_AddressSetupTime = 0x2;	 //地址建立时间
//    p.FSMC_AddressHoldTime = 0x0;	 //地址保持时间
//    p.FSMC_DataSetupTime = 0x2;		 //数据建立时间
//    p.FSMC_BusTurnAroundDuration = 0x0;
//    p.FSMC_CLKDivision = 10;
//    p.FSMC_DataLatency = 0x0;
//    p.FSMC_AccessMode = FSMC_AccessMode_B;	 // 一般使用模式B来控制LCD
    
		p.FSMC_AddressSetupTime = 6;
    p.FSMC_AddressHoldTime = 3;
    p.FSMC_DataSetupTime = 6;
    p.FSMC_BusTurnAroundDuration = 1;
    p.FSMC_CLKDivision = 1;    
    p.FSMC_DataLatency = 0;
    p.FSMC_AccessMode = FSMC_AccessMode_A;
		
		
    FSMC_NORSRAMInitStructure.FSMC_Bank = W5100_BANK_INDX;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p; 
       
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
    
    /* 使能 FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd(W5100_BANK_INDX, ENABLE);
}




/*****************************************************************
*/
void Write_W5100_Address(unsigned short addr)
{
#if 1
    *(u8*)(W5100_STADDR+1) = (addr>>8)&0xff;
    *(u8*)(W5100_STADDR+2) = addr&0xff;
#else
	unsigned short i;

	/* 设置地址为0x01 */
	GPIO_SetBits(GPIOA, W5100_A0);
	GPIO_ResetBits(GPIOA, W5100_A1);

	/* 输出地址值 */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (addr & 0xff00);
	GPIO_Write(GPIOB, i);

	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOA, W5100_CS);
	GPIO_ResetBits(GPIOA, W5100_WR);
	/*写入地址高8位 */
	GPIO_SetBits(GPIOA, W5100_WR);
	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOA, W5100_CS);


	/* 设置地址为0x02 */
	GPIO_ResetBits(GPIOA, W5100_A0);
	GPIO_SetBits(GPIOA, W5100_A1);

	/* 输出地址值 */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (addr<<8);
	GPIO_Write(GPIOB, i);

	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOA, W5100_CS);
	GPIO_ResetBits(GPIOA, W5100_WR);
	/*写入地址低8位 */
	GPIO_SetBits(GPIOA, W5100_WR);
	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOA, W5100_CS);
	
	/* 设置地址为0x11 */
	GPIO_SetBits(GPIOA, W5100_A0);
#endif	
}

/*****************************************************************
程序名：Read_W5100
输入: 地址
输出: 无
返回: 读取的数据
说明：从W5100指定的地址读取一个字节
*****************************************************************/
unsigned char Read_W5100(void)
{
	unsigned char temp = 0;
#if 1
    temp = *(u8*)(W5100_STADDR+3);
    return temp;
#else
	unsigned short i;

	GPIO_InitTypeDef  GPIO_InitStructure;

	/* 设置数据端口为输入状态 */
	GPIO_InitStructure.GPIO_Pin  = W5100_DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOA, W5100_CS);
	/* 读取数据 */
	GPIO_ResetBits(GPIOA, W5100_RD);
	GPIO_SetBits(GPIOA, W5100_RD);
	i=GPIO_ReadInputData(GPIOB);
	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOA, W5100_CS);

	/* 设置为输出状态 */
	GPIO_InitStructure.GPIO_Pin  = W5100_DATA;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	i >>= 8;
	return i;
#endif	
}

/*****************************************************************
程序名：Write_W5100
输入: 地址，字节数据
输出: 无
返回: 无
说明：将一个字节写入W5100指定的地址
*****************************************************************/
void Write_W5100(unsigned char dat)
{
#if 1
    *(u8*)(W5100_STADDR+3) = dat;
#else
	unsigned short i;

	/* 输出数据到端口 */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (dat<<8);
	GPIO_Write(GPIOB, i);

	/* 置W5100的CS为低电平 */
	GPIO_ResetBits(GPIOA, W5100_CS);
	/* 写命令 */
	GPIO_ResetBits(GPIOA, W5100_WR);
	GPIO_SetBits(GPIOA, W5100_WR);
	/* 置W5100的CS为高电平 */
	GPIO_SetBits(GPIOA, W5100_CS);
#endif	
}

/*------------------------------------------------------------------------------
						W5100初始化函数
在使用W5100之前，对W5100初始化
------------------------------------------------------------------------------*/
volatile unsigned char w5100_read_buf[10];
void W5100_Init(void)
{
	unsigned short i;
    
    W5100_Ctrl_GPIO_Init();
    W5100_FsmcInit();

    W5100_RST_0();
    OSTimeDly(10);
    W5100_RST_1();
    OSTimeDly(1000);

    *(u8*)W5100_STADDR = MODE_AI|MODE_IND;      // indirect mode(bit0),  Auto_Inc_Addr(bit1)

	/*设置网关(Gateway)的IP地址，4字节 */
	/*使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet*/
	Write_W5100_Address(W5100_GAR);
	for(i=0;i<4;i++)
		Write_W5100(Gateway_IP[i]);			/*Gateway_IP为4字节unsigned char数组,自己定义*/


	/*设置子网掩码(MASK)值，4字节。子网掩码用于子网运算*/
	Write_W5100_Address(W5100_SUBR);
	for(i=0;i<4;i++)
		Write_W5100(Sub_Mask[i]);			/*SUB_MASK为4字节unsigned char数组,自己定义*/

	/*设置物理地址，6字节，用于唯一标识网络设备的物理地址值
	该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	如果自己定义物理地址，注意第一个字节必须为偶数*/
	Write_W5100_Address(W5100_SHAR);
	for(i=0;i<6;i++)
		Write_W5100(Phy_Addr[i]);			/*PHY_ADDR6字节unsigned char数组,自己定义*/
      
		
	/*设置本机的IP地址，4个字节
	注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关*/
	Write_W5100_Address(W5100_SIPR);
	for(i=0;i<4;i++)
		Write_W5100(IP_Addr[i]);			/*IP_ADDR为4字节unsigned char数组,自己定义*/

	/*设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册*/
	Write_W5100_Address(W5100_RMSR);
	Write_W5100(0x55);			/*Socket Rx memory size=2k*/
	Write_W5100_Address(W5100_TMSR);
	Write_W5100(0x55);		/*Socket Tx mempry size=2k*/

	/* 设置重试时间，默认为2000(200ms) */
	Write_W5100_Address(W5100_RTR);
	Write_W5100(0x07);
	Write_W5100(0xd0);
    

	/* 设置重试次数，默认为8次 */
	Write_W5100_Address(W5100_RCR);
	Write_W5100(8);

	/* 启动中断，参考W5100数据手册确定自己需要的中断类型
	IMR_CONFLICT是IP地址冲突异常中断
	IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	其它是Socket事件中断，根据需要添加 */
	Write_W5100_Address(W5100_IMR);
	Write_W5100(IMR_CONFLICT|IMR_S0_INT);
    
}

/****************************************************************************
            Detect_UDP_Broadcating Gateway
input:  	None
Output: 	None
Return: 	if fail to detect gateway, return FALSE
		if detect the gateway, return TRUE
****************************************************************************/
unsigned char Detect_UDP_Broadcating(void)
{
	unsigned char i;
        
    Write_W5100_Address(W5100_S1_MR);       /*设置socket1位UDP广播模式*/
    Write_W5100(S_MR_MULTI|S_MR_UDP);
        
    Write_W5100_Address(W5100_S1_PORT);
	Write_W5100(0x1b);						/*设置socket1的端口号 */
	Write_W5100(0x58);
    
    Write_W5100_Address(W5100_S1_CR);
	Write_W5100(S_CR_OPEN);					/*打开socket1*/
    
    Write_W5100_Address(W5100_S1_SSR);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S1_CR);
		Write_W5100(S_CR_CLOSE);			/*打开不成功，关闭Socket，然后返回 */
		return FALSE;
	}
}

/****************************************************************************
                            Detect Gateway
input:  	None
Output: 	None
Return: 	if fail to detect gateway, return FALSE
		if detect the gateway, return TRUE
****************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char i;

	Write_W5100_Address(W5100_S0_MR);
	Write_W5100(S_MR_TCP);					/*设置socket0为TCP模式 */
	

    Write_W5100_Address(W5100_S0_PORT);
	Write_W5100(0x1b);					    /*设置socket0的端口号7002 */
	Write_W5100(0x5a);
	
	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_OPEN);					/*打开socket0*/

    
	Write_W5100_Address(W5100_S0_SSR);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR);
		Write_W5100(S_CR_CLOSE);			/*打开不成功，关闭Socket，然后返回 */
		return FALSE;
	}

	/*检查网关及获取网关的物理地址 */
	Write_W5100_Address(W5100_S0_DIPR);
	for(i=0;i<4;i++)
		Write_W5100(IP_Addr[i]+1);		/*向目的地址寄存器写入与本机IP不同的IP值 */

	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_CONNECT);			/*打开socket0的TCP连接 */

	OSTimeDly(20);						/* 延时20ms */

	Write_W5100_Address(W5100_S0_DHAR);
	i=Read_W5100();			/*读取目的主机的物理地址，该地址就是网关地址*/

	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_CLOSE);			/*关闭socket0*/

	if(i==0xff)
	{
		/**********没有找到网关服务器，或没有与网关服务器成功连接***********/
		/**********              自己添加处理代码                ***********/
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************
                           Socket处理, 其它3个Socket的处理可参照此程序
*****************************************************************************

						Socket初始化
如果成功则返回true, 否则返回false
-----------------------------------------------------------------------------*/
void Socket_Init(SOCKET s)
{
	unsigned int i;

	/*设置分片长度，参考W5100数据手册，该值可以不修改*/
	Write_W5100_Address(W5100_S0_MSS+s*0x100);
	Write_W5100(0x05);		/*最大分片字节数=1460*/
	Write_W5100(0xb4);
    
	/* Set Socket Port number */
	switch(s)
	{
		case 0:
			Write_W5100_Address(W5100_S0_PORT);
			Write_W5100(S0_Port[0]);			/* Set Local Socket Port number */
			Write_W5100(S0_Port[1]);

			Write_W5100_Address(W5100_S0_DPORT);
			Write_W5100(S0_DPort[0]);	/* Set Destination port number */
			Write_W5100(S0_DPort[1]);
			
			Write_W5100_Address(W5100_S0_DIPR);
			for(i=0;i<4;i++)
				Write_W5100(S0_DIP[i]);	/* Set Destination IP Address */
			break;
		case 1:
			Write_W5100_Address(W5100_S1_PORT);
			Write_W5100(S1_Port[0]);	/* Set Local Socket Port number */
			Write_W5100(S1_Port[1]);
			
			Write_W5100_Address(W5100_S1_DPORT);
			Write_W5100(S1_DPort[0]);	/* Set Destination port number */
			Write_W5100(S1_DPort[1]);

			Write_W5100_Address(W5100_S1_DIPR);
			for(i=0;i<4;i++)
				Write_W5100(S1_DIP[i]);	/* Set Destination IP Address */
			break;
		case 2:
			Write_W5100_Address(W5100_S2_PORT);
			Write_W5100(S2_Port[0]);	/* Set Local Socket Port number */
			Write_W5100(S2_Port[1]);

			Write_W5100_Address(W5100_S2_DPORT);
			Write_W5100(S2_DPort[0]);	/* Set Destination port number */
			Write_W5100(S2_DPort[1]);

			Write_W5100_Address(W5100_S2_DIPR);
			for(i=0;i<4;i++)
				Write_W5100(S2_DIP[i]);	/* Set Destination IP Address */
			break;
		case 3:
			Write_W5100_Address(W5100_S3_PORT);
			Write_W5100(S3_Port[0]);	/* Set Local Socket Port number */
			Write_W5100(S3_Port[1]);

			Write_W5100_Address(W5100_S3_DPORT);
			Write_W5100(S3_DPort[0]);	/* Set Destination port number */
			Write_W5100(S3_DPort[1]);

			Write_W5100_Address(W5100_S3_DIPR);
			for(i=0;i<4;i++)
				Write_W5100(S3_DIP[i]);	/* Set Destination IP Address */
			break;
		default:
			break;
	}
}
/*-----------------------------------------------------------------------------
                           设置Socket为客户端与远程服务器连接
当本机Socket工作在客户端模式时，引用该程序，与远程服务器建立连接
如果设置成功则返回true，否则返回false
如果启动连接后出现超时中断，则与服务器连接失败，需要重新调用该程序连接
该程序每调用一次，就与服务器产生一次连接
------------------------------------------------------------------------------*/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_TCP);						/*设置socket为TCP模式 */

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);		/*打开Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_CONNECT);		/*设置Socket为Connect模式*/

	return TRUE;

	/*至此完成了Socket的打开连接工作，至于它是否与远程服务器建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态*/
}

/*-----------------------------------------------------------------------------
                           设置Socket作为服务器等待远程主机的连接
当本机Socket工作在服务器模式时，引用该程序，等等远程主机的连接
如果设置成功则返回true, 否则返回false
该程序只调用一次，就使W5100设置为服务器模式
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_TCP);						/*设置socket为TCP模式 */

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);						/*打开Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_LISTEN);		/*设置Socket为侦听模式*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_LISTEN)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);		/*设置不成功，关闭Socket，然后返回*/
		return FALSE;
	}

	return TRUE;

	/*至此完成了Socket的打开和设置侦听工作，至于远程客户端是否与它建立连接，则需要等待Socket中断，
	以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
	在服务器侦听模式不需要设置目的IP和目的端口号*/
}

/*-----------------------------------------------------------------------------
					设置Socket为UDP模式
如果Socket工作在UDP模式，引用该程序。在UDP模式下，Socket通信不需要建立连接
如果设置成功则返回true, 否则返回false
该程序只调用一次，就使W5100设置为UDP模式
-----------------------------------------------------------------------------*/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_UDP|S_MR_MULTI);						/*设置Socket为UDP模式*/    

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);						/*打开Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_UDP)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*打开不成功，关闭Socket，然后返回*/
		return FALSE;
	}
	else
		return TRUE;

	/*至此完成了Socket的打开和UDP模式设置，在这种模式下它不需要与远程主机建立连接
	因为Socket不需要建立连接，所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	如果目的主机IP和目的Socket的端口号是固定的，在运行过程中没有改变，那么也可以在这里设置*/
}


/******************************************************************************
                              处理Socket接收和发送的数据
******************************************************************************/
/*-----------------------------------------------------------------------------
如果Socket产生接收数据的中断，则引用该程序进行处理
该程序将Socket的接收到的数据缓存到Rx_buffer数组中，并返回接收的数据字节数
-----------------------------------------------------------------------------*/
unsigned short S_rx_process(SOCKET s)
{
	unsigned short i,j;
	unsigned short rx_size,rx_offset;

	/*读取接收数据的字节数*/
	Write_W5100_Address(W5100_S0_RX_RSR+s*0x100);
	rx_size=Read_W5100();
	rx_size*=256;
	rx_size+=Read_W5100();

	/*读取接收缓冲区的偏移量 */
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	rx_offset=Read_W5100();
	rx_offset*=256;
	rx_offset+=Read_W5100();

	i=rx_offset/S_RX_SIZE;				/*计算实际的物理偏移量，S0_RX_SIZE需要在前面#define中定义*/
								/*注意S_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定*/
	rx_offset=rx_offset-i*S_RX_SIZE;

	j=W5100_RX+s*S_RX_SIZE+rx_offset;		/*实际物理地址为W5100_RX+rx_offset*/
	Write_W5100_Address(j);
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			j=W5100_RX+s*S_RX_SIZE;
			Write_W5100_Address(j);
			rx_offset=0;
		}
		Rx_Buffer[i]=Read_W5100();		/*将数据缓存到Rx_buffer数组中*/
		j++;
		rx_offset++;
	}

	/*计算下一次偏移量 */
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	rx_offset=Read_W5100();
	rx_offset*=256;
	rx_offset+=Read_W5100();

	rx_offset+=rx_size;
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	Write_W5100(rx_offset/256);
	Write_W5100(rx_offset);

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_RECV);			/*设置RECV命令，等等下一次接收*/

	return rx_size;								/*返回接收的数据字节数*/
}

unsigned char S_UDPtx_process(SOCKET s, unsigned int size)
{


}

/*-----------------------------------------------------------------------------
如果要通过Socket发送数据，则引用该程序
要发送的数据缓存在Tx_buffer中, size则是要发送的字节长度
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i,j;
	unsigned short tx_free_size,tx_offset;

	/*如果是UDP模式,可以在此设置目的主机的IP和端口号*/
	Write_W5100_Address(W5100_S0_MR+s*0x100);
    if((Read_W5100()&0x0f)==0x02)
    {
        Write_W5100_Address(W5100_S0_DIPR+s*0x100+i);
        for(i=0;i<4;i++)			/* 设置目的主机IP*/
            Write_W5100(UDP_DIPR[i]);

        Write_W5100_Address(W5100_S0_DPORT+s*0x100);
        Write_W5100(UDP_DPORT[0]);
        Write_W5100(UDP_DPORT[1]);
    }

	/*读取缓冲区剩余的长度*/
	Write_W5100_Address(W5100_S0_TX_FSR+s*0x100);
	tx_free_size=Read_W5100();
	tx_free_size*=256;
	tx_free_size+=Read_W5100();
	if(tx_free_size<size)						/*如果剩余的字节长度小于发送字节长度,则返回*/
		return FALSE;

	/*读取发送缓冲区的偏移量*/
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	tx_offset=Read_W5100();
	tx_offset*=256;
	tx_offset+=Read_W5100();

	i=tx_offset/S_TX_SIZE;					/*计算实际的物理偏移量，S0_TX_SIZE需要在前面#define中定义*/
                                            /*注意S0_TX_SIZE的值在W5100_Init()函数的W5100_TMSR中确定*/
	tx_offset=tx_offset-i*S_TX_SIZE;
	j=W5100_TX+s*S_TX_SIZE+tx_offset;			/*实际物理地址为W5100_TX+tx_offset*/
	Write_W5100_Address(j);
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)
		{
			j=W5100_TX+s*S_TX_SIZE;
			Write_W5100_Address(j);
			tx_offset=0;
		}
		Write_W5100(Tx_Buffer[i]);						/*将Tx_buffer缓冲区中的数据写入到发送缓冲区*/
		j++;
		tx_offset++;
	}

	/*计算下一次的偏移量 */
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	tx_offset=Read_W5100();
	tx_offset*=256;
	tx_offset+=Read_W5100();


	tx_offset+=size;
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	Write_W5100(tx_offset/256);
	Write_W5100(tx_offset);

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_SEND);			            /*设置SEND命令,启动发送*/
    
	return TRUE;								/*返回成功*/
}


/******************************************************************************
					W5100中断处理程序框架
******************************************************************************/
void W5100_Interrupt_Process(void)
{
	unsigned char i,j;

	W5100_Interrupt=0;

	Write_W5100_Address(W5100_IR);
	i=Read_W5100();
	Write_W5100_Address(W5100_IR);
	Write_W5100(i&0xf0);					/*回写清除中断标志*/

	if((i & IR_CONFLICT) == IR_CONFLICT)	 	/*IP地址冲突异常处理，自己添加代码*/
	{

	}

	if((i & IR_UNREACH) == IR_UNREACH)			/*UDP模式下地址无法到达异常处理，自己添加代码*/

	{
	}

	/* Socket事件处理 */
	if((i & IR_S0_INT) == IR_S0_INT)
	{
		Write_W5100_Address(W5100_S0_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S0_IR);
		Write_W5100(j);		/* 回写清中断标志 */

		if(j&S_IR_CON)				/* 在TCP模式下,Socket0成功连接 */
		{
			S0_State|=S_CONN;
		}
		if(j&S_IR_DISCON)				/* 在TCP模式下Socket断开连接处理，自己添加代码 */
		{
			Write_W5100_Address(W5100_S0_CR);
			Write_W5100(S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S0_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket0数据发送完成，可以再次启动S_tx_process()函数发送数据 */
		{
			S0_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)				/* Socket接收到数据，可以启动S_rx_process()函数 */
		{
			S0_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)			/* Socket连接或数据传输超时处理 */
		{
			Write_W5100_Address(W5100_S0_CR);
			Write_W5100(S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S0_State=0;
		}
	}

	/* Socket1事件处理 */
	if((i&IR_S1_INT)==IR_S1_INT)
	{
		Write_W5100_Address(W5100_S1_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S1_IR);
		Write_W5100(j);				/* 回写清中断标志 */

		if(j&S_IR_CON)				/* 在TCP模式下,Socket1成功连接 */
		{
			S1_State|=S_CONN;
		}
		if(j&S_IR_DISCON)				/* 在TCP模式下Socket1断开连接处理，自己添加代码 */
		{
			Write_W5100_Address(W5100_S1_CR);
			Write_W5100(S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S1_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket1数据发送完成，可以再次启动S_tx_process()函数发送数据 */
		{
			S1_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)	  			/* Socket1接收到数据，可以启动S_rx_process()函数 */
		{
			S1_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)			/* Socket1连接或数据传输超时处理 */
		{
			Write_W5100_Address(W5100_S1_CR);
			Write_W5100(S_CR_CLOSE);		/*关闭端口，等待重新打开连接 */
			S1_State=0;
		}
	}

	/* Socket2事件处理 */
	if((i&IR_S2_INT)==IR_S2_INT)
	{
		Write_W5100_Address(W5100_S2_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S2_IR);
		Write_W5100(j);		/*回写清中断标志 */

		if(j&S_IR_CON)		/* 在TCP模式下,Socket2成功连接 */
		{
			S2_State|=S_CONN;
		}
		if(j&S_IR_DISCON)		/* 在TCP模式下Socket2断开连接处理，自己添加代码 */
		{
			Write_W5100_Address(W5100_S2_CR);
			Write_W5100(S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S2_State=0;
		}
		if(j&S_IR_SENDOK)		/* Socket2数据发送完成，可以再次启动S_tx_process()函	数发送数据 */
		{
			S2_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)		/* Socket2接收到数据，可以启动S_rx_process()函数 */
		{
			S2_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)	/* Socket2连接或数据传输超时处理 */
		{
			Write_W5100_Address(W5100_S2_CR);
			Write_W5100(S_CR_CLOSE);		/*关闭端口，等待重新打开连接 */
			S2_State=0;
		}
	}

	/* Socket3事件处理 */
	if((i&IR_S3_INT)==IR_S3_INT)
	{
		Write_W5100_Address(W5100_S3_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S3_IR);
		Write_W5100(j);		/* 回写清中断标志 */

		if(j&S_IR_CON)		/* 在TCP模式下,Socket3成功连接 */
		{
			S3_State|=S_CONN;
		}
		if(j&S_IR_DISCON)		/* 在TCP模式下Socket3断开连接处理，自己添加代码 */
		{
			Write_W5100_Address(W5100_S3_CR);
			Write_W5100(S_CR_CLOSE);		/* 关闭端口，等待重新打开连接 */
			S3_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket3数据发送完成，可以再次启动S_tx_process()函数发送数据 */
		{
			S3_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)		/* Socket3接收到数据，可以启动S_rx_process()函数 */
		{
			S3_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)	/* Socket3连接或数据传输超时处理 */
		{
			Write_W5100_Address(W5100_S3_CR);
			Write_W5100(S_CR_CLOSE);		/*关闭端口，等待重新打开连接 */
			S3_State=0;
		}
	}
}
