#include "motor.h"

static uint8 Usart1RxDataBuf[4] = {0};

uint8 Usart1RxData[4] = {0};
uint8 fUsart1Rx = 0;
uint8 fMotorPoAndNegEnd = 0;//0--û�ж�ת��  1--��ת��
uint8 fMotorLock = 0;				//����ȷ�϶�ת  0--û�ж�ת 1--��ת��
uint8 DcMotorCloseFlag = 0xff;

//---------------------- ��Ƶ�� ----------------------------
/**
  * @brief   Motor_Check_Rx_Data
  * @param  
  * @retval  1--ͨ�ųɹ� �޹���  2--ͨ�ųɹ��й���   0--ͨ�Ų��ɹ�	��ʽ����
  */
static uint8 Motor_Check_Rx_Data(uint8 *pstr)
{
	//���ݲ�ͬ֡ͷ �ж�
	if(pstr[0] == MOTOR_COMMUN_PROT_FH)
	{
		return 1;
	}
	return 0;
}

/**
  * @brief   �������ݼ��
  * @param   void
  * @retval  0 ͨѶʧ��	 1 ����  2 ��ת
  */
static uint8 Check_Cmd_Motor(void)
{
	uint8 ireturn = 0;
	
	if (fUsart1Rx == TRUE)
	{
		fUsart1Rx = FALSE;
		
		memset(&Usart1RxDataBuf[0], 0, 4);
		memcpy(&Usart1RxDataBuf[0], &Usart1RxData[0], 4);
		memset(&Usart1RxData[0], 0, 4);
		
		ireturn = Motor_Check_Rx_Data(&Usart1RxDataBuf[0]);

		if(ireturn == 1)//������ȡ
		{
	
		}	
		return (uint8)(ireturn);
	}
	
	if (fMotorPoAndNegEnd)	// ��ת
	{
		return 2;
	}
	return 0;
}

/**
  * @brief   ��Ƶ������ͨѶ ��������
  * @param   
  * @retval  void
  */
static void Motor_Uart_Send_Data(uint8 cmd)
{
	uint8 datbuf[4] = {0};
	uint8 i = 0;

	datbuf[0] = MOTOR_COMMUN_PROT_FH;
	datbuf[1] = cmd;
	datbuf[2] = (uint8)(~MOTOR_COMMUN_PROT_FH);

	datbuf[3] = (uint8)(~cmd);

	for(i = 0; i < 4; i++)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, datbuf[i]);
	}
}

/**
  * @brief   ��������
  * @param   cmd �� ����ָ��
  * @retval  0--ͨ��ʧ�� 
									fState   |= MOTOR_ERR;	�����ʱͨ��	
									gGetWorkState |= WORKSTATE_ERR;			���ڹ���״̬
						 1--�ɹ�	
						 2--��ת	
  */
uint8 Check_Motor_Step(uint8 cmd)
{
	uint8 buff = 0;
	uint8 counter = 0;
	uint8 errcount = 0;

#if	!MOTOR_SWITCH
	return 1;	// debug
#else 	
	Motor_Uart_Send_Data(cmd);
	counter = 0;
	errcount = 0;
	
	//��ʱӦ��
	while (1)
	{	
		rt_thread_delay(500); 
		
		buff = Check_Cmd_Motor();

		if (buff == 1)
		{
			fState &= ~MOTOR_ERR;
			
			return 1;
		}

		if (buff == 2)	//��ת
		{
			printf("Motor lock %x ...\r\n",cmd);
			return 2;
		}
		
		counter++;	
		if (counter >= 2)
		{		
			errcount++;		
			Motor_Uart_Send_Data(cmd);
			counter = 0;
		}
			
		if (errcount >= 5)
		{
			//�����ʱͨ��
			fState |= MOTOR_ERR;
			//���ڹ���״̬
			SET_WORK_STATE_ERR;
			printf("���ͨѶ��ʱ...\r\n");
			return 0;
		}
	}
#endif
}

/**
  * @brief   ֹͣ���
  * @retval  0--ͨ��ʧ�� 
									fState   |= MOTOR_ERR;	�����ʱͨ��	
									gGetWorkState |= WORKSTATE_ERR;			���ڹ���״̬
						 1--�ɹ�						
  */
uint8 Motor_Stop(void)
{
	uint8 count = 0;
	uint8 buf = 0;
	
	buf = Check_Motor_Step(MOTOR_CMD_STOP);
	
	while (buf != 1)
	{
		if (buf == 0) return 0;	// ͨѶ����
		
		rt_thread_delay(500);		
		fMotorPoAndNegEnd = 0;
		
		if (count++ >= 1)	Motor_Uart_Send_Data(MOTOR_CMD_RESET);
		if (count >= 10)	return 0;
		
		buf = Check_Motor_Step(MOTOR_CMD_STOP);
	}
	
	return 1;
}

