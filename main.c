/*
File    : main.c

*/

#include "main.h"

//global values
char *pt_buffer1;
char *pt_buffer3;
char *pt_buffer6;
char rx_buffer[MAX_BUFFER];
uint8_t rx_buffer_iterator = 0;

volatile uint8_t user_flags = 0x00;

/****************************** function**********************************/
void send_USART1_STR(char *sendbuffer1){
  while((USART1->SR & USART_SR_TC) == 0){}
  pt_buffer1 = &(*sendbuffer1);
  USART1->CR1 &= ~(USART_CR1_RXNEIE);
  USART1->DR = (*pt_buffer1 & (uint16_t)0x01FF);
  USART1->CR1 |= USART_CR1_TCIE;
}

void send_USART3_STR(char *sendbuffer3){
  while((USART3->SR & USART_SR_TC) == 0){}
  pt_buffer3 = &(*sendbuffer3);
  USART3->CR1 &= ~(USART_CR1_RXNEIE);
  EXTI->IMR &= ~(EXTI_IMR_IM10);
  USART3->DR = (*pt_buffer3 & (uint16_t)0x01FF);
  USART3->CR1 |= USART_CR1_TCIE;
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
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN|RCC_APB1ENR_PWREN;
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PE;
  
    // 84MHz / 115200 bod / 16 = 45,57  M=45 (0x2D) F=0,57*16=9 (0x09)
  USART1->BRR = 0x02D9;
  // 42MHz / 19200 bod / 16 = 136,72  M=136 (0x88) F=0,72*16=12 (0x0C)
  USART3->BRR = 0x088C;
  // 84MHz / 19200 bod / 16 = 273,44  M=273 (0x111) F=0,44*16=7 (0x07)
  USART6->BRR = 0x1117;

  //RTC Initial
  PWR->CR |= PWR_CR_DBP;
  RCC->BDCR |= RCC_BDCR_LSEON;
  while(!(RCC->BDCR & RCC_BDCR_LSERDY)){}
  RCC->BDCR &= ~(RCC_BDCR_RTCSEL);
  RCC->BDCR |= RCC_BDCR_RTCSEL_0 | RCC_BDCR_RTCEN;
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  RTC->ISR |= RTC_ISR_INIT;
  while(!(RTC->ISR & RTC_ISR_INITF)){};	
  RTC->PRER |= 0xFF;
  RTC->PRER |= (uint32_t)(0x7F << 16);
  RTC->ISR &= ~(RTC_ISR_INIT);
  RTC->WPR = 0x00;

  //Connections Initial
  USART1->CR1 |= USART_CR1_TE|USART_CR1_RE;
  USART1->CR1 &= ~(USART_CR1_M|USART_CR1_PCE);
  USART1->CR2 &= ~(USART_CR2_STOP);

  USART3->CR1 |= USART_CR1_TE|USART_CR1_RE;
  USART3->CR1 &= ~(USART_CR1_M|USART_CR1_PCE);
  USART3->CR2 &= ~(USART_CR2_STOP);

  USART6->CR1 |= USART_CR1_RXNEIE|USART_CR1_TE|USART_CR1_RE;
  USART6->CR1 &= ~(USART_CR1_M|USART_CR1_PCE);
  USART6->CR2 &= ~(USART_CR2_STOP);

  __enable_irq();
  //GPIO Initial
  GPIOE->MODER |= GPIO_MODER_MODE13_0|GPIO_MODER_MODE14_0|GPIO_MODER_MODE15_0;
  GPIOE->MODER &= ~(GPIO_MODER_MODE10);
  GPIOA->MODER |= GPIO_MODER_MODE9_1|GPIO_MODER_MODE10_1;
  GPIOA->AFR[1] |= (7<<GPIO_AFRH_AFSEL9_Pos)|(7<<GPIO_AFRH_AFSEL10_Pos);
  GPIOC->MODER |= GPIO_MODER_MODE6_1|GPIO_MODER_MODE7_1;
  GPIOC->AFR[0] |= (8<<GPIO_AFRL_AFSEL6_Pos)|(8<<GPIO_AFRL_AFSEL7_Pos); 
  GPIOD->MODER |= GPIO_MODER_MODE8_1|GPIO_MODER_MODE9_1;
  GPIOD->AFR[1] |= (7<<GPIO_AFRH_AFSEL8_Pos)|(7<<GPIO_AFRH_AFSEL9_Pos);

  //Interrupts Initial
  //S1 & S3 & S3
  EXTI->PR |= EXTI_PR_PR10;
  EXTI->FTSR |= EXTI_FTSR_TR10;
  EXTI->IMR |= EXTI_IMR_IM10; 
  
  //Interrupt NVIC Enable
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_EnableIRQ(USART3_IRQn);
  NVIC_EnableIRQ(USART6_IRQn);
  
  //Enable USART1
  USART1->CR1 |= USART_CR1_UE;
  USART3->CR1 |= USART_CR1_UE;
  USART6->CR1 |= USART_CR1_UE;
  //LED turn off
  GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
  send_USART3_STR("\r\n");

  while(1){
  if(user_flags & (1<<ALL_STRING_RECEIVE)){
    char temp_string[MAX_BUFFER];
    strcpy(temp_string, rx_buffer);
    memset(rx_buffer,0,MAX_BUFFER);
    /*
    for(uint8_t i = strlen(rx_buffer)-1;i>=0;i--){
      temp_string[i]=rx_buffer[i];
    }
    */
    user_flags &= ~(1<<ALL_STRING_RECEIVE);
    USART6->CR1 |= USART_CR1_RXNEIE;
    send_USART1_STR(temp_string);
  }
    __NOP();
  }
}
/***********************interrupts function**************************/
//key
void EXTI15_10_IRQHandler(void){
  uint8_t dt = 0;
  char temp_string[MAX_BUFFER];
  char dt_str[10];
  dt = (uint8_t)RTC->TR & (uint8_t)0x7F;
  //dt = 16;
  dt = ((dt>>4)*10 + (dt & 0x0F));
  strcpy(temp_string,"Seconds=");
  itoa(dt, dt_str, 10);
  strcat(dt_str, "\r\n");
  strcat(temp_string, dt_str);
  send_USART3_STR(temp_string);
  EXTI->PR |= EXTI_PR_PR10;
}
//transmitt to PC
void USART1_IRQHandler(void){
  if(USART1->SR & USART_SR_TC){
    if(*pt_buffer1 == '\n'){
      USART1->CR1 &= ~(USART_CR1_TCIE);
      USART1->CR1 |= USART_CR1_RXNEIE;
    }else{
      USART1->DR = (*++pt_buffer1 & (uint16_t)0x01FF);
    } 
  }
  NVIC_ClearPendingIRQ(USART1_IRQn);
}
//transmitter
void USART3_IRQHandler(void){
  if(USART3->SR & USART_SR_TC){
    if(*pt_buffer3 == '\n'){
      USART3->CR1 &= ~(USART_CR1_TCIE);
      EXTI->IMR |= EXTI_IMR_IM10;
      USART3->CR1 |= USART_CR1_RXNEIE;
    }else{
      USART3->DR = (*++pt_buffer3 & (uint16_t)0x01FF);
    } 
  }
  NVIC_ClearPendingIRQ(USART3_IRQn);
}

//receiver
void USART6_IRQHandler(void){
  if(USART6->SR & USART_SR_RXNE){
    char receive = (uint16_t)USART6->DR & (uint16_t)0x01FF;
    if(receive == '\n' || receive == 0 || rx_buffer_iterator == 255){
      rx_buffer[rx_buffer_iterator] = '\n';
      rx_buffer_iterator = 0;
      user_flags |= (1<<ALL_STRING_RECEIVE);
      USART6->CR1 &= ~(USART_CR1_RXNEIE);
    }else{
      rx_buffer[rx_buffer_iterator++] = receive;
    }
  }
  NVIC_ClearPendingIRQ(USART6_IRQn);
}
/*************************** End of file ****************************/
