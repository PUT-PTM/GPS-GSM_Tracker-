#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <stdio.h>
#include <string.h>
//https://www.google.com/maps/search/09.1458483,016.5176548/
int gps_position;
short gsm_pos;
char gsm_char, gps_char, gps_message[128], time[16], latitude[16], longitude[16], in_gsm_message[128], out_gsm_message[1024];
uint16_t podglad;
void Delay(volatile uint32_t delay)
{
	delay*=24;
	while(delay--);
}
void Prepare_Message()
{
	strcpy(out_gsm_message,"Szerokosc: ");
	strcat(out_gsm_message, latitude);
	strcat(out_gsm_message, "\n");
	strcat(out_gsm_message, "Wysokosc: ");
	strcat(out_gsm_message, longitude);
	//strcat(out_gsm_message, "\n");
}
void AT_Send(volatile char *c)
{
	while(*c)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2, *c);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		Delay(50000);
		*c++;
	}
}
void Clear_Buffer()
{
	for( int i = 0; i < sizeof( in_gsm_message ); i++ )
		in_gsm_message[ i ] = 0;
	gsm_pos=0;
}
void SendPos()
{
	do
	{
		podglad=50;
		Clear_Buffer();
		AT_Send("AT+CMGL=\"REC UNREAD\"\r");
		Delay(80000);
	}while(strstr(in_gsm_message, "+CMGL:")==NULL);
	Prepare_Message();
	podglad=100;
	AT_Send("AT+CMGS=\"+48\"\r");
	AT_Send(out_gsm_message);
	Delay(80000);
	AT_Send("\032");
}
int IsDigit(int c)
{
	return ((c >= '0') && (c <= '9'));
}
void GPS_Configuration() //PIN C 10, C 11
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_Cmd(USART3, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(USART3_IRQn);
}
void SIM_Configuration() //PIN A 2, A 3
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(USART2_IRQn);
	do
	{
		podglad=20;
		Clear_Buffer();
		AT_Send("AT\r");
		Delay(800000);
	}while  (strstr(in_gsm_message, "OK")==NULL);
	do
	{
		podglad=30;
		Clear_Buffer();
		AT_Send("AT+CSCS=\"GSM\"\r");
		Delay(800000);
	}while(strstr(in_gsm_message, "OK")== NULL);
	do
	{
		podglad=40;
		Clear_Buffer();
		AT_Send("AT+CMGF=1\r");
		Delay(800000);
	}while (strstr(in_gsm_message, "OK")== NULL);

}
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
    	gps_char = USART_ReceiveData(USART3);
    	if (gps_char == '$')
    		gps_position = 0;
    	else if (gps_char == '\r')
    	{
    		if (!strncmp(gps_message, "GPGGA", 5))
    	    {
    			memset(time, 0, sizeof(time));
    	    	memset(latitude, 0, sizeof(latitude));
    	    	memset(longitude, 0, sizeof(longitude));
    	    	strncpy(time, &gps_message[6], 6);
    	    	if (IsDigit(gps_message[16]))
    	    	{
    	    		strncpy(latitude, (const char *)&gps_message[16], 10);
    	    		latitude[10] = gps_message[27];
    	    	    strncpy(longitude, (const char *)&gps_message[29], 11);
    	    	    longitude[11] = gps_message[41];
    	    	}
       	     }
       	}
      	else
      		gps_message[gps_position++] = gps_char;
    }
}
void USART2_IRQHandler(void) //TEST
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
    	gsm_char = USART_ReceiveData(USART2);
    	in_gsm_message[gsm_pos++] = gsm_char;
    }
}
int main(void)
{
	SystemInit();
	GPS_Configuration();
	SIM_Configuration();
    while(1)
    {
    	SendPos();
    }
}
