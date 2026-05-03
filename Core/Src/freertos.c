/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for dhtTask */
osThreadId_t dhtTaskHandle;
const osThreadAttr_t dhtTask_attributes = {
  .name = "dhtTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for tcpTask */
osThreadId_t tcpTaskHandle;
const osThreadAttr_t tcpTask_attributes = {
  .name = "tcpTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for irSensorTask */
osThreadId_t irSensorTaskHandle;
const osThreadAttr_t irSensorTask_attributes = {
  .name = "irSensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for rfidTask */
osThreadId_t rfidTaskHandle;
const osThreadAttr_t rfidTask_attributes = {
  .name = "rfidTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void dhtSystemTask(void *argument);
void tcpClientSystemTask(void *argument);
void irSensorSystemTask(void *argument);
void rfidSystemTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dhtTask */
  dhtTaskHandle = osThreadNew(dhtSystemTask, NULL, &dhtTask_attributes);

  /* creation of tcpTask */
  tcpTaskHandle = osThreadNew(tcpClientSystemTask, NULL, &tcpTask_attributes);

  /* creation of irSensorTask */
  irSensorTaskHandle = osThreadNew(irSensorSystemTask, NULL, &irSensorTask_attributes);

  /* creation of rfidTask */
  rfidTaskHandle = osThreadNew(rfidSystemTask, NULL, &rfidTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_dhtSystemTask */
/**
* @brief Function implementing the dhtTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_dhtSystemTask */
__weak void dhtSystemTask(void *argument)
{
  /* USER CODE BEGIN dhtSystemTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END dhtSystemTask */
}

/* USER CODE BEGIN Header_tcpClientSystemTask */
/**
* @brief Function implementing the tcpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tcpClientSystemTask */
__weak void tcpClientSystemTask(void *argument)
{
  /* USER CODE BEGIN tcpClientSystemTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END tcpClientSystemTask */
}

/* USER CODE BEGIN Header_irSensorSystemTask */
/**
* @brief Function implementing the irSensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_irSensorSystemTask */
__weak void irSensorSystemTask(void *argument)
{
  /* USER CODE BEGIN irSensorSystemTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END irSensorSystemTask */
}

/* USER CODE BEGIN Header_rfidSystemTask */
/**
* @brief Function implementing the rfidTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_rfidSystemTask */
__weak void rfidSystemTask(void *argument)
{
  /* USER CODE BEGIN rfidSystemTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END rfidSystemTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

