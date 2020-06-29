#include "stm32f10x.h"         
#include "esp8266.h"
#include "led.h"

volatile uint8_t Count=0;                                                      // ���������ۼӼ���       
char Rx232buffer[Buf_Max];                                                     // ��������

/**************************************************************************************
 * ��  �� : ��ʼ��ESP8266ģ���õ�������
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void esp8266_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  //������GPIO
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE); 
  
  //���õ�IO��PB10������3��TXD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           //�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
  //���õ�IO��PB11������3��RXD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
     
  //���õ�IO��PE0��ESP8266��λ����RST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //�������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
  //��ʼ״̬��ESP8266��λ�����ø�
  ESP8266_RST_H;
}

/**************************************************************************************
 * ��  �� : ��ʼ��USART3������USART3�ж����ȼ�
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void USART3_Init(void)
{
	USART_InitTypeDef				USART_InitStructure;
	USART_ClockInitTypeDef  USART_ClockInitStructure;
	NVIC_InitTypeDef				NVIC_InitStructure;

  //�򿪴���3��ʱ��
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3 , ENABLE); 
	
  //���ô���3��Ӳ������
	USART_DeInit(USART3);                                            //������USART3�Ĵ�������Ϊȱʡֵ
	USART_InitStructure.USART_BaudRate = 115200;                      //���ô���3������Ϊ115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;      //����һ��֡�д�������λ 
	USART_InitStructure.USART_StopBits = USART_StopBits_1;           //���巢�͵�ֹͣλ��ĿΪ1
	USART_InitStructure.USART_Parity = USART_Parity_No;              //��żʧ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     //Ӳ��������ʧ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                     //���ͽ���ʹ��
	USART_Init(USART3, &USART_InitStructure);    

  USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;      //ʱ�ӵ͵�ƽ�
  USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;            //����SCLK������ʱ������ļ���Ϊ�͵�ƽ
  USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;          //ʱ�ӵ�һ�����ؽ������ݲ���
  USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;  //���һλ���ݵ�ʱ�����岻��SCLK���
  USART_ClockInit(USART3, &USART_ClockInitStructure); 
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                   //ʹ��USART3�����ж�
	USART_Cmd(USART3, ENABLE);                                       //ʹ��USART3����
	
  //���ô���3�ж����ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                  //NVIC_Group:��ռ���ȼ�2λ�������ȼ�2λ
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	               //����Ϊ����3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;          //��ռ���ȼ�Ϊ2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	             //�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  //ʹ���ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

/**************************************************************************************
 * ��  �� : USART3���ͺ���
 * ��  �� : �����ַ�
 * ����ֵ : ��
 **************************************************************************************/
void USART3_SendByte(uint8_t byte)
{
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC) != SET);
	USART_SendData(USART3, byte);
}

/**************************************************************************************
 * ��  �� : �ض���c�⺯��printf��USART3
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ�USART3 */
		USART_SendData(USART3, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

/**************************************************************************************
 * ��  �� : �ض���c�⺯��scanf��USART3
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
int fgetc(FILE *f)
{
		/* �ȴ�����3�������� */
		while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(USART3);
}

/**************************************************************************************
 * ��  �� : ���ֳɹ������
 * ��  �� : char *a���Ա��ַ���
 * ����ֵ : ��
 **************************************************************************************/
bool Hand( char *a)
{ 
  if(strstr(Rx232buffer,a)!=NULL)    //�ж�ָ��a�е��ַ����Ƿ���Rx232buffer�������ַ������Ӵ�
	   return true;
	else
		 return false;
}

/**************************************************************************************
 * ��  �� : ��ս�����������
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void CLR_Buf(void)
{
	uint8_t k;
  for(k=0;k<Buf_Max;k++)      
  {
			Rx232buffer[k] = 0;
	}      
  Count = 0;                                         //���������ۼӼ������㣬���´ν��մ�ͷ��ʼ
}

/**************************************************************************************
 * ��  �� : USART3ȫ���жϷ���
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
void USART3_IRQHandler(void)
{  
  if(USART_GetITStatus(USART3,USART_IT_RXNE)!= RESET)       //�����ж�
  {  
    Rx232buffer[Count]= USART_ReceiveData(USART3);          //ͨ������USART3��������
		Count++;                                                //�����ۼӽ�������                          
	  if(Count>Buf_Max)                                       //���������ڶ����������������ʱ����ͷ��ʼ��������
		{
			 Count = 0;
		} 
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);         //���USART3���жϴ�����λ
  }  
}

