#include "work.h"

static struct rt_thread Work_Thread;
static struct rt_thread LED_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t Work_stack[512];
rt_uint8_t LED_stack[256];

/**
  * @brief  �����رգ��������
  * @param  void
  * @retval None
  */
void Para_Init(void)
{	
	//���յ���ֽ���ָ�� �����ʱ
	gMotorDelayStopCounter = 0;

	gMcuWorkStep = MCU_SYS_CLOSE;

	fState &= (MOTOR_ERR | WEIGHER_ERR);
	
	SET_WORK_STATE_CLOSE;
	
	fSendWeight = FALSE;
	gSendWeightClass = 0xff;//0--�ϴ��������� 1--�ϴ��ܹ�����
}														
														
/**
  * @brief  �������У����ƴ���
  * @param  void
  * @retval None
  */
static void Check_State(void)
{
	uint8 buf = 0;
	
	/**************************************************************/
	/*************************�쳣����*****************************/
	/**************************************************************/
	//���ӳ�ͨ�Ų��ɹ�
	buf = fState & WEIGHER_ERR;
	if (buf == WEIGHER_ERR)
	{
		if (gAckPcErr == FALSE)
		{
			//���ڹ���״̬
			SET_WORK_STATE_ERR;
			//�����ع���״̬
			CLEAR_F_STATE;

			if (gSendStateCounter >= SEND_STATE_TIM)
			{
				gSendStateCounter = 0;

				// Usart2SendData[0] = 0x01;		// ���ӳ�ͨѶʧ��
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("���͵��ӳ�ͨѶ����..\r\n");
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// ������ʼ���У��������
				case MCU_GET_CUT_PAPER_CMD:		//�������ʼ��ֽ��
				case MCU_CUT_PAPER:						//�ȴ�����ֽ��ɡ� 
				case MCU_GET_CUT_DOWM_CMD:		//�յ�����ֽ��ɡ� -> �����ʱ	
				case MCU_GET_WEIGHT_TO_SEND:	//���� 			
				case MCU_TIME_CUT_DOWN:				//�ϴ�����ʱ
								Motor_Stop();								
				
				// û�п�ʼ���У�ֱ�ӹػ�
				case MCU_RUN_PROCESS_OK:			//�����һ������			
				case MCU_SYS_OPEN:						//ɨ���ά��
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE;					//ֹͣ����			
								fMcuOpen = FALSE;											
											
								PAPER_CLOSE;						// ��ֽ�ڹر�		
								FAN_CLOSE;
								printf("���ӳ�ͨѶ���Ϲػ�\r\n");
					break;
						
				default:
					 printf("ERROR: WEIGHER_ERR...\r\n");
					break;
			}
		}
		return;
	}
	
	// ��Ƶ��ͨѶ����
	buf = fState & MOTOR_ERR;
	if (buf == MOTOR_ERR)
	{
		if (gAckPcErr == FALSE)
		{
			//���ڹ���״̬
			SET_WORK_STATE_ERR;
			//�����ع���״̬
			CLEAR_F_STATE;

			//ÿ��2S ����һ��ERR
			if (gSendStateCounter >= SEND_STATE_TIM)
			{		
				gSendStateCounter = 0;
				
				// Usart2SendData[0] = 0x02;		// ��Ƶ��ͨѶ����
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("���ͱ�Ƶ��ͨѶ����..\r\n");			
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// ������ʼ���У��������
				case MCU_GET_CUT_PAPER_CMD:		//�������ʼ��ֽ��
				case MCU_CUT_PAPER:						//�ȴ�����ֽ��ɡ� 			
				case MCU_TIME_CUT_DOWN:				//�ϴ�����ʱ						
								//Motor_Stop();
								gMotorDelayStopCounter = 0;					// Ӧ�ȴ�һ��10s�ٽ��г���
								fState |= SHREDDER_CUT_OK;
				
				case MCU_GET_WEIGHT_TO_SEND:	//���� 
				case MCU_GET_CUT_DOWM_CMD:		//�յ�����ֽ��ɡ� -> �����ʱ	
								goto	_SHREDDER_CUT_OK_FUN;							
												
				// û�п�ʼ���У�ֱ�ӹػ�
				case MCU_RUN_PROCESS_OK:			//�����һ������
				case MCU_SYS_OPEN:						//ɨ���ά��
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE; 	//ֹͣ����			
								fMcuOpen = FALSE;												
											
								PAPER_CLOSE;						// ��ֽ�ڹر�				
								FAN_CLOSE;
								printf("��Ƶ��ͨѶ���Ϲػ�\r\n");
					break;
							
				default:
						printf("ERROR: MOTOR_ERR...\r\n");
					break;
			}
		}
		return;
	}
	
	// �ⲿ��Ƶ������
	if (gEnternErrFlag == TRUE && fMcuOpen == TRUE)
	{
		if (gAckPcErr == FALSE)
		{
			//���ڹ���״̬
			SET_WORK_STATE_ERR;
			//�����ع���״̬
			CLEAR_F_STATE;
			
			//ÿ��2S ����һ��ERR
			if (gSendStateCounter >= SEND_STATE_TIM)
			{
				gSendStateCounter = 0;

				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_FAULT, 0);
				printf("���ͱ�Ƶ������..\r\n");		
			}
		}
		else 
		{
			switch (gMcuWorkStep)
			{
				case MCU_SYS_CLOSE:
					break;
				
				// ������ʼ���У��������
				case MCU_GET_CUT_PAPER_CMD:		//�������ʼ��ֽ��
				case MCU_CUT_PAPER:						//�ȴ�����ֽ��ɡ� 		
				case MCU_TIME_CUT_DOWN:				//�ϴ�����ʱ
								Motor_Stop();		
								gMotorDelayStopCounter = 0;					// Ӧ�ȴ�һ��10s�ٽ��г���
								fState |= SHREDDER_CUT_OK;
				
				case MCU_GET_WEIGHT_TO_SEND:	//���� 	
				case MCU_GET_CUT_DOWM_CMD:		//�յ�����ֽ��ɡ� -> �����ʱ	
								goto	_SHREDDER_CUT_OK_FUN;								
				
				// û�п�ʼ���У�ֱ�ӹػ�
				case MCU_RUN_PROCESS_OK:			//�����һ������
				case MCU_SYS_OPEN:						//ɨ���ά��		
								gMcuWorkStep = MCU_SYS_CLOSE;
								SET_WORK_STATE_CLOSE; //ֹͣ����			
								fMcuOpen = FALSE;												
																
								PAPER_CLOSE;					// ��ֽ�ڹر�				
								FAN_CLOSE;
								printf("��Ƶ������ػ�");
					break;
							
				default:
						printf("�ⲿ��Ƶ������...\r\n");
					break;
			}
		}
		return;
	}
	
	CLEAR_WORK_STATE_ERR;		// �����������
	
	// ��ת
	buf = fState & MOTOR_CUR_ERR;
	if (buf == MOTOR_CUR_ERR && fMcuOpen == TRUE)
	{
		printf("��ת...\r\n");
		
		if (Check_Motor_Lock_State() == 1)//��Ȼ�Ƕ�ת
		{	
			if (gAckPcErr == FALSE)
			{
				// �����ع���״̬
				CLEAR_F_STATE;

				if (gSendStateCounter >= SEND_STATE_TIM)		// ÿ��2S ����һ��			
				{			
					gSendStateCounter = 0;					
					printf("���͹���..\r\n");					
					Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_FAULT, 0);
				}			
			}		
			else 
			{
				gAckPcErr = FALSE;
				fState &= ~MOTOR_CUR_ERR;	
				fState |= SHREDDER_CUT_OK;				
				gMotorDelayStopCounter = MOTOR_DELAY_STOP_MAX;		
			}
		}
		else //����ת
		{
			fState &= ~MOTOR_CUR_ERR;			
		}
		
		return;
	}
	gAckPcErr = FALSE;
	
	// ֽ������
	buf = fState & BIN_FULL;
	if (buf == BIN_FULL)		//������ֽ״̬
	{
		if (gAckBinFull == FALSE)
		{
			//�����ع���״̬
			CLEAR_F_STATE;
			
			//��ʱ2s ����һ��
			if (gSendStateCounter >= SEND_STATE_TIM) 
			{
				gSendStateCounter = 0;

				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_BIN_FULL, 0);
				printf("���ͻ���Ͱ����..\r\n");
//				Motor_Stop();
			}
		}
		else 
		{
//			fState &= ~BIN_FULL;
//			fState |= SHREDDER_CUT_OK;				
//			gMotorDelayStopCounter = 0;				// ����Ͱ����Ӧ�ȴ�һ��10s�ٽ��г���
//			Motor_Stop();
		}		
	}
	
	//
	if (fMcuOpen != TRUE)	
	{
		fMotorPoAndNegEnd = 0;//0--û�ж�ת��  1--��ת��
		fMotorLock = 0;				//����ȷ�϶�ת  0--û�ж�ת 1--��ת��
		gEnternErrFlag = 0;
		return;	//�ػ�	
	}		
	/**************************************************************/
	/***************************�������***************************/
	/**************************************************************/
	// ʹ����ֽ
	buf = fState & SHREDDER_CUT;		
	if (buf == SHREDDER_CUT)
	{
		fState &= ~SHREDDER_CUT;
		
		if (Check_Motor_Step(MOTOR_CMD_GO) == 1) //������ֽ״̬
		{		
			gMcuWorkStep = MCU_CUT_PAPER;			//�������� ��ֽ״̬--���յ���ֽ��� ֮���ʱ���
			printf("ʹ����ֽ..\r\n");
		}
		else  //ͨ�Ŵ���err ���߶�ת
		{
			printf("ʹ����ֽʧ��..\r\n");
		}
		return;
	}
	
