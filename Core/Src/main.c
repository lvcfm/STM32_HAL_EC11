/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define num_line(y, num, rgb) OLED_ShowNum(48, y, num, 2, 16, rgb)
#define string_line(y) OLED_ShowString(72, y, "ms", 16, 0)
#define FLASH_STORAGE_ADDRESS ((uint32_t)0x08007000)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int optionIndex = 0;           // 菜单索引
int timeValues[3] = {0, 0, 0}; // 脉冲长度
int count_num;
int32_t test_num = 0; // 支持正负数�?�增�??????
uint8_t dir_flag = 2; /*  方向标志 0: 顺时 1: 逆时 2: 未动*/
// uint8_t key_click_flag = 0; // EC11 中键

unsigned short key_value;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  static uint8_t count = 0;
  static uint8_t b_flag;
  GPIO_PinState a_value = HAL_GPIO_ReadPin(EC11_A_GPIO_Port, EC11_A_Pin);
  GPIO_PinState b_value = HAL_GPIO_ReadPin(EC11_B_GPIO_Port, EC11_B_Pin);

  if (GPIO_Pin == EC11_A_Pin)
  {
    if (a_value == RESET && count == 0)
    {
      b_flag = 0;
      if (b_value)
        b_flag = 1;
      count = 1;
    }

    if (a_value == SET && count == 1)
    {
      if (b_value == RESET && b_flag == 1)
      { // 逆时针转�??
        test_num--;
        timeValues[optionIndex] = timeValues[optionIndex] > 0 ? timeValues[optionIndex] - 1 : 0;
        dir_flag = 1;
      }
      if (b_value && b_flag == 0)
      { // 顺时针转�??
        test_num++;
        timeValues[optionIndex]++;
        dir_flag = 0;
      }
      count = 0;
    }
  }

  /* EC11中键，按键中�???? */
  // if(GPIO_Pin == EC11_KEY_Pin)
  // {
  // key_click_flag = 1;
  // }
}

// 写入Flash的函�??
HAL_StatusTypeDef WriteToFlash(uint32_t Address, uint32_t Data)
{
  HAL_FLASH_Unlock(); // 解锁Flash
  uint32_t PageError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = Address;
  EraseInitStruct.NbPages = 1;                     // 根据您的Flash页大小，这里假设�??1�??
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError); // 擦除页面
  if (PageError != 0xFFFFFFFF)
  {
    HAL_FLASH_Lock(); // 上锁Flash
    return HAL_ERROR;
  }
  HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, Data);
  HAL_FLASH_Lock(); // 上锁Flash
  return status;
}

