#ifndef H_MAIN
#define H_MAIN 1

#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "string.h"

//Description connect
//USART1 8N1
#define USART_PC_BOUDRATE 115200
//USART3 & USART6 8N1
#define USART_INTERNAL_BOUDRATE 19200

#define MAX_BUFFER 255

//user_flags bits
#define ALL_STRING_RECEIVE 0

void RCC_Init(void);


#endif