#include "Main.h"

/**
  * @brief  �����ʼ��
  * @retval None
  */
static void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	USART2_Config(9600);	//ƽ��			9600	
	USART3_Config(115200);//��Ϣ��ӡ	115200
	
	Led_GPIO_Init();
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

/**
  * @brief  Show_Version
  * @retval None
  */
static void Show_Version(void)
{
	printf("\r\n\r\n");
	printf(" \\           /\r\n");
	printf("- Paperbanker - Shredder System\r\n");
	printf(" /           \\  Bootloader Version %d.%d.%d \r\n", 
						VERSION, SUBVERSION, REVISION);
	printf("                Copyright by paperbanker team\r\n\r\n");
}

/**
  * @brief  main
  * @retval None
  */
int main(void)
{		
	uint8 tim = 0;
	
	Hardware_Init();

	Show_Version();
	
	BSP_Flash_Init();

	while (1)
	{	
		BSP_Delay_us(10000);	// 10ms
		
		if (tim++ >= 50)	 		// 500ms
		{
			tim = 0; 

			check_Firmware();	 // ���̼��Ƿ�������
			Receive_Firmware();
			
			Led_Run_Show();
		}			
	}
}

static const uint8 fac_us = 72; // �δ�ʱ����ʱ��Ƶ�ʣ��δ�ʱ��ʱ��Ϊ: HCLK/8
/**
  * @brief  ��ʱn*us
  * @param  nus: ��ʱʱ�䣬��λus 
  * @retval None
  */
void BSP_Delay_us(uint32 nus)
{
	uint32 temp;	    
	
	SysTick->LOAD = nus * fac_us; 						//ʱ�����	  		 
	SysTick->VAL = 0x00;        							//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//��ʼ����	
  
	do
	{
		temp = SysTick->CTRL;
	}while((temp & 0x01) && !(temp & (1 << 16)));		//�ȴ�ʱ�䵽�� 
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;			//�رռ�����
	SysTick->VAL = 0X00;      											//��ռ�����	 
}