/**
  * @brief   �ػ�ʱ���Զ���λ
  * @retval  0--ͨ��ʧ�� 
									fState   |= MOTOR_ERR;	�����ʱͨ��	
									gGetWorkState |= WORKSTATE_ERR;			���ڹ���״̬
						 1--�ɹ�						
  */
uint8 Motor_Stop_Reset(void)
{
	uint8 count = 0;
	uint8 buf = 0;
	
	buf = Check_Motor_Step(MOTOR_CMD_STOP);
	
	while (buf != 1)
	{
		if (buf == 0) return 0;	// ͨѶ����
		
		rt_thread_delay(500);		
		fMotorPoAndNegEnd = 0;		
		
		if (count++ >= 1)	Motor_Uart_Send_Data(MOTOR_CMD_RESET);
		
		if (count >= 10)	return 0;
		
		buf = Check_Motor_Step(MOTOR_CMD_STOP);
	}
	
	if (Usart1RxDataBuf[1] & 0x40) 	Motor_Uart_Send_Data(MOTOR_CMD_CHANGE_DIR);
	
	return 1;
}

//-------------------------- ��ת���� ---------------------------------
/**
  * @brief   ��תʱ����ת5s������ת���ٶ�ת����ת5s��ֹͣ
  * @param   void
  * @retval  1--��ת  
									gGetWorkState |= WORKSTATE_ERR;
						 0--����ת	
  */
static uint8 Check_Motor_CurErr_State(void)
{
	uint8 gMotorCurErrCounter = 0; //��תʱ������ת����
	
	fMotorPoAndNegEnd = 0;
	
	//����ֽ�Ź��� ���µ����ס
	Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_MOTOR_LOCK, 0);
	
	while(1)
	{	
		if (gMotorCurErrCounter < MOTOR_CUR_ERR_MAX)	// 
		{		
			gMotorCurErrCounter++;	
			
			// ��ת5s  ��ת5s
			Motor_Stop();
			
			fMotorPoAndNegEnd = 0;			
			Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);
			rt_thread_delay(300);		
			Check_Motor_Step(MOTOR_CMD_GO);	
			
			if (gMotorCurErrCounter == 1 )	
			{	
				printf("1 ��ת5s\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	// 5s		
			}
			else 
			{	
				printf("1 ��ת5s\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	//2s		
				fUsart1Rx = FALSE;
			}
		}
		else //����ת���� һ��ʱ�� �����Ȼ���յ� ������� ���Ƕ�ת
		{
//			if (( fUsart1Rx != TRUE ) && ( fMotorPoAndNegEnd != 1))
//			{
//				Motor_Uart_Send_Data(MOTOR_CMD_STOP);			
//				rt_thread_delay(300);
//				printf("1...fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
//			}
//			
//			if (( fUsart1Rx != TRUE ) && ( fMotorPoAndNegEnd != 1))
//			{
//				Motor_Uart_Send_Data(MOTOR_CMD_STOP);			
//				rt_thread_delay(300);
//				printf("2....fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
//			}
			printf("fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
			if (fMotorPoAndNegEnd)	
			{			
				Motor_Stop();

				Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);
				rt_thread_delay(300);
				Check_Motor_Step(MOTOR_CMD_GO);
				printf("2 ��ת5s ...\r\n");
				rt_thread_delay(RT_TICK_PER_SECOND * 5);	// ��ת5s
			
				Motor_Stop();
				rt_thread_delay(300);
				Check_Motor_Step(MOTOR_CMD_CHANGE_DIR);		
				
				SET_WORK_STATE_ERR;
				fMotorPoAndNegEnd = 1;
			}
			else 
			{
				Check_Motor_Step(MOTOR_CMD_GO);	
			}
			
			printf("fUsart1Rx = %d ,fMotorPoAndNegEnd = %d\r\n",fUsart1Rx,fMotorPoAndNegEnd);
			return fMotorPoAndNegEnd;											
		}
	}
}

/**
  * @brief   �������ת
  * @param   void
  * @retval  1--��ת  0--����ת
  */
uint8 Check_Motor_Lock_State(void)
{
	//�Ѿ��жϹ���Ƶ����ת��
	if(fMotorLock == 1)
	{
		Motor_Stop();
		SET_WORK_STATE_ERR;
		return 1;
	}
	
	//û���жϹ���ת���� �����ж϶�ת���� ���������תһ��
	if(Check_Motor_CurErr_State() == 1)
	{
		fMotorLock = 1;
		return 1;
	}
	else
	{
		return 0;
	}	
}