// 从Flash读取数据的函�??
uint32_t ReadFromFlash(uint32_t Address)
{
  return *(__IO uint32_t *)Address; // 直接从指定地�??读取数据
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */
  HAL_StatusTypeDef HAL_UART_STATUS;
  uint32_t tick2, tick3;

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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // ######################################################################################################
  uint32_t Main_Fosc = HAL_RCC_GetSysClockFreq();
  printf("Main_Fosc:%dHz \r\n", Main_Fosc);

  // "hello embedded!\n"
  uint8_t *str = (uint8_t *)"串口通信测试�?? \n";
  HAL_UART_STATUS = HAL_UART_Transmit(&huart1, str, strlen((char *)str), HAL_MAX_DELAY);

  // 如果成功发�?�就执行下面的语�?????
  if (HAL_UART_STATUS == HAL_OK)
  {
    str = (uint8_t *)"success\r\n";
    HAL_UART_Transmit(&huart1, str, strlen((char *)str), HAL_MAX_DELAY);
  }

  // 从Flash读取变量
  uint32_t read_value = ReadFromFlash(FLASH_STORAGE_ADDRESS);
  count_num = (int)read_value > 0 ? (int)read_value : 0; // 将读取的值转换回原来的类�??

  HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_RESET);

  OLED_Init();  // OLED初始
  OLED_Clear(); // 清屏
  // 正相显示6X8--�??12�??0�??  反相显示8X16--�??16�??1�??

  // 显示汉字“脉冲一�??
  OLED_ShowCHinese(0, 0, 0, 0);
  OLED_ShowCHinese(16, 0, 1, 0);
  OLED_ShowCHinese(32, 0, 2, 0);
  num_line(0, 0, 0);
  string_line(0);
  // 显示汉字“间隔�??
  OLED_ShowCHinese(0, 2, 4, 0);
  OLED_ShowCHinese(32, 2, 5, 0);
  num_line(2, 0, 0);
  string_line(2);
  // 显示汉字“脉冲二�??
  OLED_ShowCHinese(0, 4, 0, 0);
  OLED_ShowCHinese(16, 4, 1, 0);
  OLED_ShowCHinese(32, 4, 3, 0);
  num_line(4, 0, 0);
  string_line(4);
  // 显示汉字“计数�??
  OLED_ShowCHinese(0, 6, 6, 0);
  OLED_ShowCHinese(32, 6, 7, 0);
  OLED_ShowNum(72, 6, count_num, 3, 16, 0);
  OLED_ShowNum(88, 0, 1, 1, 12, 0);
  //  #
  //  ######################################################################################################

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // ######################################################################################################
    if (dir_flag != 2)
    {
      switch (dir_flag)
      {
      case 0:
        printf("顺时针转动\r\n");
        break;
      case 1:
        printf("逆时针转动\r\n");
        break;
      }
      dir_flag = 2;
      printf("num: %d\r\n", test_num);
      num_line(optionIndex * 2, timeValues[optionIndex], 0);
    }
    if (HAL_GetTick() - tick2 >= 1)
    {
      tick2 = HAL_GetTick();
      key_check_all_loop_1ms();
    }
    /* Key按键按下查询 */
    if (HAL_GetTick() - tick3 >= 10)
    {
      tick3 = HAL_GetTick();
      key_value = key_read_value();

      if (key_value == KEY0_UP_SHORT)
      { // 单击事件 切换索引
        printf("\r\n单击\r\n");
        HAL_GPIO_TogglePin(PC_LED_GPIO_Port, PC_LED_Pin);
        optionIndex = (optionIndex + 1) % 3;
        OLED_ShowNum(88, 0, optionIndex + 1, 1, 12, 0);
        // 实现单击KEY
        // test_num++;
      }
      else if (key_value == KEY0_UP_DOUBLE)
      { // 双击事件 切换索引
        printf("\r\n双击\r\n");
        HAL_GPIO_TogglePin(PC_LED_GPIO_Port, PC_LED_Pin);
        timeValues[optionIndex] = 0;
        printf("按键计数清零,项目�??%d val�??%d ms\r\n", optionIndex + 1, timeValues[optionIndex]);
        num_line(optionIndex * 2, 0, 0);
      }
      else if (key_value == KEY0_LONG)
      { // 长按事件 触发脉冲
        printf("\r\n长按\r\n");
        // 实现长按KEY
        count_num++;
        // 存储变量到Flash
        if (WriteToFlash(FLASH_STORAGE_ADDRESS, (uint32_t)count_num) != HAL_OK)
        {
          // 处理错误
          printf("\r\n写入数据失败\r\n");
        }
        OLED_ShowNum(56, 6, count_num, 3, 16, 0);
        // 双脉�??
        if (timeValues[0] > 0 && timeValues[2] > 0)
        {
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_SET);
          HAL_Delay(timeValues[0]);
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_RESET);
          HAL_Delay(timeValues[1]);
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_SET);
          HAL_Delay(timeValues[2]);
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_RESET);
        }
        // 单脉�??
        else if (timeValues[0] > 0)
        {
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_SET);
          HAL_Delay(timeValues[0]);
          HAL_GPIO_WritePin(PA_PWM_GPIO_Port, PA_PWM_Pin, GPIO_PIN_RESET);
        }
      }
    }
    // ######################################################################################################
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