_SHREDDER_CUT_OK_FUN:	
	// ��ֽ���
	buf = fState & SHREDDER_CUT_OK;
	if (buf == SHREDDER_CUT_OK)
	{	
		if (gMotorDelayStopCounter >= MOTOR_DELAY_STOP_MAX)	//�ȴ�10S ��ֽ
		{
			fState &= ~SHREDDER_CUT_OK;
			
			if(Motor_Stop() == 1)
			{
				printf("��ֽ���\r\n");
				
				// �������״̬
				fState  |= GET_WEIGHT_AND_SEND;
				gMcuWorkStep = MCU_GET_WEIGHT_TO_SEND;
			}
			else // ���߶�ת
			{
				printf("��ת..\r\n");
			}			
		}
		else
		{		
			gMcuWorkStep = MCU_GET_CUT_DOWM_CMD;	//MCU���ڵ��10S��ʱ��
		}
	}

	// ��ȡ���ӳ�����
	buf = fState & GET_WEIGHT_AND_SEND;
	if(buf == GET_WEIGHT_AND_SEND)
	{
		return;
	}

	// ʹ���ϴ�����
	if (fSendWeight == TRUE)
	{
		SET_WORK_STATE_WORK;
		//ÿ��1S ����һ��
		if(gSendStateCounter >= 100)
		{
			gSendStateCounter = 0;
			//����
			if (gSendWeightClass == 0)
			{
				Usart2SendData[0] = (uint8)(gOneWeightData >> 8);
				Usart2SendData[1] = (uint8)gOneWeightData;
				printf("���͵������� 0x%x, %d\r\n", gOneWeightData, gOneWeightData);
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_ONE_WEIGHT, 2);
			}
			else if (gSendWeightClass == 1)//������
			{
				Usart2SendData[0] = (uint8)(gAllWeightData >> 8);
				Usart2SendData[1] = (uint8)gAllWeightData;
				printf("���������� 0x%x, %d\r\n", gAllWeightData, gAllWeightData);
				Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_ALL_WEIGHT, 2);
			}
		}
		return;
	}
	
	// �ȴ�һ��ʱ�� û����Ϊȷ�� ��ֽ��� ֪ͨ��λ��Ϊ��ʼ����ʱ
	// ��CheckCmd() ��� TIME_CUT_DOWN
	buf = fState & TIME_CUT_DOWN;
	if (buf == TIME_CUT_DOWN)
	{
		//ÿ��1S ����һ��		
		if (gSendStateCounter >= (SEND_STATE_TIM/2))
		{
			gSendStateCounter = 0;
			Android_Uart_Send_Data(&Usart2SendData[0], SHREDDER_ACK_COUNT_DOWN, 0);
			printf("����ʱ..\r\n");
		}
		return;
	}	
}