/**
  * @brief   ����ͨѶ��������ʱ����У�麯��   (USART1_IRQHandler�е���)
  * @param   void
  * @retval  1--ͨ�ųɹ� ���ұ�Ƶ���й��� 
  */
uint8 Motor_Check_ErrCode(uint8*pstr)
{
	uint8 buf[4];
	//��ʽ����
	buf[0] = *pstr;
	pstr++;
	buf[1] = *pstr;
	pstr++;
	buf[2] = *pstr;
	pstr++;
	buf[3] = *pstr;
	
	if(buf[0] == MOTOR_ERR_FH)
	{
		if(buf[0] != (uint8)(~buf[2]))
		{
			return 0;
		}
		if(buf[1] != (uint8)(~buf[3]))
		{
			return 0;
		}
		return 1;
	}
	return 0;
}

//------------------------��ֽ�ڵ��-------------------------------------
uint8 MotorCloseCount = 0;		// ����رռ������ظ��ر��������ٹر���ֽ��
/**
  * @brief   ��ֽ��ֱ��������  10ms
  * @param   void
  * @retval  void
  */
void Check_Motor_Limit_State(void)
{
	static uint8 ForwardCounter = 0;
	static uint8 BackCounter = 0;
	static uint16 CurErrCounter = 0;
	
	//���� �������̧��  
	if (gCurAdcErr == TRUE)
	{	
		if (DC_MOTOR_FORWARD_OK == 0)
		{
			CurErrCounter++;
			if(CurErrCounter >= 3)	// 30ms
			{
				DC_MOTOR_CLOSE;
				DcMotorCloseFlag = 0xff;
			}	
			if (CurErrCounter >= 500)	// 5s �����¹ر���ֽ��
			{
				MotorCloseCount++;
				gCurAdcErr = FALSE;
				printf("\r\n two close motor\r\n");
				
				if (MotorCloseCount >= MOTOR_CLOSE_COUNT_MAX)		// ����2�ιر���ֽ�ڣ��ر�ʧ���򲻹ر�
				{
					DcMotorCloseFlag = 0xff;
				}
				else 
				{
					PAPER_CLOSE;						// ��ֽ�ڹر�				
				}											
			}
		}
		else
		{
			CurErrCounter = 0;
		}
		return;//�������ȼ���	
	}
	
	//��������Ƿ�λ
	if ((gGetWorkState & WORK_STAT_BIT) != WORKSTATE_CLOSE)
	{
		if (DC_MOTOR_FORWARD_OK == 0)
		{
			ForwardCounter++;
			if (ForwardCounter >= 3)	// 30ms
			{		
				DC_MOTOR_CLOSE;
				ForwardCounter = 200;	
				DcMotorCloseFlag = 0xff;
			}
		}
		else
		{
			ForwardCounter = 0;
		}
	}
	
	//�ػ�����Ƿ�λ
	if ((gGetWorkState & WORK_STAT_BIT) == WORKSTATE_CLOSE)
	{
		if (DC_MOTOR_BACK_OK == 0)
		{
			BackCounter++;
			if (BackCounter >= 3)	// 30ms
			{
				DC_MOTOR_CLOSE;
				BackCounter = 200;
				DcMotorCloseFlag = 0xff;
			}
		}
		else
		{
			BackCounter = 0;	
		}
	}	
}

/**
  * @brief   PWM�����ʼ�� ,MT_INTʹ�ܽ�		
  * @param   arr���Զ���װֵ
						 psc��ʱ��Ԥ��Ƶ��
  * @retval  void
  */
static void TIM3_PWM_Init(uint16 arr,uint16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3������ӳ��  TIM3_CH2->PB5    
 
   //���ø�����Ϊ�����������,���TIM3 CH2��PWM���岨��	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO
 
   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; 			//�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
 
	TIM_Cmd(TIM3, DISABLE);  //ʧ��TIM3
	//TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
}

/**
  * @brief   ֱ�����IO��
	* @note 	 PB6,PB9������ֽ��λ��
						 PC8,PC9 ���Ƶ������ת
						 PB5 �������ʹ�ܽ�
  * @param   void						
  * @retval  void
  */
void Motor_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	//HX1 -- PC9
	//HX2 -- PC8  �������ת
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//MT_ENA ���ʹ�ܶ�
	TIM3_PWM_Init(99,719);	// PWMƵ�ʣ�72Mhz/720/100 = 1Khz	
	
	DC_MOTOR_CLOSE;				//�ر�ֱ�����	  
	
	//V1.0.0   PB3,PB4������ֽ��λ��
	//V2.0.0   PB6,PB9������ֽ��λ��
#if (VERSION == 1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
#else 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
#endif
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
} 








