#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"

#define MOTOR_CLOSE_COUNT_MAX		2			// ��ֽ���ظ��رմ���

//��Ƶ��ͨ��
#define MOTOR_COMMUN_PROT_FH    0x4C
#define MOTOR_ERR_FH            0x4B

#define MOTOR_CMD_STOP          0x80	//ֹͣ
#define MOTOR_CMD_GO            0x40	//ת
#define MOTOR_CMD_CHANGE_DIR    0x20	//�ı䷽��
#define MOTOR_CMD_RESET         0x00  //��λ

/***********************	�����������  **************************/
// ��ֽ�ڴ� 
#define PAPER_OPEN							{ DcMotorCloseFlag = 0; MotorCloseCount = 0;\
																	if (DC_MOTOR_FORWARD_OK == 1)	\
																	{	MOTOR_ONE_GO_FORWARD;	DC_MOTOR_OPEN;}}	

// ��ֽ�ڹر�
#define PAPER_CLOSE							{	if (DC_MOTOR_BACK_OK == 1)	\
																	{	DcMotorCloseFlag = 1;		MotorDelay = 0;		MOTOR_ONE_GO_BACK;		DC_MOTOR_OPEN;	}	}	

//ֱ��������� 
#define MOTOR_ONE_GO_FORWARD    {	GPIO_SetBits(GPIOC, GPIO_Pin_9);		GPIO_ResetBits(GPIOC, GPIO_Pin_8);}
#define MOTOR_ONE_GO_BACK				{	GPIO_ResetBits(GPIOC, GPIO_Pin_9);	GPIO_SetBits(GPIOC, GPIO_Pin_8);}
#define MOTOR_ONE_STOP			 		{	GPIO_SetBits(GPIOC, GPIO_Pin_9);		GPIO_SetBits(GPIOC, GPIO_Pin_8);}

// MT_ENA
#if (VERSION == 1)
#define DC_MOTOR_OPEN    		 		{	TIM_SetCompare2(TIM3, 70);	TIM_Cmd(TIM3, ENABLE);}	// ռ�ձ�100 Ԥ�趨70������Ԥ�趨ֵ����͵�ƽ
#else 
#define DC_MOTOR_OPEN    		 		{	TIM_SetCompare2(TIM3, 100);	TIM_Cmd(TIM3, ENABLE);}	// ռ�ձ�100 Ԥ�趨70������Ԥ�趨ֵ����͵�ƽ
#endif 

#define DC_MOTOR_CLOSE    	 		{	TIM_SetCompare2(TIM3, 0);	 	TIM_Cmd(TIM3, DISABLE); MOTOR_ONE_STOP;}
	
// ��ֽ�ڵ�����Կ��� 
#if (VERSION == 1)
#define DC_MOTOR_FORWARD_OK    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)
#define DC_MOTOR_BACK_OK       GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
#else 
#define DC_MOTOR_FORWARD_OK    	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)
#define DC_MOTOR_BACK_OK       	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)
#endif 

/***********************				 **************************/
#define MOTOR_CUR_ERR_MAX        (int8)2

extern uint8 Usart1RxData[4];
extern uint8 fUsart1Rx;

extern uint8 fMotorLock;
extern uint8 fMotorPoAndNegEnd;
extern uint8 DcMotorCloseFlag;

extern uint8 MotorCloseCount;

/***********************	��Ƶ��	 **************************/
uint8 Check_Motor_Lock_State(void);
uint8 Check_Motor_Step(uint8 cmd);

uint8 Motor_Check_ErrCode(uint8*pstr);

/***********************	��ʼ��	 **************************/
void Motor_GPIO_Init(void);
void Motor_Thread_Init(void);

/***********************	ֱ�����	 **************************/
void Check_Motor_Limit_State(void);
uint8 Motor_Stop(void);
uint8 Motor_Stop_Reset(void);

#endif


