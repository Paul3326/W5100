/********************************************************************************
	�ṩ�̣��ɶ���Ȼ�������޹�˾
	��  ַ��http://www.hschip.com

    ʱ  ��: 2007-11-30

    ���������5�����֣�
    	1. W5100��ʼ��
    	2. W5100��Socket��ʼ��
    	3. Socket����
    	   ���Socket����ΪTCP������ģʽ�������Socket_Listen()����,W5100��������״̬��ֱ��Զ�̿ͻ����������ӡ�
    	   ���Socket����ΪTCP�ͻ���ģʽ�������Socket_Connect()������
    	                                  ÿ����һ��Socket_Connect(s)����������һ�����ӣ�
    	                                  ������Ӳ��ɹ����������ʱ�жϣ�Ȼ������ٵ��øú����������ӡ�
    	   ���Socket����ΪUDPģʽ,�����Socket_UDP����
    	4. Socket���ݽ��պͷ���
    	5. W5100�жϴ���

    ��W5100Ϊ������ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)�����ù��̼���ɣ��ȴ��ͻ��˵����ӡ�
    ��W5100Ϊ�ͻ���ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)�����ù��̼���ɣ�����Զ�̷��������ӡ�
    ��W5100ΪUDPģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)�����ù��̼���ɣ�������Զ������UDPͨ�š�

    W5100���������ӳɹ�����ֹ���ӡ��������ݡ��������ݡ���ʱ���¼��������Դ��ж�״̬�л�á�
********************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ucos_ii.h"

#include "defines.h"
#include "global.h"
#include "macro_def.h"

#include "stm32f4xx.h"
#include"W5100.h"					/* ����W5100�ļĴ�����ַ��״̬ */

#define TRUE	0xff
#define FALSE	0x00

typedef unsigned char SOCKET;

/* �˿����ݻ����� */
extern unsigned char Rx_Buffer[1460];			/* �˿ڽ������ݻ����� */
extern unsigned char Tx_Buffer[1460];			/* �˿ڷ������ݻ����� */

/* ��������Ĵ��� */
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

//    p.FSMC_AddressSetupTime = 0x2;	 //��ַ����ʱ��
//    p.FSMC_AddressHoldTime = 0x0;	 //��ַ����ʱ��
//    p.FSMC_DataSetupTime = 0x2;		 //���ݽ���ʱ��
//    p.FSMC_BusTurnAroundDuration = 0x0;
//    p.FSMC_CLKDivision = 10;
//    p.FSMC_DataLatency = 0x0;
//    p.FSMC_AccessMode = FSMC_AccessMode_B;	 // һ��ʹ��ģʽB������LCD
    
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
    
    /* ʹ�� FSMC Bank1_SRAM Bank */
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

	/* ���õ�ַΪ0x01 */
	GPIO_SetBits(GPIOA, W5100_A0);
	GPIO_ResetBits(GPIOA, W5100_A1);

	/* �����ֵַ */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (addr & 0xff00);
	GPIO_Write(GPIOB, i);

	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOA, W5100_CS);
	GPIO_ResetBits(GPIOA, W5100_WR);
	/*д���ַ��8λ */
	GPIO_SetBits(GPIOA, W5100_WR);
	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOA, W5100_CS);


	/* ���õ�ַΪ0x02 */
	GPIO_ResetBits(GPIOA, W5100_A0);
	GPIO_SetBits(GPIOA, W5100_A1);

	/* �����ֵַ */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (addr<<8);
	GPIO_Write(GPIOB, i);

	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOA, W5100_CS);
	GPIO_ResetBits(GPIOA, W5100_WR);
	/*д���ַ��8λ */
	GPIO_SetBits(GPIOA, W5100_WR);
	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOA, W5100_CS);
	
	/* ���õ�ַΪ0x11 */
	GPIO_SetBits(GPIOA, W5100_A0);
#endif	
}

