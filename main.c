#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

const unsigned char header[] = {0xB5, 0x62};
struct Nav_Posllh
{
	unsigned char header;
	unsigned char id;
	unsigned short len;
	unsigned long iTDW;
	long lon;
	long lat;
	long height;
	long hMSL;
	unsigned long hAcc;
	unsigned long yAcc;
};

Nav_Posllh NAV;

void Cheksum(unsigned char* CS)
{
	CS[0] = 0;
	CS[1] = 0;
	for(int i = 0; i < (int)sizeof(NAV); i++)
	{
		CS[0] = CS[0] + ((unsigned char*)(&NAV))[i];
		CS[1] = CS[1] + CS[0];
	}
}

void GPS_Configuration() //PINY C 10 i 11
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	// wlaczenie taktowania wybranego uk³adu USART
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	// konfiguracja linii Rx i Tx
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	// ustawienie funkcji alternatywnej dla pinów (USART)
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);
	USART_InitTypeDef USART_InitStructure;
	// predkosc transmisji (mozliwe standardowe opcje: 9600, 19200, 38400, 57600, 115200, ...)
	USART_InitStructure.USART_BaudRate = 9600;
	// d³ugoœæ s³owa (USART_WordLength_8b lub USART_WordLength_9b)
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// liczba bitów stopu (USART_StopBits_1, USART_StopBits_0_5, USART_StopBits_2, USART_StopBits_1_5)
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// sprawdzanie parzystoœci (USART_Parity_No, USART_Parity_Even, USART_Parity_Odd)
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// sprzêtowa kontrola przep³ywu (USART_HardwareFlowControl_None, USART_HardwareFlowControl_RTS, USART_HardwareFlowControl_CTS, USART_HardwareFlowControl_RTS_CTS)
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// tryb nadawania/odbierania (USART_Mode_Rx, USART_Mode_Rx )
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// konfiguracja
	USART_Init(USART3, &USART_InitStructure);
	// wlaczenie ukladu USART
	USART_Cmd(USART3, ENABLE);
	//struktura do konfiguracji kontrolera NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	// wlaczenie przerwania zwi¹zanego z odebraniem danych (pozostale zrodla przerwan zdefiniowane sa w pliku stm32f4xx_usart.h)
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	// konfiguracja kontrolera przerwan
	NVIC_Init(&NVIC_InitStructure);
	// wlaczenie przerwan od ukladu USART
	NVIC_EnableIRQ(USART3_IRQn);
}
/*void GPS_Message(char* message)
{
	uint8_t data;
	GpsPut('$');
	while((data = message) != 0)
	{
		GpsPut(data);
		message++;
	}
	GpsPut('*');
	GpsPut(HEX_CHARS[cs >> 4]);
	GpsPut(HEX_CHARS[cs & 0x0F]);
	GpsPut('\r');
	GpsPut('\n');
	USART_ITConfig(GPS_USART, USART_IT_TXE, ENABLE);
}*/
void USART3_IRQHandler(void)
{
    // sprawdzenie flagi zwiazanej z odebraniem danych przez USART
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        // odebrany bajt znajduje sie w rejestrze USART3->DR
    	uint16_t byte = USART_ReceiveData(USART3);
    }
}
int main(void)
{
	SystemInit();
	GPS_Configuration();
    while(1)
    {
    }
}
