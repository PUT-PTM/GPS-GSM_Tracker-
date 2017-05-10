#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <stdio.h>
#include <string.h>
//https://www.google.com/maps/search/09.1458483,016.5176548/
int position;
short number_size;
short pos;
char gsm_char, gps_char, gps_message[128], time[16], latitude[16], longitude[16], in_gsm_message[128], number[32];
uint16_t podglad;
bool sim_ready=false;
void SendOrder(char order[128],int size)
{
	for(int i = 0; i<size; i++)
	{
	 	USART_SendData(USART2, order[i]);
	}
	USART_SendData(USART2, '\r');
}
void SendPos()
{
	char out_message[16]="\nlatitude: ";
	for(int i = 0; i<11; i++)
	{
	 	USART_SendData(USART2, out_message[i]);
	}
	for(int i = 0; i<16; i++)
	{
	 	USART_SendData(USART2, latitude[i]);
	}
	char out_message[256]="\nlongitude: ";
	for(int i = 0; i<12; i++)
	{
	 	USART_SendData(USART2, out_message[i]);
	}
	for(int i = 0; i<16; i++)
	{
	 	USART_SendData(USART2, longitude[i]);
	}
	char out_message[256]="\ntime: ";
	for(int i = 0; i<7; i++)
	{
	 	USART_SendData(USART2, out_message[i]);
	}
	for(int i = 0; i<16; i++)
	{
	 	USART_SendData(USART2, time[i]);
	}
	USART_SendData(USART2, 26);
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
	USART_InitStructure.USART_BaudRate = 9600;
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
	while(sim_ready==false)
	{
		SendOrder("AT+CPIN?",9);
		Sleep(300);
	}
	SendOrder("AT+CMGF=1",9);
	SendOrder("AT+CSCS=\"GSM\"",13);
}
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
    	gps_char = USART_ReceiveData(USART3);
    	if (gps_char == '$')
    		position = 0;
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
      		gps_message[position++] = gps_char;
    }
}
void USART2_IRQHandler(void) //TEST
{
    // sprawdzenie flagi zwiazanej z odebraniem danych przez USART
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // odebrany bajt znajduje sie w rejestrze USART3->DR
    	gsm_char = USART_ReceiveData(USART2);
    	if (gsm_char == '\r')
    	{
    		pos=0;
    		if(strncmp(in_gsm_message, "+CPIN: READY",11))
    		   {
    		    	sim_ready=true;
    		   }
    		if(strncmp(in_gsm_message, "+CMTI: \"SM\"",11))
    		{
    			SendOrder("AT+CMGL=\"REC UNREAD\"",20);
    		}
    		if(strncmp(in_gsm_message, "+CMGR: \"REC UNREAD\",\"",21))
    		{
    			number_size=0;
    			short number_it=21;
    			while(in_gsm_message[number_it]!='\"')
    			{
    				number[number_size]=in_gsm_message[number_it];
    				number_size++;
    				number_it++;
    			}
    			SendOrder("AT+SMGS=\"" + number + "\"", 10 + number_size);
    			SendPos();
    			SendOrder("AT+CMGDA=\"DEL UNREAD\"", 10 + number_size);

    		}
    	}
    	else
    		in_gsm_message[pos++] = gsm_char;
    	//czekaj na opróżnienie bufora wyjściowego
    	//while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    	// wyslanie danych
    	//USART_SendData(USART2, byte);
    	// czekaj az dane zostana wyslane
    	//while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    }
}
int main(void)
{
	SystemInit();
	SIM_Configuration();
	GPS_Configuration();

    while(1)
    {
    }
}