/*****************************************************************
��������Read_W5100
����: ��ַ
���: ��
����: ��ȡ������
˵������W5100ָ���ĵ�ַ��ȡһ���ֽ�
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

	/* �������ݶ˿�Ϊ����״̬ */
	GPIO_InitStructure.GPIO_Pin  = W5100_DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOA, W5100_CS);
	/* ��ȡ���� */
	GPIO_ResetBits(GPIOA, W5100_RD);
	GPIO_SetBits(GPIOA, W5100_RD);
	i=GPIO_ReadInputData(GPIOB);
	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOA, W5100_CS);

	/* ����Ϊ���״̬ */
	GPIO_InitStructure.GPIO_Pin  = W5100_DATA;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	i >>= 8;
	return i;
#endif	
}

/*****************************************************************
��������Write_W5100
����: ��ַ���ֽ�����
���: ��
����: ��
˵������һ���ֽ�д��W5100ָ���ĵ�ַ
*****************************************************************/
void Write_W5100(unsigned char dat)
{
#if 1
    *(u8*)(W5100_STADDR+3) = dat;
#else
	unsigned short i;

	/* ������ݵ��˿� */
	i=GPIO_ReadOutputData(GPIOB);
	i &= 0x00ff;
	i = i | (dat<<8);
	GPIO_Write(GPIOB, i);

	/* ��W5100��CSΪ�͵�ƽ */
	GPIO_ResetBits(GPIOA, W5100_CS);
	/* д���� */
	GPIO_ResetBits(GPIOA, W5100_WR);
	GPIO_SetBits(GPIOA, W5100_WR);
	/* ��W5100��CSΪ�ߵ�ƽ */
	GPIO_SetBits(GPIOA, W5100_CS);
#endif	
}

/*------------------------------------------------------------------------------
						W5100��ʼ������
��ʹ��W5100֮ǰ����W5100��ʼ��
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

	/*��������(Gateway)��IP��ַ��4�ֽ� */
	/*ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet*/
	Write_W5100_Address(W5100_GAR);
	for(i=0;i<4;i++)
		Write_W5100(Gateway_IP[i]);			/*Gateway_IPΪ4�ֽ�unsigned char����,�Լ�����*/


	/*������������(MASK)ֵ��4�ֽڡ���������������������*/
	Write_W5100_Address(W5100_SUBR);
	for(i=0;i<4;i++)
		Write_W5100(Sub_Mask[i]);			/*SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����*/

	/*���������ַ��6�ֽڣ�����Ψһ��ʶ�����豸�������ֵַ
	�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	����Լ����������ַ��ע���һ���ֽڱ���Ϊż��*/
	Write_W5100_Address(W5100_SHAR);
	for(i=0;i<6;i++)
		Write_W5100(Phy_Addr[i]);			/*PHY_ADDR6�ֽ�unsigned char����,�Լ�����*/
      
		
	/*���ñ�����IP��ַ��4���ֽ�
	ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����*/
	Write_W5100_Address(W5100_SIPR);
	for(i=0;i<4;i++)
		Write_W5100(IP_Addr[i]);			/*IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����*/

	/*���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5100�����ֲ�*/
	Write_W5100_Address(W5100_RMSR);
	Write_W5100(0x55);			/*Socket Rx memory size=2k*/
	Write_W5100_Address(W5100_TMSR);
	Write_W5100(0x55);		/*Socket Tx mempry size=2k*/

	/* ��������ʱ�䣬Ĭ��Ϊ2000(200ms) */
	Write_W5100_Address(W5100_RTR);
	Write_W5100(0x07);
	Write_W5100(0xd0);
    

	/* �������Դ�����Ĭ��Ϊ8�� */
	Write_W5100_Address(W5100_RCR);
	Write_W5100(8);

	/* �����жϣ��ο�W5100�����ֲ�ȷ���Լ���Ҫ���ж�����
	IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�
	IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	������Socket�¼��жϣ�������Ҫ��� */
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
        
    Write_W5100_Address(W5100_S1_MR);       /*����socket1λUDP�㲥ģʽ*/
    Write_W5100(S_MR_MULTI|S_MR_UDP);
        
    Write_W5100_Address(W5100_S1_PORT);
	Write_W5100(0x1b);						/*����socket1�Ķ˿ں� */
	Write_W5100(0x58);
    
    Write_W5100_Address(W5100_S1_CR);
	Write_W5100(S_CR_OPEN);					/*��socket1*/
    
    Write_W5100_Address(W5100_S1_SSR);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S1_CR);
		Write_W5100(S_CR_CLOSE);			/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻� */
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
	Write_W5100(S_MR_TCP);					/*����socket0ΪTCPģʽ */
	

    Write_W5100_Address(W5100_S0_PORT);
	Write_W5100(0x1b);					    /*����socket0�Ķ˿ں�7002 */
	Write_W5100(0x5a);
	
	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_OPEN);					/*��socket0*/

    
	Write_W5100_Address(W5100_S0_SSR);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR);
		Write_W5100(S_CR_CLOSE);			/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻� */
		return FALSE;
	}

	/*������ؼ���ȡ���ص������ַ */
	Write_W5100_Address(W5100_S0_DIPR);
	for(i=0;i<4;i++)
		Write_W5100(IP_Addr[i]+1);		/*��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ */

	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_CONNECT);			/*��socket0��TCP���� */

	OSTimeDly(20);						/* ��ʱ20ms */

	Write_W5100_Address(W5100_S0_DHAR);
	i=Read_W5100();			/*��ȡĿ�������������ַ���õ�ַ�������ص�ַ*/

	Write_W5100_Address(W5100_S0_CR);
	Write_W5100(S_CR_CLOSE);			/*�ر�socket0*/

	if(i==0xff)
	{
		/**********û���ҵ����ط���������û�������ط������ɹ�����***********/
		/**********              �Լ���Ӵ������                ***********/
		return FALSE;
	}
	return TRUE;
}

