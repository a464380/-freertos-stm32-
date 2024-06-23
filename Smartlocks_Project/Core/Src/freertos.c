/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "drivers_oled.h"
#include "driver_hx1838.h"
#include "semphr.h"
#include "usart.h"
#include "driver_sound.h"
#include "tim.h"
#include <stdio.h>
#include "driver_AS608.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//���ڽ���buf
extern uint8_t RecvTmpBuf[50];
extern int lockState;
extern int addfingerstate;
extern int removefingerstate;
//����ͣ��ʱ��
int stayTime = 0; 
//i2c�ź���
static SemaphoreHandle_t i2cSemaphoreHandle_t;
//usart�ź���
static SemaphoreHandle_t usartSemaphoreHandle_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
void getSemaphore(SemaphoreHandle_t semaphoreHandle_t)
{
	xSemaphoreTake(semaphoreHandle_t, portMAX_DELAY);
}

void releaseSemaphore(SemaphoreHandle_t semaphoreHandle_t)
{
	xSemaphoreGive(semaphoreHandle_t);
}


/* USER CODE END Variables */
/* Definitions for soundTask */
osThreadId_t soundTaskHandle;
const osThreadAttr_t soundTask_attributes = {
  .name = "soundTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for infraredTask */
osThreadId_t infraredTaskHandle;
const osThreadAttr_t infraredTask_attributes = {
  .name = "infraredTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for fingerprintTask */
osThreadId_t fingerprintTaskHandle;
const osThreadAttr_t fingerprintTask_attributes = {
  .name = "fingerprintTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for bluetoothTask */
osThreadId_t bluetoothTaskHandle;
const osThreadAttr_t bluetoothTask_attributes = {
  .name = "bluetoothTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t OledTaskHandle;
const osThreadAttr_t OledTask_attributes = {
  .name = "OledTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};
void OledTask(void *argument);
/* USER CODE END FunctionPrototypes */

void soundTask1(void *argument);
void infraredTask1(void *argument);
void fingerprintTask1(void *argument);
void bluetoothTask1(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  OLED_Init();
  OLED_Clear();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  //i2c�ź���
  i2cSemaphoreHandle_t = xSemaphoreCreateCounting(1,1);
  //usart1�ź���
  usartSemaphoreHandle_t = xSemaphoreCreateCounting(1,1);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of soundTask */
  soundTaskHandle = osThreadNew(soundTask1, NULL, &soundTask_attributes);

  /* creation of infraredTask */
  infraredTaskHandle = osThreadNew(infraredTask1, NULL, &infraredTask_attributes);

  /* creation of fingerprintTask */
  fingerprintTaskHandle = osThreadNew(fingerprintTask1, NULL, &fingerprintTask_attributes);

  /* creation of bluetoothTask */
  bluetoothTaskHandle = osThreadNew(bluetoothTask1, NULL, &bluetoothTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  OledTaskHandle = osThreadNew(OledTask, NULL, &OledTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_soundTask1 */
/**
  * @brief  Function implementing the soundTask thread.
  * @param  argument: Not used
  * @retval None
  */
//��������
/* USER CODE END Header_soundTask1 */
void soundTask1(void *argument)
{
  /* USER CODE BEGIN soundTask1 */
  /* Infinite loop */
  for(;;)
  {
	  
	getSemaphore(usartSemaphoreHandle_t);
	//��������
	set_volume(20,1000);
	//broadcast_sound(13, 1000);
    releaseSemaphore(usartSemaphoreHandle_t);
	//broadcast_sound(2, 1000);
	//vTaskDelay(2000);
    //osDelay(1);
	  
	//��ɱ
	vTaskDelete(soundTaskHandle);
  }
  /* USER CODE END soundTask1 */
}

/* USER CODE BEGIN Header_infraredTask1 */
/**
* @brief Function implementing the infraredTask thread.
* @param argument: Not used
* @retval None
//ң��ģ��
*/
/* USER CODE END Header_infraredTask1 */
void infraredTask1(void *argument)
{
  /* USER CODE BEGIN infraredTask1 */
  /* Infinite loop */
  hx1838_cap_start();
  for(;;)
  {
	 //��ȡi2c�ź���
	 getSemaphore(i2cSemaphoreHandle_t);
	 //��ȡusart1�ź���
	 getSemaphore(usartSemaphoreHandle_t);
	 HX1838_demo();
	 //�ͷ�i2c�ź���
	 releaseSemaphore(usartSemaphoreHandle_t);
	 //�ͷ�usart1�ź���
	 releaseSemaphore(i2cSemaphoreHandle_t);
	 vTaskDelay(50);
    //osDelay(1);
  }
  /* USER CODE END infraredTask1 */
}

/* USER CODE BEGIN Header_fingerprintTask1 */
/**
* @brief Function implementing the fingerprintTask thread.
* @param argument: Not used
* @retval None
*/
//ָ������
/* USER CODE END Header_fingerprintTask1 */
void fingerprintTask1(void *argument)
{
  /* USER CODE BEGIN fingerprintTask1 */
  /* Infinite loop */
  //��ͣ���������
  //vTaskSuspendAll();
  uint8_t res = 1;
  printf("AS608ָ��ģ����Կ�ʼ\r\n");
  res = as608_init();
  if(res == 0)
  {
  	  printf("AS608ָ��ģ���ʼ���ɹ�\r\n");
  }
  else
   	  printf("AS608ָ��ģ���ʼ��ʧ��\r\n");
	
  for(;;)
  {
//	getSemaphore(i2cSemaphoreHandle_t);
//	OLED_PrintString(0,4,"lock:");
//	OLED_PrintSignedVal(0,6,lockState);
//	releaseSemaphore(i2cSemaphoreHandle_t);
	  
	press_FR();//ָ�ƽ���
	  
	if(addfingerstate)
	{
		Add_FR();//���ָ��,Ĭ��IDΪ1�� �ɸ���ʵ����Ҫ�޸�
	}
	
	if(removefingerstate)
	{
		Del_FR_Lib();//���ָ��
	}
	
	//printf("AS608ָ��ģ����Կ�ʼ\r\n");
	
//	printf("AS608ָ��ģ����Կ�ʼ\r\n");
	
    //osDelay(1);
  }
  /* USER CODE END fingerprintTask1 */
}

/* USER CODE BEGIN Header_bluetoothTask1 */
/**
* @brief Function implementing the bluetoothTask thread.
* @param argument: Not used
* @retval None
*/
//��������
/* USER CODE END Header_bluetoothTask1 */
void bluetoothTask1(void *argument)
{
  /* USER CODE BEGIN bluetoothTask1 */
  /* Infinite loop */
  uint8_t str = 0xaa;
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RecvTmpBuf, 50);			//ʹ��dma + idle����
  for(;;)
  {
	getSemaphore(i2cSemaphoreHandle_t);
	OLED_ClearLine(0,4);
	OLED_PrintString(0,4,RecvTmpBuf);
	releaseSemaphore(i2cSemaphoreHandle_t);
	vTaskDelay(1000);
    //osDelay(1);
  }
  /* USER CODE END bluetoothTask1 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//������ⱨ������
void OledTask(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
//	uint32_t i = 0;
//	getSemaphore(i2cSemaphoreHandle_t);
//	OLED_ClearLine(0,2);
//	releaseSemaphore(i2cSemaphoreHandle_t);
  for(;;)
  {
	 //�����⵽���� ��ʼ��ʱ
	 if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
	 {
		getSemaphore(i2cSemaphoreHandle_t);
		HAL_TIM_Base_Start_IT(&htim2);						//������ʱ����ʼ����
		OLED_PrintSignedVal(0,2,stayTime);
		 
		getSemaphore(usartSemaphoreHandle_t);
		if(stayTime == 30)									//���ͣ����30s
		{
			broadcast_sound(9, 1000);						//��һ������
		}
		else if(stayTime == 60)								//���ͣ����60s
		{	
			broadcast_sound(10, 1000);						//�ڶ�������
		}
		else if(stayTime == 90)								//���ͣ����90s
		{
			broadcast_sound(11, 1000);						//����������
		}
		else if(stayTime == 120)							//���ͣ����120s
		{	
			broadcast_sound(13, 1000);						//����
		}else if((stayTime > 120) && (stayTime % 30 == 0))	//���һֱ���� ��ÿ30s����һ�α�����
		{
			broadcast_sound(13, 1000);						//����
		}
		releaseSemaphore(usartSemaphoreHandle_t);
		releaseSemaphore(i2cSemaphoreHandle_t);
	 }else
	 {
		//ֹͣ��ʱ ����ֵ����
		HAL_TIM_Base_Stop_IT(&htim2);
		stayTime = 0;
	 }
	 vTaskDelay(50);
	 //��ȡi2c�ź���
//	 getSemaphore(i2cSemaphoreHandle_t);
//	 OLED_PrintSignedVal(0,2,i++);
//	 //�ͷ�i2c�ź���
//	 releaseSemaphore(i2cSemaphoreHandle_t);
//	 vTaskDelay(50);
    //osDelay(1);
  }
  /* USER CODE END StartTask04 */
}
/* USER CODE END Application */

