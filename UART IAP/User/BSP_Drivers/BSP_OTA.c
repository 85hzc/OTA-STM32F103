#include "BSP_OTA.h"

uint8 OTA_RxBuf[OTA_RX_LEN] __attribute__ ((at(0X20001000)));	//���ջ���,���OTA_RX_LEN���ֽ�,��ʼ��ַΪ0X20001000. 

struct OTA_struct OTA;

uint16 Usart_Rx_Cnt, oldCount, AppLenth;

iapfun jump2app; 


/**
  * @brief  ��ת��Ӧ�ó����
  * @param  appxaddr:�û�������ʼ��ַ.
  * @retval None	  
  */
static void Iap_Load_App(uint32 Addr)
{
	if(((*(vu32*)Addr) & 0x2FFE0000) == 0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app = (iapfun)*(vu32*)(Addr + 4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)					
		__set_MSP(*(vu32*) Addr);  	//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP. 
	}
}	

/**
  * @brief  ���桢ִ�й̼�
  * @retval 0  �������ݷǹ̼�
  */
static uint8 Update_Firmware(uint32 addr)
{
		if(((*(vu32*)(addr + 4)) & 0xFF000000) == 0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.																			
		{	 
			printf("��ʼִ��FLASH�û�����!!\r\n");
			Iap_Load_App(addr);			//ִ��FLASH APP����	
		}
		
		printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");

		OTA.RunStatu = OTA_WAIT;	
		Usart_Rx_Cnt = 0;
		oldCount = 0;	
		AppLenth = 0;
		
		return 0;
}

/**
  * @brief  APPд��FLASH
  * @param  appxaddr:Ӧ�ó������ʼ��ַ
  * @param 	appbuf:  Ӧ�ó���CODE.
  * @param	appsize: Ӧ�ó����С(�ֽ�).
  * @retval None	  
  */
static void IAP_Write_APP(uint32 App_Addr, uint8 *App_Buf, uint32 App_Size)
{
	uint32 t;
	uint16 buf[512];
	uint16 i = 0;
	uint8  *dfu = App_Buf;
	
	printf("Static write flash...\r\n");
	printf("write addr = 0x%x\r\n",App_Addr);
	for(t = 0; t < App_Size; t += 2)
	{			
		buf[i]  = (uint16)dfu[1] << 8;
		buf[i] += (uint16)dfu[0];	  
		
		dfu += 2;		//ƫ��2���ֽ�
		i++;
		
		if(i == 512)
		{
			i = 0;
			BSP_FLASHWrite(App_Addr, buf, 512);
			App_Addr += 1024; //ƫ��2048  32 = 4*8.����Ҫ����4.
		}
	}
	
	if(i) BSP_FLASHWrite(App_Addr, buf, i);//������һЩ�����ֽ�д��ȥ. 
	App_Addr += i*2;
	
	printf("Write flash OK...\r\n");
}


/**
  * @brief  ���չ̼�,��д�뵽flash
  * @param  void 
  * @retval NONE
  */
void Receive_Firmware(void)
{
	if (OTA.RunStatu == OTA_RXD_UART_OK)
	{
		IAP_Write_APP(OTA_ADDR, OTA_RxBuf, AppLenth);
		Update_Firmware(OTA_ADDR);
	}
}

/**
  * @brief  check_Firmware
	* @note   ���̼��Ƿ�������
	* @param  void 
  * @retval NONE
  */
void check_Firmware(void)
{
	static uint16 oldCount = 0;
	if (Usart_Rx_Cnt)
	{
		if (Usart_Rx_Cnt == oldCount)
		{
			AppLenth = Usart_Rx_Cnt;
			Usart_Rx_Cnt = 0;
			oldCount = 0;

			printf("�û�����������!\r\n");
			printf("���볤��:%dBytes\r\n",AppLenth);
			OTA.RunStatu = OTA_RXD_UART_OK;
		}
		else
			oldCount = Usart_Rx_Cnt;
	}
}

/**
  *	@brief  ���ع�ѡ�������ز��֣��ظ��������Σ����ɲ���flashд���Ƿ�ɹ�
	* @note 	�ϵ�ʱ����ȡflash����
  * @retval None
  */
void BSP_Flash_Init(void)
{
	uint16 flag = 0;
	
//	Run_Bootloader();
	
	flag = BSP_FLASHReadHalfWord(OTA_FLAG_ADDR);
	
	switch (flag)
	{
		case FLAG_FIRMWARE:	
						printf("\r\n");
						printf("----- Have Firmware -----\r\n");
						printf("----- Run Firmware! -----\r\n");
						printf("\r\n");
						Iap_Load_App(OTA_ADDR);
					break;
		
		case FLAG_NO_FIRMWARE:
						printf("\r\n");
						printf("----- One Download -----\r\n");
						printf("----- No Firmware! -----\r\n");
						printf("\r\n");
					break;
		
		case FLAG_RX_FIRMWARE:
						printf("\r\n");
						printf("----- Update Firmware -----\r\n");
						printf("\r\n");
						
						flag = FLAG_NO_FIRMWARE;
						BSP_FLASHWrite(OTA_FLAG_ADDR, &flag, 1);
		
						Android_Uart_Send_Data(&Usart2SendData[0], OTA_VERSION_UPDATE, 0);
					break;
		
		default:
						flag = FLAG_NO_FIRMWARE;
						BSP_FLASHWrite(OTA_FLAG_ADDR, &flag, 1);
						printf("\r\n");
						printf("----- flash error, init -----\r\n");
						printf("\r\n");
					break;
	}
}