/******************************************************************************
                           Socket����, ����3��Socket�Ĵ���ɲ��մ˳���
*****************************************************************************

						Socket��ʼ��
����ɹ��򷵻�true, ���򷵻�false
-----------------------------------------------------------------------------*/
void Socket_Init(SOCKET s)
{
	unsigned int i;

	/*���÷�Ƭ���ȣ��ο�W5100�����ֲᣬ��ֵ���Բ��޸�*/
	Write_W5100_Address(W5100_S0_MSS+s*0x100);
	Write_W5100(0x05);		/*����Ƭ�ֽ���=1460*/
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
                           ����SocketΪ�ͻ�����Զ�̷���������
������Socket�����ڿͻ���ģʽʱ�����øó�����Զ�̷�������������
������óɹ��򷵻�true�����򷵻�false
����������Ӻ���ֳ�ʱ�жϣ��������������ʧ�ܣ���Ҫ���µ��øó�������
�ó���ÿ����һ�Σ��������������һ������
------------------------------------------------------------------------------*/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_TCP);						/*����socketΪTCPģʽ */

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);		/*��Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_CONNECT);		/*����SocketΪConnectģʽ*/

	return TRUE;

	/*���������Socket�Ĵ����ӹ������������Ƿ���Զ�̷������������ӣ�����Ҫ�ȴ�Socket�жϣ�
	���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬*/
}

/*-----------------------------------------------------------------------------
                           ����Socket��Ϊ�������ȴ�Զ������������
������Socket�����ڷ�����ģʽʱ�����øó��򣬵ȵ�Զ������������
������óɹ��򷵻�true, ���򷵻�false
�ó���ֻ����һ�Σ���ʹW5100����Ϊ������ģʽ
-----------------------------------------------------------------------------*/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_TCP);						/*����socketΪTCPģʽ */

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);						/*��Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_INIT)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_LISTEN);		/*����SocketΪ����ģʽ*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_LISTEN)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);		/*���ò��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}

	return TRUE;

	/*���������Socket�Ĵ򿪺�������������������Զ�̿ͻ����Ƿ������������ӣ�����Ҫ�ȴ�Socket�жϣ�
	���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬
	�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�*/
}

