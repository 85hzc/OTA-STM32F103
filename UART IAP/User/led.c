#include "led.h"

/**
  * @brief  LEDָʾ��(PA4)
  * @param  void  
  * @retval void
  */
void Led_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  ������IO(PB4)
  * @param  void  
  * @retval void
  */
void LOCK_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	LOCK_CLOSE;
}
/**
  * @brief  ����ʹ��PIN(PB8)
  * @param  void  
  * @retval void
  */
void FAN_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);	
	
	//KEY3 PA15��Ҫ�ȹر�JTAG����
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// ����ʹ��PIN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	FAN_CLOSE;
}

/**
  * @brief  LED(PA4)���е�  1s��˸һ��
  * @param  void  
  * @retval void
  */
void Led_Run_Show(void)
{
	static uint8 ledshow = 0;
	
	if(ledshow)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		ledshow = 0;
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_4);
		ledshow = 1;
	}
}

uint8 Lock_Time = LOCK_TIME_MAX;
/**
  * @brief  �յ��������������8s��8s���Զ�����
  * @param  void  
  * @retval void
  */
void Lock_Check(void)
{
	if (Lock_Time >= LOCK_TIME_MAX + 8)	
	{
		LOCK_CLOSE;
		return;
	}
	
	Lock_Time += 4;

	if (Lock_Time >= LOCK_TIME_MAX)	
	{
		LOCK_CLOSE;
		printf("LOCK_CLOSE\r\n");
		return;
	}
	
	LOCK_OPEN;
	printf("LOCK_OPEN\r\n");
}








