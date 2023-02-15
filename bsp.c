 /******************************************************************************
 *
 * Module: BSP
 *
 * File Name: bsp.c
 *
 * Description: Source file for functions definitions for BSP module
 *
 * Author: Ramy Mohamed Ahmed Mohamed
 *
 *******************************************************************************/
#include "tm4c123gh6pm.h"
#include "bsp.h"

/**************************** Functions Definitions ****************************/
void uart0_init(unsigned int clk, unsigned int baudrate)
{ //UART0 pins(PA0, PA1)
	SYSCTL_RCGCUART_R |= (1<<0); //Enable UART0 clock
	while((SYSCTL_PRUART_R & (1<<0))==0){}; //Wait till enabled
	
	SYSCTL_RCGCGPIO_R |= (1<<0); //Enable PORTA clock source
	while ((SYSCTL_PRGPIO_R & (1<<0)) == 0){}; //Wait till enabled
		
	GPIO_PORTA_AFSEL_R |= (1<<0) | (1<<1);
	GPIO_PORTA_PCTL_R |= (1<<0) | (1<<4);
	GPIO_PORTA_DEN_R |= (1<<0) | (1<<1);
		
	UART0_CTL_R &= ~(1<<0); // UARTEN = 0
	int BRD = ((clk<<2)+(baudrate>>1))/baudrate;
	UART0_IBRD_R = BRD>>6;
	UART0_FBRD_R = BRD&63;
	UART0_LCRH_R = (0x3<<5); // 8-bit word length,
	UART0_CC_R = 0x0; // System clock by default
	UART0_CTL_R |= (1<<0) | (1<<8) | (1<<9); // RXE = 1, TXE = 1 , UARTEN = 1
		
	/*Enable interrupt*/
	UART0_ICR_R &= ~(0x0010); // Clear receive interrupt
  UART0_IM_R  = 0x0010;     // Enable receiving interrupt Rx
  NVIC_EN0_R |= 0x00000020; // Enable IRQ5 for UART0
  
}

void clearUART0INT(void)
{
	UART0_ICR_R &= ~(0x0010);
}

char readChar(void)
{
	char c;
	while((UART0_FR_R & (1<<4))!=0);
	c = UART0_DR_R;
	return c;
	
}
void uart0_putchar(char c)
{
	while((UART0_FR_R & (1<<5))!=0);
	UART0_DR_R = c;
}

void print(const char *str)
{
	while(*str)
	{
		if(*str == '\n')
		{
			uart0_putchar('\r');
		}
		uart0_putchar(*str);
		str++;
	}
}

void printn(const char *str, int n)
{
	int i = 0;
	for(i = 0; i < n; i++)
	{
		if(str[i] == '\n')
		{
			uart0_putchar('\r');
		}
		uart0_putchar(str[i]);
	}
}