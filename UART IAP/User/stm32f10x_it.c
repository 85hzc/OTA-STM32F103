#include "stm32f10x_it.h"

void NMI_Handler(void)
{
}


uint8 gUsart2RxDataCounter = 0;
//ƽ��
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		
		if (OTA.RunStatu == OTA_RXD_UART)
		{
			Usart2RxData[gUsart2RxDataCounter] = USART_ReceiveData(USART2);
			
			if (Usart2RxData[gUsart2RxDataCounter] == ANDROID_COMMUN_PROT_FT)	// ���ڽ�����־λ
			{
				fUsart2Rx = TRUE;
				gUsart2RxDataCounter = 0;
			}
			else
			{
				gUsart2RxDataCounter++;

				if (gUsart2RxDataCounter >= 12)
				{
					gUsart2RxDataCounter = 0;
				}
			}
		}
		else 
		{
			OTA_RxBuf[OTA.APPCount++] = USART_ReceiveData(USART2);	
		}
	}
}

uint8 UART3_buf;
void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		UART3_buf = USART_ReceiveData(USART3);
				
		switch (OTA.RunStatu)
		{
			case OTA_RXD_UART:		// ���ܹ̼�
							OTA_RxBuf[Usart_Rx_Cnt++] = UART3_buf;
				break;
			
			case OTA_WAIT:	// �ȴ������ܹ̼�������
							OTA.RunStatu = OTA_RXD_UART;
							Usart_Rx_Cnt = 0;
							printf("Please send firmware\r\n");
				break;
			
			default:
						printf("input err!\r\n");
				break;
		}
	}
}

void CAN1_RX0_IRQHandler(void)
{
}

void CAN2_RX0_IRQHandler(void)
{
}


/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval : None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval : None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval : None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval : None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval : None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval : None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval : None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
void SysTick_Handler(void)
{
}


/****************************************************************************
* ��    �ƣ�void EXTI9_5_IRQHandler(void)
* ��    �ܣ�EXTI9-5�жϴ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�������
****************************************************************************/
void EXTI9_5_IRQHandler(void)
{
}

/****************************************************************************
* ��    �ƣ�void EXTI1_IRQHandler(void)
* ��    �ܣ�EXTI2�жϴ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�������
****************************************************************************/
void EXTI1_IRQHandler(void)
{
}

/****************************************************************************
* ��    �ƣ�void EXTI2_IRQHandler(void)
* ��    �ܣ�EXTI2�жϴ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�������
****************************************************************************/
void EXTI2_IRQHandler(void)
{
}

/****************************************************************************
* ��    �ƣ�void EXTI3_IRQHandler(void)
* ��    �ܣ�EXTI3�жϴ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�������
****************************************************************************/
void EXTI3_IRQHandler(void)
{
}
