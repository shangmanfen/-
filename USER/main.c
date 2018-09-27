#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "hc05.h"
#include "usart2.h"			 	 
#include "string.h"	 
#include "key.h"
//ALIENTEKminiSTM32��������չʵ�� 
//ATK-HC05��������ģ��ʵ��-�⺯���汾  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾ 
//ALIENTEKս��STM32������ʵ��13
//TFTLCD��ʾʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
void HC05_Progress(void);
void GetTemAndHum(void); 	
extern u8 raw_data[20],Receive_ok;	
void TIM1_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��ʹ��
	                                                                         	 //����TIM3��CH2�����PWMͨ����LED��ʾ
					
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

   //���ø�����Ϊ�����������,���TIM3 CH2��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_WriteBit(GPIOA, GPIO_Pin_7,Bit_SET); // PA7����	

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	//TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
	//TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR2�ϵ�Ԥװ�ؼĴ���
	
  //TIM_ARRPreloadConfig(TIM3, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
 
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
   
}
	//��ʾATK-HC05ģ�������״̬
void HC05_Role_Show(void)
{
//	if(HC05_Get_Role()==1)LCD_ShowString(30,140,200,16,16,"ROLE:Master");	//����
//	else LCD_ShowString(30,140,200,16,16,"ROLE:Slave ");			 		//�ӻ�
}
//��ʾATK-HC05ģ�������״̬
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)LCD_ShowString(120,10,120,16,16,"STA:Connected ");			//���ӳɹ�
	else LCD_ShowString(120,10,120,16,16,"STA:Disconnect");	 			//δ����				 
}
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_X;
  
  /* 4����ռ���ȼ���4����Ӧ���ȼ� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /*��ռ���ȼ��ɴ���жϼ���͵��ж�*/
	/*��Ӧ���ȼ����ȼ�ִ��*/
	NVIC_X.NVIC_IRQChannel = USART1_IRQn;//�ж�����
  NVIC_X.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
  NVIC_X.NVIC_IRQChannelSubPriority = 0;//��Ӧ���ȼ�
  NVIC_X.NVIC_IRQChannelCmd = ENABLE;//ʹ���ж���Ӧ
  NVIC_Init(&NVIC_X);
}
//����һ���ֽ�����
//input:byte,�����͵�����
void USART1_send_byte(uint8_t byte)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);//�ȴ��������
	USART1->DR=byte;	
}
//���Ͷ��ֽ�����
void USART_Send_bytes(uint8_t *Buffer, uint8_t Length)
{
	uint8_t i=0;
	while(i<Length)
	{
		USART1_send_byte(Buffer[i++]);
	}
}
//���Ͷ��ֽ�����+У���
void USART_Send(uint8_t *Buffer, uint8_t Length)
{
	uint8_t i=0;
	while(i<Length)
	{
		if(i<(Length-1))
		Buffer[Length-1]+=Buffer[i];//�ۼ�Length-1ǰ������
		USART1_send_byte(Buffer[i++]);
	}
}
void send_com(u8 data)
{
	u8 bytes[3]={0};
	bytes[0]=0xa5;
	bytes[1]=data;//�����ֽ�
	USART_Send(bytes,3);//����֡ͷ�������ֽڡ�У���
}
typedef struct
{
    uint32_t P;
    uint16_t Temp;
    uint16_t Hum;
    uint16_t Alt;
} bme;
u16 a,b,c,d;u8 flag[1]={0x41};
bme Bme={0,0,0,0};
int main(void)
 {
	
		delay_init();
		NVIC_Configuration();
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		LED_Init();				//��ʼ����LED���ӵ�Ӳ���ӿ�
		KEY_Init();				//��ʼ������
		uart_init(9600);
		delay_ms(100);//�ȴ�ģ���ʼ�����
		send_com(0x82);//���Ͷ���ѹ��ʪ��ָ��
		LCD_Init();	
		LCD_ShowString(10,150,200,16,16,"Temp=             degree");
		LCD_ShowString(10,180,200,16,16,"Hum=");
		while(HC05_Init()) 		//��ʼ��ATK-HC05ģ��  
		{
			LCD_ShowString(30,10,200,16,16,"ATK-HC05 Error!"); 
			delay_ms(500);
			LCD_ShowString(30,10,200,16,16,"Please Check!!!"); 
			delay_ms(100);
		}		
			LCD_ShowString(30,10,200,16,16,"WK_UP:ROLE KEY0:SEND/STOP");  
			LCD_ShowString(30,60,200,16,16,"Send:");	
			LCD_ShowString(30,80,200,16,16,"Receive:");
			POINT_COLOR=BLUE;
						
		while(1)
	{   
		HC05_Progress(); 		
		GetTemAndHum();
	}								    
}
void HC05_Progress(void){
	 u8 key;u8 sendmask=0;u8 sendcnt=0;u8 sendbuf[20];u8 reclen=0;u8 flag=5;u8 a=1;
	if(USART2_RX_STA&0X8000)			//���յ�һ��������
		{
			LCD_Fill(30,200,240,320,WHITE);	//�����ʾ
 			reclen=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
		  USART2_RX_BUF[reclen]=0;	 	//���������
			
 			LCD_ShowString(80,70,209,119,16,USART2_RX_BUF);//��ʾ���յ�������
			if(USART2_RX_BUF[0]=='A'){
				LED1=!LED1;
				sprintf((char*)sendbuf,"%.2f",(float)Bme.Temp/100);
				u2_printf("%.2f",(float)Bme.Temp/100);
				delay_ms(500);
				USART_RX_BUF[0]=9;
			}
			else if(USART2_RX_BUF[0]=='B'){
				LED0=!LED0;
				sprintf((char*)sendbuf,"%.2f",(float)Bme.Temp/100);
				u2_printf("%.2f",(float)Bme.Hum/100);
				delay_ms(500);
				USART_RX_BUF[0]=9;
			}
			else if(USART2_RX_BUF[0]=='C'){
				LED1=1;
			}
			else if(USART2_RX_BUF[0]=='D'){
				LED1=0;
			}
			else if(USART2_RX_BUF[0]=='E'){
				a=1;flag=5;
				while(a){
					flag++;
					if(flag>=20){ 
						flag=20;
						a=0;
					}
					TIM1_PWM_Init(199,7199);//(7200*200)/72000000=0.02=20ms
					TIM_SetCompare2(TIM3,flag);
					delay_ms(30);				
				}
			}
			else if(USART2_RX_BUF[0]=='F'){
				a=1;flag=20;
				while(a){
					flag--;
					if(flag<=5){ 
						flag=5;
						a=0;
					}
					TIM1_PWM_Init(199,7199);//(7200*200)/72000000=0.02=20ms
					TIM_SetCompare2(TIM3,flag);
					delay_ms(30);	
				}
			}
		}
		USART2_RX_STA=0;	
}
void GetTemAndHum(void)
{
	u8 sum=0,i=0;int16_t data=0;
	uint16_t data_16[2]={0};
	float temperInt,temperXiao,HumInt,HumXiao,t;
	int aaa,bbb,ccc,ddd;
		if(Receive_ok)//���ڽ������
		{
			for(sum=0,i=0;i<(raw_data[3]+4);i++)//rgb_data[3]=3
			sum+=raw_data[i];
			if(sum==raw_data[i])//У����ж�
			{
				Bme.Temp=(raw_data[4]<<8)|raw_data[5];
				data_16[0]=(((uint16_t)raw_data[6])<<8)|raw_data[7];
				data_16[1]=(((uint16_t)raw_data[8])<<8)|raw_data[9];
				Bme.P=(((uint32_t)data_16[0])<<16)|data_16[1];
        Bme.Hum=(raw_data[10]<<8)|raw_data[11];
        Bme.Alt=(raw_data[12]<<8)|raw_data[13]; 
				//send_3out(&raw_data[4],10,0x45);//�ϴ�����λ��
				//LCD_ShowString(30,30,200,16,16,"l am successful");
				a=(float)Bme.Temp/100;
				b=(float)Bme.P/100;
				c=(float)Bme.Hum/100;
				d=(float)Bme.Alt/100;
				aaa=(int)a;
				temperInt=(float)aaa;
				temperXiao=((float)Bme.Temp/100-temperInt)*100;
				aaa=(int)temperXiao;
				temperXiao=(float)aaa;		
				LCD_ShowxNum(100,150,temperInt,2,16,0);
				LCD_ShowxNum(125,150,temperXiao,2,16,0);
				LCD_ShowString(118,150,200,16,16,".");
				
				ccc=(int)c;
				HumInt=(float)ccc;
				HumXiao=((float)Bme.Hum/100-HumInt)*100;
				ccc=(int)HumXiao;
				HumXiao=(float)ccc;		
				LCD_ShowxNum(100,180,HumInt,2,16,0);
				LCD_ShowxNum(125,180,HumXiao,2,16,0);
				LCD_ShowString(118,180,200,16,16,".");
				
			  printf("Temp: %.2f  DegC  ",(float)Bme.Temp/100);
		    printf("  P: %.2f  Pa ",(float)Bme.P/100);
			  printf("  Hum: %.2f   ",(float)Bme.Hum/100);
		    printf("  Alt: %.2f  m\r\n ",(float)Bme.Alt);
			}
			Receive_ok=0;//����������ϱ�־
		}
}
