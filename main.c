/*
File    : main.c

*/

#include "main.h"

char *pt_buffer;

/****************************** function**********************************/
void send_USART_STR(char *sendbuffer){
  while(USART1->CR1 & USART_CR1_TCIE){}
  pt_buffer = &(*sendbuffer);
  USART1->CR1 &= ~(USART_CR1_RXNEIE);
  USART1->DR = (*pt_buffer & (uint16_t)0x01FF);
  USART1->CR1 |= USART_CR1_TCIE;
}

/*********************************main************************************/
int main(void) {
  //Values initial

  //System Initial
  SystemInit();
  RCC_Init();
  // Clock BUS Initial
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN|RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOCEN|RCC_AHB1ENR_GPIODEN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_USART1EN|RCC_APB2ENR_USART6EN;
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PE;
  
  // 84MHz / 115200bod / 16 = 45,57  M=45 (0x2D) F=0,57*16=9 (0x09)
  USART1->BRR = 0x02D9;

  //Connections Initial
  USART1->CR1 |= USART_CR1_TE|USART_CR1_RE;
  USART1->CR1 &= ~(USART_CR1_M|USART_CR1_PCE);
  USART1->CR2 &= ~(USART_CR2_STOP);

  __enable_irq();
  //GPIO Initial
  GPIOE->MODER &= ~(GPIO_MODER_MODE10);
  GPIOA->MODER |= GPIO_MODER_MODE9_1|GPIO_MODER_MODE10_1;
  GPIOA->AFR[1] |= (7<<GPIO_AFRH_AFSEL9_Pos)|(7<<GPIO_AFRH_AFSEL10_Pos);
 
  //Interrupts Initial
  //S1 & S3 & S3
  EXTI->PR |= EXTI_PR_PR10;
  EXTI->FTSR |= EXTI_FTSR_TR10;
  EXTI->IMR |= EXTI_IMR_IM10; 
  
  //Interrupt NVIC Enable
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_EnableIRQ(USART1_IRQn);
  
  //Enable USART1
  USART1->CR1 |= USART_CR1_UE;
  //LED turn off
  GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
  send_USART_STR(" \r\n");

  while(1){
    __NOP();
  }
}
/***********************interrupts function**************************/
void EXTI15_10_IRQHandler(void){

  
  EXTI->PR |= EXTI_PR_PR10;
}

void USART1_IRQHandler(void){
  if(USART1->SR & USART_SR_TC){
    if(*pt_buffer == '\n'){
      USART1->CR1 &= ~(USART_CR1_TCIE);
      USART1->CR1 |= USART_CR1_RXNEIE;
    }else{
      USART1->DR = (*++pt_buffer & (uint16_t)0x01FF);
    } 
  }
  NVIC_ClearPendingIRQ(USART1_IRQn);
}
/*************************** End of file ****************************/
