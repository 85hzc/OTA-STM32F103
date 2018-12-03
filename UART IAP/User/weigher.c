//���ع����Ż���
//���󣺻���Ͱֽ��ˮ�֣�һ��ʱ���ӷ�����Ӱ����һ���û�������ֵ��
//�Ż�������ʱ����ȡһ�ε���������������ӷ�����ֵ��

//���󣺻���Ͱ�õ��󣬵��ӳӻ��Ǹ�����
//�Ż���ֵΪ����ʱ���ϱ�����Ϊ0
#include "weigher.h"

const uint8 AsiiData[13] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', '+', '-'};
static uint8 Usart4RxDataBuf[9] = {0};
static int8 gOverWeightCounter = 0;//��������

uint8 Usart4RxData[9] = {0};
uint8 fUsart4Rx = 0;

uint8 gSendWeightClass = 0xff;//0--�ϴ��������� 1--�ϴ��ܹ�����
uint8 fSendWeight = FALSE;
//�Ƿ��Ѿ��ɹ����� ����Ͱ���� ����Ϣ

uint16 gAllWeightData = 0;
uint16 gOneWeightData = 0;

/**
  * @brief  ����ͨѶ����������
  * @param  
  * @retval None
  */
static void Weigher_Uart_Send_Data(uint8 dat)
{
	while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
	USART_SendData(UART4, dat);
}

/**
  * @brief  ��������ת��
  * @param  
  * @retval None
  */
static float Asii_To_Num(uint8 *asii, uint8 len)
{
	int32 buf = 0;
	uint8 i = 0, j = 0;
	uint8 dat = 0;
	
	for(i = 0; i < len; i++)
	{
		dat = (uint8)(*asii);
		
		for(j = 0; j < 13; j++)
		{
			if(dat == AsiiData[j])
			{
				if(j < 10)
				{
					buf	+= j;
					buf *= 10;
				}
				else if(j == 12) //����
				{
						return (float)0;
				}

				break;
			}
		}

		asii++;
	}
	
	return (float)(buf * 1.0 / 1000);	
}

/**
  * @brief  ��ȡ������ or ��������
  * @param  cmd ��ALL_WEIGHT_CMD��INCREASE_WEIGHT_CMD
  * @retval 0 û�д������ݣ�1  ��������
  */
static uint8 Check_Cmd_Weigher(uint8 cmd)
{
	float buf = 0;

	//����4 ���ӳ�
	if(fUsart4Rx == TRUE)
	{
		fUsart4Rx = FALSE;
		
		memset(&Usart4RxDataBuf[0], 0, sizeof(Usart4RxDataBuf));
		memcpy(&Usart4RxDataBuf[0], &Usart4RxData[0], sizeof(Usart4RxData));
		memset(&Usart4RxData[0], 0, sizeof(Usart4RxData));

		buf = Asii_To_Num(&Usart4RxDataBuf[0], 8);

		switch(cmd)
		{
			case ALL_WEIGHT_CMD://������
					gAllWeightData = (int16)(buf * 1000);
//     		 	printf("Get all weight = %d\r\n", gAllWeightData);

					//�����жϳ������ֵ
					if (gAllWeightData >= ALL_WEIGHT_FULL)
					{
						gOverWeightCounter++;
						if(gOverWeightCounter >= 2)
						{
							gOverWeightCounter = 3;
						}
					}
					else if (gAllWeightData < ALL_WEIGHT_NO_FULL)
					{
						//û�г������ֵ
						gOverWeightCounter--;
						if(gOverWeightCounter <= 0)
						{
							gOverWeightCounter = 0;
						}
					}
				break;

			case INCREASE_WEIGHT_CMD:	//��������
					gOneWeightData = (int16)(buf * 1000);
//					printf("Get one buf = %d\r\n", gOneWeightData);
				break;

			default:
				break;
		}		
		return 1;
	}
	return 0;
}

/**
  * @brief  ��ȡһ�ε�������
  * @param 	void
  * @retval 0--ͨ��ʧ�� 1--�ɹ�
  */
static uint8 Get_Weight_One(void)
{
	uint8 counter = 0;
	uint8 errcount = 0;
	
#if !WEIGHER_SWITCH		
	return 1;
#else 	
	
	//����
	Weigher_Uart_Send_Data(INCREASE_WEIGHT_CMD);
	counter = 0;
	errcount = 0;
	
	while(1)
	{	
		rt_thread_delay(100);		
		
		if (Check_Cmd_Weigher(INCREASE_WEIGHT_CMD) == 1)
		{
			break;
		}

		counter++;
		if(counter >= 5)			// 500ms
		{
			errcount++;
			Weigher_Uart_Send_Data(INCREASE_WEIGHT_CMD);
			counter = 0;
		}

		if(errcount >= 4)			// 500*4 = 2s
		{
			printf("���ӳ�ͨѶ��ʱ...\r\n");
			return 0;                            //�˳���ѭ��
		}
	}
	return 1;
	
#endif	
}

/**
  * @brief  ��ȡһ��������
  * @param 	void
  * @retval 0--ͨ��ʧ�� 1--�ɹ�
  */
static uint8 Get_Weight_All(void)
{
	uint8 counter = 0;
	uint8 errcount = 0;
	
#if !WEIGHER_SWITCH		
	return 1;
#else 			
	
	Weigher_Uart_Send_Data(ALL_WEIGHT_CMD);
	counter = 0;
	errcount = 0;
	
	while(1)
	{
		rt_thread_delay(100);		
		
		if (Check_Cmd_Weigher(ALL_WEIGHT_CMD) == 1)
		{
			break;
		}

		counter++;
		if (counter >= 4)		// �ȴ�400ms
		{
			errcount++;
			Weigher_Uart_Send_Data(ALL_WEIGHT_CMD);
			counter = 0;
		}

		if (errcount >= 4)		// 400 * 4  = 1.6s
		{	
			printf("���ӳ�ͨѶ��ʱ...\r\n");
			return 0;                            //�˳���ѭ��
		}
	}
	
	return 1;
	
#endif	
}