/*-----------------------------------------------------------------------------
					����SocketΪUDPģʽ
���Socket������UDPģʽ�����øó�����UDPģʽ�£�Socketͨ�Ų���Ҫ��������
������óɹ��򷵻�true, ���򷵻�false
�ó���ֻ����һ�Σ���ʹW5100����ΪUDPģʽ
-----------------------------------------------------------------------------*/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5100_Address(W5100_S0_MR+s*0x100);
	Write_W5100(S_MR_UDP|S_MR_MULTI);						/*����SocketΪUDPģʽ*/    

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_OPEN);						/*��Socket*/

	Write_W5100_Address(W5100_S0_SSR+s*0x100);
	if(Read_W5100()!=S_SSR_UDP)
	{
		Write_W5100_Address(W5100_S0_CR+s*0x100);
		Write_W5100(S_CR_CLOSE);	/*�򿪲��ɹ����ر�Socket��Ȼ�󷵻�*/
		return FALSE;
	}
	else
		return TRUE;

	/*���������Socket�Ĵ򿪺�UDPģʽ���ã�������ģʽ��������Ҫ��Զ��������������
	��ΪSocket����Ҫ�������ӣ������ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶��ģ������й�����û�иı䣬��ôҲ��������������*/
}


/******************************************************************************
                              ����Socket���պͷ��͵�����
******************************************************************************/
/*-----------------------------------------------------------------------------
���Socket�����������ݵ��жϣ������øó�����д���
�ó���Socket�Ľ��յ������ݻ��浽Rx_buffer�����У������ؽ��յ������ֽ���
-----------------------------------------------------------------------------*/
unsigned short S_rx_process(SOCKET s)
{
	unsigned short i,j;
	unsigned short rx_size,rx_offset;

	/*��ȡ�������ݵ��ֽ���*/
	Write_W5100_Address(W5100_S0_RX_RSR+s*0x100);
	rx_size=Read_W5100();
	rx_size*=256;
	rx_size+=Read_W5100();

	/*��ȡ���ջ�������ƫ���� */
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	rx_offset=Read_W5100();
	rx_offset*=256;
	rx_offset+=Read_W5100();

	i=rx_offset/S_RX_SIZE;				/*����ʵ�ʵ�����ƫ������S0_RX_SIZE��Ҫ��ǰ��#define�ж���*/
								/*ע��S_RX_SIZE��ֵ��W5100_Init()������W5100_RMSR��ȷ��*/
	rx_offset=rx_offset-i*S_RX_SIZE;

	j=W5100_RX+s*S_RX_SIZE+rx_offset;		/*ʵ�������ַΪW5100_RX+rx_offset*/
	Write_W5100_Address(j);
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			j=W5100_RX+s*S_RX_SIZE;
			Write_W5100_Address(j);
			rx_offset=0;
		}
		Rx_Buffer[i]=Read_W5100();		/*�����ݻ��浽Rx_buffer������*/
		j++;
		rx_offset++;
	}

	/*������һ��ƫ���� */
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	rx_offset=Read_W5100();
	rx_offset*=256;
	rx_offset+=Read_W5100();

	rx_offset+=rx_size;
	Write_W5100_Address(W5100_S0_RX_RR+s*0x100);
	Write_W5100(rx_offset/256);
	Write_W5100(rx_offset);

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_RECV);			/*����RECV����ȵ���һ�ν���*/

	return rx_size;								/*���ؽ��յ������ֽ���*/
}

unsigned char S_UDPtx_process(SOCKET s, unsigned int size)
{


}

