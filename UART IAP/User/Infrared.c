#include "Infrared.h"

uint16 gNoPaperCounter = 0;					//�ϴ�����ʱ

/**
  * @brief  �ϴ�����ʱ ֻҪ��⵽��ֽ 
	*	@note   ����ˢ�¹���״̬, ȥ������ʱ״̬
  * @param  void  
  * @retval void
  */
static void Check_Infrared_Cut_Dowm(void)
{
	if (fBody & BODY_HAVE)	return;			// ����
	
	if (INFRARE_STATE == 0)	// ��ֽ
	{		
		fBody &= ~INF_HAVE;
		
		fState &= ~TIME_CUT_DOWN;	// �������ʱ
		fState |= SHREDDER_CUT; 	// ������ֽ״̬
		
		gMcuWorkStep = MCU_CUT_PAPER;
		
		SET_WORK_STATE_WORK;			// ���빤��״̬		
		CLEAR_NO_PAPER_COUTER;		// ˢ�� ��ֽ״̬���ֽ�Ĳ���	
	}
}

/**
  * @brief  δ�ϴ�����ʱ������Ƿ���ֽ
	* @note		��ֽ����ˢ�¹���״̬
  * @param  void  
  * @retval void
  */
static void Check_Infrared_State(void)
{
	uint8 buf = 0;

	// �˹��̷��������ת Ҳ�����ж��Ƿ���ֽ����
	buf = fState & MOTOR_CUR_ERR;
	if (buf == MOTOR_CUR_ERR)
	{
		return;
	}

	// һ�������ϱ�����ʱ �����ж��Ƿ���ֽ���� ֱ������ʱ���
	buf = fState & TIME_CUT_DOWN;
	if (buf == TIME_CUT_DOWN)
	{
		return;
	}

	//��ֽ
	if (INFRARE_STATE == 0) 
	{
		CLEAR_NO_PAPER_COUTER;
		fState &= ~TIME_CUT_DOWN;
	}
	else
	{
		gNoPaperCounter++;
		if (gNoPaperCounter >= NO_PAPER_COUNTER_MAX)
		{
			gNoPaperCounter = NO_PAPER_COUNTER_MAX;
			SET_WORK_STATE_FREE;									// ����״̬
			
			gMcuWorkStep = MCU_TIME_CUT_DOWN;			// MCU�����ϴ�����ʱ״̬		
			
			if (fBody == 0)			// ��⵽���֣����ϴ���ֽ֪ͨ
			{			
				fState |= TIME_CUT_DOWN;  						// �ϴ�����ʱ							
			}	
			fBody |= INF_HAVE;		
		}
	}
}

/**
  * @brief  ��ֽ�ڼ��	
  * @param  void  
  * @retval void
  */
void Check_Infrared_Every_State(void)
{
	switch (gMcuWorkStep)
	{
		case MCU_CUT_PAPER:					//�ȴ�����ֽ��ɡ� 
					Check_Infrared_State();
			break;

		case MCU_TIME_CUT_DOWN:			//�ϴ�����ʱ
					Check_Infrared_Cut_Dowm();					
			break;
		
		case MCU_SYS_OPEN:					//ɨ���ά��				 
		case MCU_GET_CUT_PAPER_CMD:	//�������ʼ��ֽ��
		case MCU_GET_CUT_DOWM_CMD:	//�յ�����ֽ��ɡ� -> �����ʱ
			
		default:
					fBody &= ~INF_HAVE;
					CLEAR_NO_PAPER_COUTER;
			break;
	}
}

/**
  * @brief  ����-��Դ PA11  �����ƽ��� PA8
  * @param  void  
  * @retval void
  */
void Infrared_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//���� ��Դ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_11);	
	
	//�����ƽ���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}