/**
  * @brief  �ж��ǲ�����Ҫ�������Ͱ
	* @note   ������״̬��fState��BIN_FULL
						δ����	gOverWeightCounter <= 0
						���� δ�ϴ����ݣ�	gAckBinFull = FALSE	gOverWeightCounter >= 2
						���� �ϴ����ݣ�		gAckBinFull = TRUE
  * @retval void
  */
static void Check_Bin_State(void)
{
	static uint8 clearweight = 0;

	//����ջ���û��
	if (gOverWeightCounter <= 0)
	{
		gGetWorkState &= ~WORKSTATE_BIN_FULL;	
		gAckBinFull = FALSE;
		//���
		if (clearweight == 1)
		{
			clearweight = 0;
		}
		return;
	}

	//���� �ϴ��ɹ�
	if (gAckBinFull == TRUE)
	{
		clearweight = 1;
		return;
	}

	//���� δ�ϴ��ɹ�
	if ((gOverWeightCounter >= 2) && (gAckBinFull == FALSE))
	{
		gGetWorkState |= WORKSTATE_BIN_FULL;
		//if(fMcuOpen == TRUE)	//����
			fState |= BIN_FULL;
	}
}

/**
  * @brief  ��ȡ������  and ��������
	* @note   
  * @retval void
  */
static uint8 Get_All_One_Weight(void)
{			
		if (!Get_Weight_One())	
		{
			fState |= WEIGHER_ERR;			//���ǲ�ͨѶ�ɹ� ������
			printf("Weigher Error... gStateFlag = %x\r\n",fState);
			return 0;
		}
		else 
		{
			Get_Weight_All();
			printf("gOneWeightData = %d,gAllWeightData = %d\r\n\n",gOneWeightData,gAllWeightData);
		}	
		return 1;
}

/**
  * @brief  ����׿�������ȡ������MCU��ʱ��ȡ����������ʱ1s ��ȡ������ ��
  * @param 	void
  * @retval void
  */
static void Time_Get_Weigth(void)
{
		static uint8 WeightCount = 0,ErrCount = 0;
			
		if (Get_Weight_All())
		{
//				fState &= ~WEIGHER_ERR;
//			printf("gAllWeightData = %d\r\n",gAllWeightData);
		}
		else 
		{
				ErrCount++;
//				fState |= WEIGHER_ERR;			//���ǲ�ͨѶ�ɹ� ������
		}
		
		WeightCount++;
		if (WeightCount >= 10)		// 10�ζ��أ�2�ζ�ȡ������������Ϊ��·�ϻ���ͨѶʧ��
		{			
			if (ErrCount >= 2)	fState |= WEIGHER_ERR;			
			else fState &= ~WEIGHER_ERR;		
			
			ErrCount = 0;
			WeightCount = 0;
		}	
}

/**
  * @brief  ������ɣ���ȡ����
  * @param 	void
  * @retval void
  */
static void Get_weight_work_OK(void)
{
	uint8 buf;
	
	//��ȡ���ӳ�����
	buf = fState & GET_WEIGHT_AND_SEND;
	if(buf == GET_WEIGHT_AND_SEND)
	{
		rt_thread_delay(3000);
		if(Get_All_One_Weight() == 1)
		{
			SET_WORK_STATE_WORK;
			printf("��ȡ�������..\r\n");

			//������һ�� �ϴ�������
			fSendWeight = TRUE;
			gSendWeightClass = 0;
		}
		else  //����err
		{
			SET_WORK_STATE_ERR;					// ���ӳ�ͨѶ����				
			printf("EROR: Get Weight err...\r\n");
		}
		
		fState &= ~GET_WEIGHT_AND_SEND;
		return;
	}
}

static struct rt_thread Weigth_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t Weigth_stack[512];

/**
  * @brief  Weigth_Thread_entry
  * @param 	void
  * @retval void
  */
void Weigth_Thread_entry(void *parameter)
{
	uint8 tim1s = 0;
	uint8 fStartRead;
	
	while(1)
	{
			rt_thread_delay(RT_TICK_PER_SECOND/100);	// 10ms����һ��
 		
			Get_weight_work_OK();
		
			if (fMcuOpen == TRUE)		
			{
				if (fStartRead)					// ɨ�뿪����δ�������ʼ��ֽ��ǰ��һ������
				{
					fStartRead --;
					Get_All_One_Weight();
				}
			}
			else 
				fStartRead = 1;
			
			if (++tim1s >= 200 && !fMcuOpen)	// 2s ��ȡһ��������
			{
				tim1s = 0;			
				Time_Get_Weigth();		
				Check_Bin_State();							//�ж��Ƿ���Ҫ�������Ͱ	
			}			
	}
}

/**
  * @brief  Weigth_Thread_Init:	���ȼ�22 ʱ��Ƭ20
  * @param 	void
  * @retval void
  */
void Weigth_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&Weigth_Thread,
													"Work",
													&Weigth_Thread_entry,
													RT_NULL,
													&Weigth_stack[0],
													sizeof(Weigth_stack),
													22,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&Weigth_Thread);			
				printf("Weigth_Thread start success...\r\n\r\n");
		}	
}


