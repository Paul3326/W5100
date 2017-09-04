
#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;



unsigned char Temp_Buffer[1024];


/* 端口数据缓冲区 */
unsigned char Rx_Buffer[1024];				/* 端口接收数据缓冲区 */
unsigned char Tx_Buffer[1024];				/* 端口发送数据缓冲区 */


#if 1
unsigned char Gateway_IP[4]={192,168,60,1};	     		/* Gateway IP Address */
unsigned char Sub_Mask[4]={255,255,255,0};			 		/* Subnet Mask */
unsigned char Phy_Addr[6]={0x12,0x08,0x35,0x39,0x1f,0x55};  /* Physical Address */
unsigned char IP_Addr[4]={192,168,60,24};			 		/* Loacal IP Address */

unsigned char S0_Port[2]={0x1b,0x5a}; 		/* Socket0 Port number 7002 */
unsigned char S0_DIP[4]={192,168,60,43};	/* Socket0 Destination IP Address */
unsigned char S0_DPort[2]={0x1b,0x5a}; 		/* Socket0 Destination Port number 7002*/

unsigned char S1_Port[2]={0x1b,0x58}; 		/* Socket1 Port number 7000 */
unsigned char S1_DIP[4]={192,168,60,11};	/* Socket1 Destination IP Address */
unsigned char S1_DPort[2]={0x1b,0x58}; 		/* Socket1 Destination Port number 7000*/

unsigned char S2_Port[2]={0x1b,0x58};		/* 7000 */				/* 端口2的端口号 */
unsigned char S2_DIP[4]={192,168,200,43};;				            /* 端口2目的IP地址 */
unsigned char S2_DPort[2]={0x1b,0x58};	    /* 7000 */			    /* 端口2目的端口号 */

unsigned char S3_Port[2]={0x1f,0x40};		/* 8000 */			    /* 端口3的端口号 */
unsigned char S3_DIP[4]={192,168,200,43};;				            /* 端口3目的IP地址 */
unsigned char S3_DPort[2]={0x1f,0x40};	    /* 8000 */		        /* 端口3目的端口号 */
    
/* UDP Destionation IP address and Port number */
unsigned char UDP_DIPR[4]={192,168,60,11};
unsigned char UDP_DPORT[2]={0x1b,0x58};		/* 7000 */


#define TCP_SERVER		0x00		/* TCP服务器模式 */
#define TCP_CLIENT		0x01		/* TCP客户端模式 */
#define UDP_MODE		0x02		/* UDP模式 */
	
/* 端口的运行模式 */
unsigned char S0_Mode=TCP_SERVER;
unsigned char S1_Mode=UDP_MODE;
//unsigned char S2_Mode=TCP_SERVER;
//unsigned char S3_Mode=UDP_MODE;

//unsigned char S0_State=0;	/* Socket0 state recorder */
//unsigned char S0_Data;		/* Socket0 receive data and transmit OK */

//unsigned char S1_State=0;	/* Socket1 state recorder */
//unsigned char S1_Data;		/* Socket1 receive data and transmit OK */

unsigned char S0_State;				/* 端口0状态记录 */
unsigned char S1_State;				/* 端口1状态记录 */
unsigned char S2_State;				/* 端口2状态记录 */
unsigned char S3_State;				/* 端口3状态记录 */
	#define S_INIT	0x01				/* 端口完成初始化 */
	#define S_CONN	0x02				/* 端口完成连接，可以正常传输数据 */

unsigned char S0_Data;			/* 端口0接收和发送数据的状态 */
unsigned char S1_Data;			/* 端口1接收和发送数据的状态 */
unsigned char S2_Data;			/* 端口2接收和发送数据的状态 */
unsigned char S3_Data;			/* 端口3接收和发送数据的状态 */
	#define S_RECEIVE		0x01		/* 端口接收到一个数据包 */
	#define S_TRANSMITOK	0x02		/* 端口发送一个数据包完成 */

#endif

#if  ORIGINAL_DEFINE
/* Network parameter registers */
unsigned char Gateway_IP[4];			/* 网关IP地址 */
unsigned char Sub_Mask[4];				/* 子网掩码 */
unsigned char Phy_Addr[6];  			/* 物理地址 */
unsigned char IP_Addr[4];				/* 本机IP地址 */

unsigned char S0_Port[2];   			/* 端口0的端口号 */
unsigned char S0_DIP[4];				/* 端口0目的IP地址 */
unsigned char S0_DPort[2];				/* 端口0目的端口号 */

unsigned char S1_Port[2];   			/* 端口1的端口号 */
unsigned char S1_DIP[4];   				/* 端口1目的IP地址 */
unsigned char S1_DPort[2];				/* 端口1目的端口号 */

unsigned char S2_Port[2];				/* 端口2的端口号 */
unsigned char S2_DIP[4];				/* 端口2目的IP地址 */
unsigned char S2_DPort[2];				/* 端口2目的端口号 */

unsigned char S3_Port[2];				/* 端口3的端口号 */
unsigned char S3_DIP[4];				/* 端口3目的IP地址 */
unsigned char S3_DPort[2];				/* 端口3目的端口号 */

/* 端口的运行模式 */
unsigned char S0_Mode;
unsigned char S1_Mode;
unsigned char S2_Mode;
unsigned char S3_Mode;
	#define TCP_SERVER		0x00		/* TCP服务器模式 */
	#define TCP_CLIENT		0x01		/* TCP客户端模式 */
	#define UDP_MODE		0x02		/* UDP模式 */

unsigned char S0_State;				/* 端口0状态记录 */
unsigned char S1_State;				/* 端口1状态记录 */
unsigned char S2_State;				/* 端口2状态记录 */
unsigned char S3_State;				/* 端口3状态记录 */
	#define S_INIT	0x01				/* 端口完成初始化 */
	#define S_CONN	0x02				/* 端口完成连接，可以正常传输数据 */

unsigned char S0_Data;			/* 端口0接收和发送数据的状态 */
unsigned char S1_Data;			/* 端口1接收和发送数据的状态 */
unsigned char S2_Data;			/* 端口2接收和发送数据的状态 */
unsigned char S3_Data;			/* 端口3接收和发送数据的状态 */
	#define S_RECEIVE		0x01		/* 端口接收到一个数据包 */
	#define S_TRANSMITOK	0x02		/* 端口发送一个数据包完成 */

#endif


unsigned char W5100_Interrupt;


extern void W5100_Init(void);
extern unsigned char Detect_Gateway(void);
extern unsigned char Detect_UDP_Broadcating(void);
extern void Socket_Init(SOCKET s);
unsigned char Read_W5100(void);
extern unsigned char Socket_Connect(SOCKET s);
extern unsigned char Socket_Listen(SOCKET s);
extern unsigned char Socket_UDP(SOCKET s);
extern unsigned short S_rx_process(SOCKET s);
extern unsigned char S_tx_process(SOCKET s, unsigned int size);
extern void W5100_Interrupt_Process(void);
extern void Write_W5100_Address(unsigned short addr);
