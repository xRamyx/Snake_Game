#include "snake.h"
#include "XorShift.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

/* The tasks to be created. */
static void initTask( void *pvParameters );
static void drawTask( void *pvParameters );
static void gameTask( void *pvParameters );
/*-----------------------------------------------------------*/
xSemaphoreHandle xMutex;
TimerHandle_t xTickTimer = NULL;

BaseType_t dTask, gTask, iTask;

uint8 screen[DX*DY] = {0};
uint16 snakeLength = 3;
boolean drawFirst = TRUE;
boolean timerStarted = FALSE;
boolean gameOver = FALSE;
boolean removeTail = TRUE;
boolean win = FALSE;
boolean specialHidden = FALSE;
uint8 specialAppear = 0;
uint8 specialHide = 0;
uint32 speed = 0;
uint16 score = 0;
uint16 highestScore = 0;
uint16 winScore = 0;
uint16 time = 0;
char buffer[10]; //a buffer used to store a score and time strings to be displayed
xorshift32_state powerupXPlusPos = {1};
xorshift32_state *powerupXPlusPtr = &powerupXPlusPos;

xorshift32_state powerupXDollarPos = {1};
xorshift32_state *powerupXDollarPtr = &powerupXDollarPos;

xorshift32_state powerupYPlusPos = {5};
xorshift32_state *powerupYPlusPtr = &powerupYPlusPos;

xorshift32_state powerupYDollarPos = {5};
xorshift32_state *powerupYDollarPtr = &powerupYDollarPos;

Point snake[DX*DY]=
{
	{(DX/2), (DY/2)},
	{((DX/2)+1), (DY/2)},
	{((DX/2)+2), (DY/2)}
};

SnakeDir direction = RIGHT;
/*-----------------------------------------------------------*/
void UART0_Handler(void)
{
	clearUART0INT();
	
	char c = readChar();
	if(c == 'A' || c == 'a')
	{
		if(direction != RIGHT)
		{
			direction = LEFT;
		}
	}
	else if(c == 'D' || c == 'd')
	{
		if(direction != LEFT)
		{
			direction = RIGHT;
		}
	}
	else if(c == 'W' || c == 'w')
	{
		if(direction != DOWN)
		{
			direction = UP;
		}
	}
	else if(c == 'S' || c == 's')
	{
		if(direction != UP)
		{
			direction = DOWN;
		}
	}
}

void vTickTimerCallback(TimerHandle_t pxTimer)
{
	time++;
}
/*-----------------------------------------------------------*/
int main( void )
{
	xTickTimer = xTimerCreate
             (
                  /* Just a text name, not used by the RTOS kernel. */
                  "Tick Timer",
                  /* The timer period in ticks. */
                  pdMS_TO_TICKS( 1000 ),
                  pdTRUE,
                  /* The id is not used by the callback so can take any
                  value. */
                  0,
                  /* The callback function that switches the LCD back-light
                  off. */
                  vTickTimerCallback
              );
	
  xMutex = xSemaphoreCreateMutex();
	
	/* Initializing UART0 */
	uart0_init(UART_SYS_CLK, BAUDRATE);
	
	if( xMutex != NULL )
	{
		dTask = xTaskCreate( drawTask, "Draw Task", 240, NULL, 3, NULL );
		gTask = xTaskCreate( gameTask, "Game Task", 240, NULL, 2, NULL );
		iTask = xTaskCreate( initTask, "Initialization Task", 240, NULL, 4, NULL );
		
		if(dTask != pdPASS || gTask != pdPASS || iTask != pdPASS)
		{
			/* Tasks were not created successfully */
		}
		else
		{
			vTaskStartScheduler();
		}
	}

	for( ;; );
}
/*-----------------------------------------------------------*/
static void initTask( void *pvParameters )
{
	int i;
	for(;;)
	{
		i = 0;
		
		/* clearing the screen */
		memsetf(screen, ' ', (DX*DY));
	
		/* initializing the boundaries */
		for(i = 0; i < DX; i++)
		{
			screen[i] = 'X';
		}
		for(i = 0; i < DX; i++)
		{
			screen[(DX*(DY-1))+i] = 'X';
		}
		for(i = 0; i < DY; i++)
		{
			screen[DX*i] = 'X';
		}
		for(i = 0; i < DY; i++)
		{
			screen[(DX*i)+(DX-1)] = 'X';
		}
		
		/* draw initial power ups */
		do
		{
			xorshift32(powerupXPlusPtr);
			xorshift32(powerupYPlusPtr);
		}while(screen[(DX*DY)/(powerupYPlusPtr->a % 16)+(powerupXPlusPtr->a % 32)] != ' ');
		screen[(DX*DY)/(powerupYPlusPtr->a % 16)+(powerupXPlusPtr->a % 32)] = '+';
		
		do
		{
			xorshift32(powerupXDollarPtr);
			xorshift32(powerupYDollarPtr);
		}while(screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] != ' ');
		screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] = '$';
		
		/* initializing the snake position */
		for(i = 0; i < snakeLength; i++)
		{
			Point *p = snake + i;
			screen[(DX*(p->y))+(p->x)] = 'O';
		}
		
		print("Welcome to the famous snake game!\n");
		print("You can control your snake moves with W,S,A, and D\n");
		print("Highest score: ");
		sprintf(buffer,"%d",highestScore); // Change integer to string and making explicit type casting from float to integer
		print(buffer);
		print("\n");
		print("******Press ENTER to start the game!******\n\n");
		vTaskDelete(NULL);
	}
}