/*-----------------------------------------------------------------------------
���Ҫͨ��Socket�������ݣ������øó���
Ҫ���͵����ݻ�����Tx_buffer��, size����Ҫ���͵��ֽڳ���
-----------------------------------------------------------------------------*/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i,j;
	unsigned short tx_free_size,tx_offset;

	/*�����UDPģʽ,�����ڴ�����Ŀ��������IP�Ͷ˿ں�*/
	Write_W5100_Address(W5100_S0_MR+s*0x100);
    if((Read_W5100()&0x0f)==0x02)
    {
        Write_W5100_Address(W5100_S0_DIPR+s*0x100+i);
        for(i=0;i<4;i++)			/* ����Ŀ������IP*/
            Write_W5100(UDP_DIPR[i]);

        Write_W5100_Address(W5100_S0_DPORT+s*0x100);
        Write_W5100(UDP_DPORT[0]);
        Write_W5100(UDP_DPORT[1]);
    }

	/*��ȡ������ʣ��ĳ���*/
	Write_W5100_Address(W5100_S0_TX_FSR+s*0x100);
	tx_free_size=Read_W5100();
	tx_free_size*=256;
	tx_free_size+=Read_W5100();
	if(tx_free_size<size)						/*���ʣ����ֽڳ���С�ڷ����ֽڳ���,�򷵻�*/
		return FALSE;

	/*��ȡ���ͻ�������ƫ����*/
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	tx_offset=Read_W5100();
	tx_offset*=256;
	tx_offset+=Read_W5100();

	i=tx_offset/S_TX_SIZE;					/*����ʵ�ʵ�����ƫ������S0_TX_SIZE��Ҫ��ǰ��#define�ж���*/
                                            /*ע��S0_TX_SIZE��ֵ��W5100_Init()������W5100_TMSR��ȷ��*/
	tx_offset=tx_offset-i*S_TX_SIZE;
	j=W5100_TX+s*S_TX_SIZE+tx_offset;			/*ʵ�������ַΪW5100_TX+tx_offset*/
	Write_W5100_Address(j);
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)
		{
			j=W5100_TX+s*S_TX_SIZE;
			Write_W5100_Address(j);
			tx_offset=0;
		}
		Write_W5100(Tx_Buffer[i]);						/*��Tx_buffer�������е�����д�뵽���ͻ�����*/
		j++;
		tx_offset++;
	}

	/*������һ�ε�ƫ���� */
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	tx_offset=Read_W5100();
	tx_offset*=256;
	tx_offset+=Read_W5100();


	tx_offset+=size;
	Write_W5100_Address(W5100_S0_TX_WR+s*0x100);
	Write_W5100(tx_offset/256);
	Write_W5100(tx_offset);

	Write_W5100_Address(W5100_S0_CR+s*0x100);
	Write_W5100(S_CR_SEND);			            /*����SEND����,��������*/
    
	return TRUE;								/*���سɹ�*/
}


