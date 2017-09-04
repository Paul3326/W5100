
#define TRUE	0xff
#define FALSE	0x00

typedef  unsigned char SOCKET;



unsigned char Temp_Buffer[1024];


/* �˿����ݻ����� */
unsigned char Rx_Buffer[1024];				/* �˿ڽ������ݻ����� */
unsigned char Tx_Buffer[1024];				/* �˿ڷ������ݻ����� */


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

unsigned char S2_Port[2]={0x1b,0x58};		/* 7000 */				/* �˿�2�Ķ˿ں� */
unsigned char S2_DIP[4]={192,168,200,43};;				            /* �˿�2Ŀ��IP��ַ */
unsigned char S2_DPort[2]={0x1b,0x58};	    /* 7000 */			    /* �˿�2Ŀ�Ķ˿ں� */

unsigned char S3_Port[2]={0x1f,0x40};		/* 8000 */			    /* �˿�3�Ķ˿ں� */
unsigned char S3_DIP[4]={192,168,200,43};;				            /* �˿�3Ŀ��IP��ַ */
unsigned char S3_DPort[2]={0x1f,0x40};	    /* 8000 */		        /* �˿�3Ŀ�Ķ˿ں� */
    
/* UDP Destionation IP address and Port number */
unsigned char UDP_DIPR[4]={192,168,60,11};
unsigned char UDP_DPORT[2]={0x1b,0x58};		/* 7000 */


#define TCP_SERVER		0x00		/* TCP������ģʽ */
#define TCP_CLIENT		0x01		/* TCP�ͻ���ģʽ */
#define UDP_MODE		0x02		/* UDPģʽ */
	
/* �˿ڵ�����ģʽ */
unsigned char S0_Mode=TCP_SERVER;
unsigned char S1_Mode=UDP_MODE;
//unsigned char S2_Mode=TCP_SERVER;
//unsigned char S3_Mode=UDP_MODE;

//unsigned char S0_State=0;	/* Socket0 state recorder */
//unsigned char S0_Data;		/* Socket0 receive data and transmit OK */

//unsigned char S1_State=0;	/* Socket1 state recorder */
//unsigned char S1_Data;		/* Socket1 receive data and transmit OK */

unsigned char S0_State;				/* �˿�0״̬��¼ */
unsigned char S1_State;				/* �˿�1״̬��¼ */
unsigned char S2_State;				/* �˿�2״̬��¼ */
unsigned char S3_State;				/* �˿�3״̬��¼ */
	#define S_INIT	0x01				/* �˿���ɳ�ʼ�� */
	#define S_CONN	0x02				/* �˿�������ӣ����������������� */

unsigned char S0_Data;			/* �˿�0���պͷ������ݵ�״̬ */
unsigned char S1_Data;			/* �˿�1���պͷ������ݵ�״̬ */
unsigned char S2_Data;			/* �˿�2���պͷ������ݵ�״̬ */
unsigned char S3_Data;			/* �˿�3���պͷ������ݵ�״̬ */
	#define S_RECEIVE		0x01		/* �˿ڽ��յ�һ�����ݰ� */
	#define S_TRANSMITOK	0x02		/* �˿ڷ���һ�����ݰ���� */

#endif

#if  ORIGINAL_DEFINE
/* Network parameter registers */
unsigned char Gateway_IP[4];			/* ����IP��ַ */
unsigned char Sub_Mask[4];				/* �������� */
unsigned char Phy_Addr[6];  			/* �����ַ */
unsigned char IP_Addr[4];				/* ����IP��ַ */

unsigned char S0_Port[2];   			/* �˿�0�Ķ˿ں� */
unsigned char S0_DIP[4];				/* �˿�0Ŀ��IP��ַ */
unsigned char S0_DPort[2];				/* �˿�0Ŀ�Ķ˿ں� */

unsigned char S1_Port[2];   			/* �˿�1�Ķ˿ں� */
unsigned char S1_DIP[4];   				/* �˿�1Ŀ��IP��ַ */
unsigned char S1_DPort[2];				/* �˿�1Ŀ�Ķ˿ں� */

unsigned char S2_Port[2];				/* �˿�2�Ķ˿ں� */
unsigned char S2_DIP[4];				/* �˿�2Ŀ��IP��ַ */
unsigned char S2_DPort[2];				/* �˿�2Ŀ�Ķ˿ں� */

unsigned char S3_Port[2];				/* �˿�3�Ķ˿ں� */
unsigned char S3_DIP[4];				/* �˿�3Ŀ��IP��ַ */
unsigned char S3_DPort[2];				/* �˿�3Ŀ�Ķ˿ں� */

/* �˿ڵ�����ģʽ */
unsigned char S0_Mode;
unsigned char S1_Mode;
unsigned char S2_Mode;
unsigned char S3_Mode;
	#define TCP_SERVER		0x00		/* TCP������ģʽ */
	#define TCP_CLIENT		0x01		/* TCP�ͻ���ģʽ */
	#define UDP_MODE		0x02		/* UDPģʽ */

unsigned char S0_State;				/* �˿�0״̬��¼ */
unsigned char S1_State;				/* �˿�1״̬��¼ */
unsigned char S2_State;				/* �˿�2״̬��¼ */
unsigned char S3_State;				/* �˿�3״̬��¼ */
	#define S_INIT	0x01				/* �˿���ɳ�ʼ�� */
	#define S_CONN	0x02				/* �˿�������ӣ����������������� */

unsigned char S0_Data;			/* �˿�0���պͷ������ݵ�״̬ */
unsigned char S1_Data;			/* �˿�1���պͷ������ݵ�״̬ */
unsigned char S2_Data;			/* �˿�2���պͷ������ݵ�״̬ */
unsigned char S3_Data;			/* �˿�3���պͷ������ݵ�״̬ */
	#define S_RECEIVE		0x01		/* �˿ڽ��յ�һ�����ݰ� */
	#define S_TRANSMITOK	0x02		/* �˿ڷ���һ�����ݰ���� */

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
