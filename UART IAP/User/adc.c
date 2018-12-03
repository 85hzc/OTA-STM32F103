#include "adc.h"

#define CUR_ADC_MAX  10

#if (VERSION == 1)
#define 	ADC_ADD_VALUE		50			
#else 
#define 	ADC_ADD_VALUE		30
#endif 

static uint16 CurAdcValuebuf[CUR_ADC_MAX]={0,0,0,0,0,0};
static uint16 gCurAdcValue,CUR_ADC_VALUE_MAX = 1800;
static uint16 sum = 0;
static uint16	AdcMax = 0,AdcMin = 0xffff;
static uint8 gCurAdcCounter = 0;

uint16 MotorDelay = 0;	// ��ֽ�ڵ������˲����ʱ
uint16 ADC_ConvertedValue[2] = {0,0};
uint8 gCurAdcErr = FALSE;   // TRUE ��ֽ��ֱ���������

/**
  * @brief  10ms����һ�Σ�ADC_Thread_entry �е���
  * @param  void  
  * @retval void
  */
static void Get_Cur_Adc_Value(void)
{
	CurAdcValuebuf[gCurAdcCounter] = ADC_ConvertedValue[0];
	
	if(CurAdcValuebuf[gCurAdcCounter] != 0)
	{	
		sum += CurAdcValuebuf[gCurAdcCounter];
		
		// Get ADC Max && Min value
		if (AdcMax < CurAdcValuebuf[gCurAdcCounter]) AdcMax = CurAdcValuebuf[gCurAdcCounter];
		if (AdcMin > CurAdcValuebuf[gCurAdcCounter]) AdcMin = CurAdcValuebuf[gCurAdcCounter];
		
		gCurAdcCounter++;
		if(gCurAdcCounter >= CUR_ADC_MAX)
		{	
			sum -= AdcMin;
			sum -= AdcMax;
			gCurAdcValue = sum / (CUR_ADC_MAX - 2);		
			//printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);
			
			AdcMax = sum = 0;
			AdcMin = 0xFFFF;
			gCurAdcCounter = 0;
			
			if((gCurAdcValue >= CUR_ADC_VALUE_MAX) || (gCurAdcValue <= CUR_ADC_VALUE_MIN))
			{
				printf("��������  ");
				printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);
				
				//���� �������̧��
				gCurAdcErr = TRUE;
				DcMotorCloseFlag = 0;	 // �����̧���رն�ȡ����					
				
				DC_MOTOR_CLOSE;
				DC_MOTOR_OPEN;
				MOTOR_ONE_GO_FORWARD;				
			} 
		}  						
	}
}


/**
  * @brief  10ms
  * @param  void  
  * @retval void
  */
void ADC_Thread_entry(void *parameter)
{
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/100);
		
		switch(DcMotorCloseFlag)
		{
			case 1:		// ��ֽ�����ڹر�
						if (MotorDelay >= 50)
						{	
							Get_Cur_Adc_Value();//������		
							MotorDelay = 50;	
//							printf("gCurAdcValue = %d  %d  %d\r\n", gCurAdcValue,CUR_ADC_VALUE_MAX,gCurAdcErr);							
						}
//						if (MotorDelay == 0) printf("gCurAdcValue = %d  %d \r\n", gCurAdcValue,CUR_ADC_VALUE_MAX);
						break;		
						
			case 0xff:		// ��ֽ���޶���
						Get_Cur_Adc_Value();
						if (gCurAdcValue > 200)
							CUR_ADC_VALUE_MAX = gCurAdcValue + ADC_ADD_VALUE;			
			 			break;
			
			case 0:			// ��ֽ�����ڴ�
			default:		
						break;
		}
		// �����λ���ؼ��
		Check_Motor_Limit_State();
		
		// ��ֽ�ں�����
		Check_Infrared_Every_State();
		
#if (VERSION == 1) 		
		fBody &= ~BODY_HAVE;
#else 		
		// �����Ӧ���
		Check_Body_State();		
#endif 		
	}
}

static struct rt_thread ADC_Thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t ADC_stack[256];

/**
  * @brief  ADC_Thread_Init: ���ȼ�15 ʱ��Ƭ20
  * @param  void  
  * @retval void
  */
void ADC_Thread_Init(void)
{
		rt_err_t err_t;
		
		err_t = rt_thread_init(&ADC_Thread,
													"Work",
													&ADC_Thread_entry,
													RT_NULL,
													&ADC_stack[0],
													sizeof(ADC_stack),
													15,
													20);
													
		if (err_t == RT_EOK)
		{
				rt_thread_startup(&ADC_Thread);
				printf("ADC_Thread start success...\r\n");
		}	
}

/**
  * @brief  ADC_GPIO_Init:	PA7
  * @param  void  
  * @retval void
  */
static void ADC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		  //ģ������
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
}

/**
  * @brief  ���β���ʱ�� 1/9Mhz * (55.5+12.5) = 7.5us
  * @param  void  
  * @retval void
  */
static void ADC_Mode_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	/* DMA channel1 configuration */
	DMA_DeInit(DMA1_Channel1);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	 			//ADC��ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;	//�ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//������Ϊ����Դ
	DMA_InitStructure.DMA_BufferSize = 1;							    //���δ���2������
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	   //�����ַ�ر���������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  				//�ڴ��ַ��������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//16λ���������
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//16λ���������
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;										//ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* Enable DMA channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration */	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;			//����ADCģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	 				//��ֹɨ��ģʽ��ɨ��ģʽ���ڶ�ͨ���ɼ�
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;			//��������ת��ģʽ������ͣ�ؽ���ADCת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//��ʹ���ⲿ����ת��
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	//�ɼ������Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	 								//Ҫת����ͨ����Ŀ2
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/*����ADCʱ�ӣ�ΪPCLK2��8��Ƶ����9MHz*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	/*����ADC1��ͨ��  55.5���������� */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7 , 1, ADC_SampleTime_55Cycles5);	//���β���ʱ�� 1/9Mhz * (55.5+12.5) = 7.5us

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	
	/*��λУ׼�Ĵ��� */   
	ADC_ResetCalibration(ADC1);
	/*�ȴ�У׼�Ĵ�����λ��� */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* ADCУ׼ */
	ADC_StartCalibration(ADC1);
	/* �ȴ�У׼���*/
	while(ADC_GetCalibrationStatus(ADC1));
	
	/* ����û�в����ⲿ����������ʹ���������ADCת�� */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/**
  * @brief  ADC_Config
  * @param  void  
  * @retval void
  */
void ADC_Config(void)
{
	ADC_GPIO_Init();
	ADC_Mode_Init();
}