/**
  * @brief  Work_Thread_entry
  * @param  void  
  * @retval void
  */
void Work_Thread_entry(void *parameter)
{	
	uint8 time = 0;
	uint16 countTime = 0;
		
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/100);	// 10ms����һ��
	
		Check_State();
		
		if (fMcuOpen != TRUE)		//�ػ�
		{					
			Para_Init();//��ղ���
			
			if (countTime++ >= 500)	// 5s	ֹͣһ�µ��
			{
				countTime = 0;
				Motor_Stop_Reset();						
			}
		}
		else 
		{
			if (time++ > 100) { time = 0; Check_Motor_Step(MOTOR_CMD_RESET);}
		}
	}
}


/**
  * @brief  Printf_Thread_entry
  * @param  void  
  * @retval void
  */
void LED_Thread_entry(void *parameter)
{
		uint8 i = 0;
		while(1)
		{
				if(fMcuOpen)
					rt_thread_delay(RT_TICK_PER_SECOND/4);	// 250ms����һ��
				else 
					rt_thread_delay(RT_TICK_PER_SECOND/2);	// 500ms����һ��
				
				IWDG_Feed();	//ι��			
				Led_Run_Show();	
				
				Lock_Check();
				
				if (++i >= 4)
				{
					i = 0;					
//					printf("fState = %x\r\n", fState);
					printf("INFRARE_STATE = %d\r\n", INFRARE_STATE);		
//					printf("gMcuWorkStep = %d\r\n", gMcuWorkStep);		
				}
		}
}

/**
  * @brief  Work_Thread_Init: ���ȼ�18 ʱ��Ƭ20
						LED_Thread_Init : ���ȼ�30 ʱ��Ƭ20
  * @param  void  
  * @retval void
  */
void Work_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&Work_Thread,
													"Work",
													&Work_Thread_entry,
													RT_NULL,
													&Work_stack[0],
													sizeof(Work_stack),
													18,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&Work_Thread);
				printf("Work_Thread start success...\r\n");
		}	
		
		err_t =  rt_thread_init(&LED_Thread,
													"LED",
													&LED_Thread_entry,
													RT_NULL,
													&LED_stack[0],
													sizeof(LED_stack),
													30,
													20);	
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&LED_Thread);
				printf("Printf_Thread start success...\r\n");
		}												
}








