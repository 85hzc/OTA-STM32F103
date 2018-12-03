#include "body.h"

uint8 fBody = 0;		// 1������   0 ����

/**
	* @brief  ����Ƿ�����������ֽ��, �м�ֹͣ���
  * @param  void  
  * @retval void
  */
static void Check_Body_In(void)
{
	static uint8 BodyCount = 0, HaveCount = 0;
	static uint8 DelayTime = 0;			// �����Ӧ������ʱ��
	
	/***
	*		����߼������ǵ���·���ϻ����ܴ�����������ȶ��ĸߵ�ƽ
	*							10�ζ�ȡ�У�������5�θߵ�ƽ������Ϊ������������ߵ�ƽ
	*/
	if (BODY_STATE)		// ����
	{
		HaveCount++;
	}	

	if (++BodyCount >= 10)
	{
		if (HaveCount <= 2) 			// û����
		{		
			if (++DelayTime >= 15)		// ������ʱ����СΪ1s, Ҳ��֤��Ƶ��������ֹͣ��1.5s��϶
			{
				if (  (fBody & BODY_HAVE) )  // (gMcuWorkStep == MCU_TIME_CUT_DOWN) &&
				{
					fState |= SHREDDER_CUT; 	// ������ֽ״̬				
				}
				CLEAR_BODY_HAVE;
				fBody &= ~BODY_HAVE;
				DelayTime = 15;						
			}
		}
		else if (HaveCount >= 5) 	// ����
		{				
			SET_BODY_HAVE;
					
			DelayTime = 0;	
			Motor_Stop();								
			
			if ((fBody & BODY_HAVE) == 0)
			{
				fBody |= BODY_HAVE;
				
				if ((fBody & INF_HAVE) == 0)			fState |= TIME_CUT_DOWN;  // �ϴ�����ʱ	
			}						
		}
		
		HaveCount = 0;
		BodyCount = 0;
	}
}

/**
	* @brief  ����Ƿ�����������ֽ��, �м�ֹͣ���
  * @param  void  
  * @retval void
  */
void Check_Body_State(void)
{
	switch (gMcuWorkStep)
	{
		case MCU_GET_CUT_PAPER_CMD:		// �������ʼ��ֽ��
		case MCU_CUT_PAPER:						// �ȴ�����ֽ��ɡ� 
		case MCU_TIME_CUT_DOWN:				// �ϴ�����ʱ		
							Check_Body_In();	
				break;
		
		case MCU_GET_CUT_DOWM_CMD:		// �յ�����ֽ��ɡ� -> �����ʱ	
		case MCU_SYS_OPEN:						// ɨ���ά��
		case MCU_GET_WEIGHT_TO_SEND: 	// ���� 
		case MCU_RUN_PROCESS_OK:    	// �����һ������
		case MCU_SYS_CLOSE:						// �����ر���
				break;
		
		default:
					
				break;
	}
}

/**
  * @brief  �����Ӧ���� -- PA12
  * @param  void  
  * @retval void
  */
void Body_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}




