/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "extern.h"
//#include <exception>
//#include "../../class/TEST_CLASS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t one_millisec_flag=0;
uint8_t ten_millisec_flag=0;
uint8_t hnd_millisec_flag=0;
uint8_t draw_UI_flag=0;
uint8_t one_sec_flag=0;
int stand_by=1000;
static int t3_tick=0;

extern uint8_t uart1_rx_rs485[256];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define xIMG_DOWN_FROM_PC //ICON
//#define __HAL_AFIO_REMAP_PD01_ENABLE()  AFIO_REMAP_ENABLE(AFIO_MAPR_PD01_REMAP)
//#define __HAL_AFIO_REMAP_PD01_DISABLE() AFIO_REMAP_DISABLE(AFIO_MAPR_PD01_REMAP)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#ifdef __cplusplus
extern "C" int _write(int32_t file, uint8_t *ptr, int32_t len)
{
#else
int _write(int32_t file, uint8_t *ptr, int32_t len) {
#endif
	if (HAL_UART_Transmit(&huart3, ptr, len, len) == HAL_OK)	return len;
	else return 0;
}

void MyErrorHandler()

{
    printf("MyErrorHandler 발생\r\n");
    exit(-1);
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#define BUFFER_SIZE 64
uint8_t rx_buffer[BUFFER_SIZE];
uint8_t rx_cnt=0;
uint8_t rx_flag=0;
uint8_t rx_ok=0;

uint8_t OneSecond_RTC_IRQ = 0;
int key_action_delay = 0;
uint16_t gBG_COLOR=MAIN_BG_COLOR;
uint8_t TOGGLE = 0;

//extern uint16_t t1_ADC_key_tick;

extern uint8_t uart1_rx_rs485[256];
extern uint8_t uart2_rx_modem[512];
extern uint16_t uart1_rx_cnt;//=0;//485
extern uint16_t uart2_rx_cnt;//=0;//modem
extern uint16_t uart_recv_try_cnt;

//void Main_IRQ_KEY_Process();

extern void Get_AdcData();
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void delay_ms(uint16_t time)
{
	uint16_t i=0;
   while(time--)
   {
      i=12000;  // Define your own
      while(i--) ;
   }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	//__HAL_AFIO_REMAP_PD01_DISABLE();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */

	pSTM32_IO = new STM32_IO();
	//pFlashClass = new SPI1_FLASH();
	pIli9488 = new ILI9488_DRV();
	pLCD_MAIN = new LCD_MAIN();
	pDataClass = new data_class();
	pCAN = new tja1050();
	//	pRS485 = new RS485();
	pZIGBEE =new zigbee();
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_ADCEx_Calibration_Start(&hadc1);
	pIli9488->ILI9488_DRV_Init();
	pCAN->init();
	HAL_Delay(100);
	__HAL_IWDG_START(&hiwdg);//40KHZ LCLK 128분주-3125(312.5*10)
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);

	printf("==========================\r\n");	//328byte
	printf("START VCU Board [ReV %.1f]\r\n\r\n",REV_NUM);
	pDataClass->SYSTEM_POWER(ON);

//pDataClass->get_cpuid();
//	pLCD_MAIN->Ozone_calcuration();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		//printf("========================\r\n");	//328byte
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);

		if(pLCD_MAIN->uiEvent.event_flag){
			pLCD_MAIN->Draw_Key_Event();
		}
		if(ten_millisec_flag){
		  ten_millisec_flag=0;
		  pDataClass->ten_millisec_routine();
		}
		if(one_sec_flag){
			//printf("one_sec_flag====================\r\n");
			pDataClass->onesec_routine();
		  one_sec_flag=0;
		}
		if(hnd_millisec_flag){
		  pSTM32_IO->Main_IRQ_KEY_Process();
		  pDataClass->hnd_millisec_routine();
		  hnd_millisec_flag=0;
		}
		if(draw_UI_flag){
			draw_UI_flag=0;
			if(pSTM32_IO->navi_state.system_setup==0) pLCD_MAIN->update_DrawMainMenu(0);
		}
		HAL_Delay(1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static unsigned short tick=0;
void HAL_SYSTICK_Callback(void)
{
	one_millisec_flag=1;
	HAL_IncTick();
	tick++;
	if(tick>=10){
		tick=0;
		ten_millisec_flag=1;
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM3){//TIM2 100ms
		hnd_millisec_flag=1;
		//if(key_action_delay>0) key_action_delay--;
		if(t3_tick++>10){
			t3_tick=0;
				one_sec_flag=1;
		}
		//printf("sys_count[%d]\r\n", sys_count);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//printf("HAL_UART_RxCpltCallback~\r\n");
	if (huart->Instance == USART1) {
		uint8_t uart_data;
		uint32_t tmp_flag = 0, tmp_it_source = 0;
		tmp_flag =  __HAL_UART_GET_FLAG(&huart1,UART_FLAG_RXNE);
		tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE);

		if ((tmp_it_source != RESET) && (tmp_flag != RESET)){

			uart_data=huart1.Instance->DR;
			{
				//printf("[%02X]\r\n",uart_data);
				//HAL_UART_Transmit(&huart3, &uart_data, 1, 1);
				if(uart_data==0xAF && rx_flag==0){
					rx_cnt=0;
					memset(&rx_buffer, 0, sizeof(rx_buffer));
					rx_flag=1;
				}
				if(rx_flag)rx_buffer[rx_cnt++]=uart_data;
				if(rx_cnt>16){
					rx_flag=0;
					rx_cnt=16;
					pZIGBEE->zigbee_received(rx_buffer);
				}
			}

		}
		__HAL_UART_CLEAR_PEFLAG(&huart1);

	}
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
}
//HAL_GPIO_WritePin
//static unsigned short timer_count=0;
//
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//    timer_count += 1;
//    if(timer_count >= 100){
//        //printf("timer interrupt![%d]\r\n",sys_count);
//       // pDataClass->ten_millisec_routine();
//        timer_count = 0;
//    }
//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	NVIC_SystemReset();//for rebooting
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