static void drawTask( void *pvParameters )
{
	int i;
	
	for(;;)
	{
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if(drawFirst)
		{
			char c;
			do
			{
				c = readChar();
			}while(c != '\r');
			
			if(!timerStarted)
			{
				/* Timer will interrupt every 1 sec */
				xTimerStart(xTickTimer, 0);
				timerStarted = TRUE;
			}
			
			/* print the screen with the initialized array */
			for(i = 0; i < DY; i++)
			{
				printn((screen+(DX*i)), DX);
				print("\n");
			}
			print("Score: ");
			sprintf(buffer,"%d",score); // Change integer to string and making explicit type casting from float to integer
			print(buffer);
			print("            Time: ");
			sprintf(buffer,"%d",time); // Change integer to string and making explicit type casting from float to integer
			print(buffer);
			print("\n\n");
			
			drawFirst = FALSE;
			
			xSemaphoreGive(xMutex);
			vTaskDelay((200/portTICK_RATE_MS));
		}
		else
		{
			i = 0;
			for(i = 0; i < DY; i++)
			{
				printn((screen+(DX*i)), DX);
				print("\n");
			}
			print("Score: ");
			sprintf(buffer,"%d",score); // Change integer to string and making explicit type casting from float to integer
			print(buffer);
			print("            Time: ");
			sprintf(buffer,"%d",time); // Change integer to string and making explicit type casting from float to integer
			print(buffer);
			print("\n\n");
			
			if(gameOver)
			{
				print("           Game Over!           \n");
				xTimerStop(xTickTimer, 0);
				timerStarted = FALSE;
				
				win = FALSE;
				gameOver = FALSE;
				
				removeTail = TRUE;
				direction = RIGHT;
				
				snakeLength = 3;
				snake->x = DX/2;
				snake->y = DY/2;
				snake[1].x = DX/2+1;
				snake[1].y = DY/2;
				snake[2].x = DX/2+2;
				snake[2].y = DY/2;
				
				speed = 0;
				if((winScore+score) > highestScore)
				{
					highestScore = winScore + score;
				}
				winScore = 0;
				score = 0;
				time = 0;
				drawFirst = TRUE;
				specialHidden = FALSE;
				specialAppear = 0;
				specialHide = 0;
				xTaskCreate( initTask, "Initialization Task", 240, NULL, 4, NULL );
			}
			
			if(win)
			{
				print("           You Won!           \n");
				print("Now the game will be faster!           \n");
				xTimerStop(xTickTimer, 0);
				timerStarted = FALSE;
				
				if(speed < 150)
				{
					speed +=50;
				}
				
				win = FALSE;
				removeTail = TRUE;
				direction = RIGHT;
				
				snakeLength = 3;
				snake->x = DX/2;
				snake->y = DY/2;
				snake[1].x = DX/2+1;
				snake[1].y = DY/2;
				snake[2].x = DX/2+2;
				snake[2].y = DY/2;
				
				winScore+=score;
				score = 0;
				time = 0;
				drawFirst = TRUE;
				specialHidden = FALSE;
				specialAppear = 0;
				specialHide = 0;
				xTaskCreate( initTask, "Initialization Task", 240, NULL, 4, NULL );
			}
			
			xSemaphoreGive(xMutex);
			vTaskDelay((200/portTICK_RATE_MS));
		}
	}
}

