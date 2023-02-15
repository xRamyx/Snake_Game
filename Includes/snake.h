 /******************************************************************************
 *
 * Module: SNAKE
 *
 * File Name: snake.h
 *
 * Description: Header file for functions prototypes for SNAKE module
 *
 * Author: Ramy Mohamed Ahmed Mohamed
 *
 *******************************************************************************/
#ifndef SNAKE_H_
#define SNAKE_H_

#include "std_types.h"
#include "bsp.h"

#define DX	64
#define DY	30

/* Struct to hold the points of the snake */
typedef struct Point
{
	uint8 x;
	uint8 y;
}Point;

/* Enum to represent the directions of the snake */
typedef enum SnakeDir
{
	RIGHT,
	LEFT,
	UP,
	DOWN
} SnakeDir;


/****************************** Functions Prototypes ******************************/
void *memsetf(void *dest, register int val, register uint32 len);
	
#endif /* SNAKE_H_ */