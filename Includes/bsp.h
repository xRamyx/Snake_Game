 /******************************************************************************
 *
 * Module: BSP
 *
 * File Name: bsp.h
 *
 * Description: Header file for functions prototypes for BSP module
 *
 * Author: Ramy Mohamed Ahmed Mohamed
 *
 *******************************************************************************/
#ifndef BSP_H_
#define BSP_H_

#define UART_SYS_CLK	16000000
#define BAUDRATE	115200

/****************************** Functions Prototypes ******************************/
void uart0_init(unsigned int clk, unsigned int baudrate);
void clearUART0INT(void);
char readChar(void);
void uart0_putchar(char c);
	
void print(const char *str);
void printn(const char *str, int n);

#endif /* BSP_H_ */