/******************************************************************************
					W5100�жϴ��������
******************************************************************************/
void W5100_Interrupt_Process(void)
{
	unsigned char i,j;

	W5100_Interrupt=0;

	Write_W5100_Address(W5100_IR);
	i=Read_W5100();
	Write_W5100_Address(W5100_IR);
	Write_W5100(i&0xf0);					/*��д����жϱ�־*/

	if((i & IR_CONFLICT) == IR_CONFLICT)	 	/*IP��ַ��ͻ�쳣�����Լ���Ӵ���*/
	{

	}

	if((i & IR_UNREACH) == IR_UNREACH)			/*UDPģʽ�µ�ַ�޷������쳣�����Լ���Ӵ���*/

	{
	}

	/* Socket�¼����� */
	if((i & IR_S0_INT) == IR_S0_INT)
	{
		Write_W5100_Address(W5100_S0_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S0_IR);
		Write_W5100(j);		/* ��д���жϱ�־ */

		if(j&S_IR_CON)				/* ��TCPģʽ��,Socket0�ɹ����� */
		{
			S0_State|=S_CONN;
		}
		if(j&S_IR_DISCON)				/* ��TCPģʽ��Socket�Ͽ����Ӵ����Լ���Ӵ��� */
		{
			Write_W5100_Address(W5100_S0_CR);
			Write_W5100(S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S0_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket0���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
		{
			S0_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)				/* Socket���յ����ݣ���������S_rx_process()���� */
		{
			S0_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)			/* Socket���ӻ����ݴ��䳬ʱ���� */
		{
			Write_W5100_Address(W5100_S0_CR);
			Write_W5100(S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S0_State=0;
		}
	}

	/* Socket1�¼����� */
	if((i&IR_S1_INT)==IR_S1_INT)
	{
		Write_W5100_Address(W5100_S1_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S1_IR);
		Write_W5100(j);				/* ��д���жϱ�־ */

		if(j&S_IR_CON)				/* ��TCPģʽ��,Socket1�ɹ����� */
		{
			S1_State|=S_CONN;
		}
		if(j&S_IR_DISCON)				/* ��TCPģʽ��Socket1�Ͽ����Ӵ����Լ���Ӵ��� */
		{
			Write_W5100_Address(W5100_S1_CR);
			Write_W5100(S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S1_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket1���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
		{
			S1_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)	  			/* Socket1���յ����ݣ���������S_rx_process()���� */
		{
			S1_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)			/* Socket1���ӻ����ݴ��䳬ʱ���� */
		{
			Write_W5100_Address(W5100_S1_CR);
			Write_W5100(S_CR_CLOSE);		/*�رն˿ڣ��ȴ����´����� */
			S1_State=0;
		}
	}

	/* Socket2�¼����� */
	if((i&IR_S2_INT)==IR_S2_INT)
	{
		Write_W5100_Address(W5100_S2_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S2_IR);
		Write_W5100(j);		/*��д���жϱ�־ */

		if(j&S_IR_CON)		/* ��TCPģʽ��,Socket2�ɹ����� */
		{
			S2_State|=S_CONN;
		}
		if(j&S_IR_DISCON)		/* ��TCPģʽ��Socket2�Ͽ����Ӵ����Լ���Ӵ��� */
		{
			Write_W5100_Address(W5100_S2_CR);
			Write_W5100(S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S2_State=0;
		}
		if(j&S_IR_SENDOK)		/* Socket2���ݷ�����ɣ������ٴ�����S_tx_process()��	���������� */
		{
			S2_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)		/* Socket2���յ����ݣ���������S_rx_process()���� */
		{
			S2_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)	/* Socket2���ӻ����ݴ��䳬ʱ���� */
		{
			Write_W5100_Address(W5100_S2_CR);
			Write_W5100(S_CR_CLOSE);		/*�رն˿ڣ��ȴ����´����� */
			S2_State=0;
		}
	}

	/* Socket3�¼����� */
	if((i&IR_S3_INT)==IR_S3_INT)
	{
		Write_W5100_Address(W5100_S3_IR);
		j=Read_W5100();
		Write_W5100_Address(W5100_S3_IR);
		Write_W5100(j);		/* ��д���жϱ�־ */

		if(j&S_IR_CON)		/* ��TCPģʽ��,Socket3�ɹ����� */
		{
			S3_State|=S_CONN;
		}
		if(j&S_IR_DISCON)		/* ��TCPģʽ��Socket3�Ͽ����Ӵ����Լ���Ӵ��� */
		{
			Write_W5100_Address(W5100_S3_CR);
			Write_W5100(S_CR_CLOSE);		/* �رն˿ڣ��ȴ����´����� */
			S3_State=0;
		}
		if(j&S_IR_SENDOK)				/* Socket3���ݷ�����ɣ������ٴ�����S_tx_process()������������ */
		{
			S3_Data|=S_TRANSMITOK;
		}
		if(j&S_IR_RECV)		/* Socket3���յ����ݣ���������S_rx_process()���� */
		{
			S3_Data|=S_RECEIVE;
		}
		if(j&S_IR_TIMEOUT)	/* Socket3���ӻ����ݴ��䳬ʱ���� */
		{
			Write_W5100_Address(W5100_S3_CR);
			Write_W5100(S_CR_CLOSE);		/*�رն˿ڣ��ȴ����´����� */
			S3_State=0;
		}
	}
}