static void gameTask( void *pvParameters )
{
		unsigned portBASE_TYPE uxPriority;
		uxPriority = uxTaskPriorityGet( NULL );
	
		int i = 0;

		for(;;)
		{
			xSemaphoreTake(xMutex, portMAX_DELAY);
			
			/* special power-up '$' appears briefly at random locations */
			if(specialAppear == 30 && !specialHidden)
			{
				specialAppear = 0;
				
				do
				{
					xorshift32(powerupXDollarPtr);
					xorshift32(powerupYDollarPtr);
				}while(screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] != '$');
				screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] = ' ';
				specialHidden = TRUE;
				
			}
			else if(specialHide == 40 && specialHidden)
			{
				specialHide = 0;

				do
				{
					xorshift32(powerupXDollarPtr);
					xorshift32(powerupYDollarPtr);
				}while(screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] != ' ');
				screen[(DX*DY)/(powerupYDollarPtr->a % 16)+(powerupXDollarPtr->a % 32)] = '$';
				specialHidden = FALSE;
			}
			else if(!specialHidden)
			{
				specialAppear++;
			}
			else if(specialHidden)
			{
				specialHide++;
			}
			
			/* switch on the direction and move the snake head */
			switch(direction)
			{
				case RIGHT:
					snake[snakeLength].x = snake[snakeLength-1].x+1;
					snake[snakeLength].y = snake[snakeLength-1].y;
					break;
				
				case LEFT:
					snake[snakeLength].x = snake[snakeLength-1].x-1;
					snake[snakeLength].y = snake[snakeLength-1].y;
					break;
				
				case UP:
					snake[snakeLength].x = snake[snakeLength-1].x;
					snake[snakeLength].y = snake[snakeLength-1].y-1;
					break;
				
				case DOWN:
					snake[snakeLength].x = snake[snakeLength-1].x;
					snake[snakeLength].y = snake[snakeLength-1].y+1;
					break;
			}
			
			/* switch on the new head position to see if it is a power up or game over*/
			switch(screen[DX*snake[snakeLength].y+snake[snakeLength].x])
			{
				case 'O':
				case 'X':
					gameOver = TRUE;
					break;
				
				case '+':
					score+=10;
					snakeLength++;
					if(snakeLength == 10 || score >=100)
					{
						win = TRUE;
					}
					removeTail = FALSE;
					
					do
					{
						xorshift32(powerupXPlusPtr);
						xorshift32(powerupYPlusPtr);
					}while(screen[(DX*DY)/(powerupYPlusPtr->a % 16)+(powerupXPlusPtr->a % 32)] != ' ');
					screen[(DX*DY)/(powerupYPlusPtr->a % 16)+(powerupXPlusPtr->a % 32)] = '+';
					break;
					
				case '$':
					score += 20;
					if(score >= 100)
					{
						win = TRUE;
					}
					specialHidden= TRUE;
					specialHide = 0;
					specialAppear = 0;
					break;
			}
			
			if(!gameOver)
			{
				/* extend the snake by one if it takes '+' power up, or just move his body to new position */
				screen[DX*snake[snakeLength-!removeTail].y+snake[snakeLength-!removeTail].x] = 'O';
				if(removeTail)
				{
					int i = 0;
					screen[DX*(snake->y)+(snake->x)] = ' ';
					for(i = 0; i < snakeLength; i++)
					{
						snake[i] = snake[i+1];
					}
				}
				removeTail = TRUE;
			}
			
			xSemaphoreGive(xMutex);
			vTaskDelay((205-speed/portTICK_RATE_MS));
		}
}
/*-----------------------------------------------------------*/
void vApplicationIdleHook( void )
{
	/* Place the processor into low power mode */
	__asm("WFI");
}