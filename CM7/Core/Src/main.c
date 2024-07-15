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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "chart_handler.h"
#include "config.h"
#include "lcd.h"
#include "lvgl_api.h"
#include "stm32h7xx_hal_adc.h"
#include "touch_screen.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc2;

CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

DSI_HandleTypeDef hdsi;

I2C_HandleTypeDef hi2c4;

LTDC_HandleTypeDef hltdc;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;

SDRAM_HandleTypeDef hsdram2;

/* USER CODE BEGIN PV */

static LvHandler lv_handler;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C4_Init(void);
static void MX_DMA2D_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM7_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_ADC2_Init(void);
static void MX_LTDC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC3_Init(void);
static void MX_FMC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Start the timer used to count microseconds
 */
void start_channels_conversion(void) {
    // Restart microseconds timer
    __HAL_TIM_SET_COUNTER(&htim7, 0U);
    HAL_TIM_Base_Start(&htim7);
    // Start ADC conversion
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)CHART_CH1_RAW_DATA_ADDRESS, CHART_SAMPLE_COUNT); 
}

/**
 * @brief Stop the timer used to count microseconds
 *
 * @return uint32_t The timer counter value
 */
uint32_t stop_channels_conversion(void) {
    // Stop microseconds timer and ADC conversion
    HAL_TIM_Base_Stop(&htim7);
    HAL_ADC_Stop_DMA(&hadc2);
    return __HAL_TIM_GET_COUNTER(&htim7);
}

void select_knob_channel(size_t i) {
    ADC_ChannelConfTypeDef config = {
        .Channel = ADC_CHANNEL_0,
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLETIME_16CYCLES_5,
        .SingleDiff = ADC_SINGLE_ENDED,
        .OffsetNumber = ADC_OFFSET_NONE,
        .Offset = 0,
        .OffsetSignedSaturation = DISABLE
    };

    switch (i) {
        case 0: 
            config.Channel = ADC_CHANNEL_0;
            break;
        case 1: 
            config.Channel = ADC_CHANNEL_1;
            break;
        case 2: 
            config.Channel = ADC_CHANNEL_6;
            break;
        default:
            config.Channel = ADC_CHANNEL_0;
            break;
    }
    HAL_ADC_ConfigChannel(&hadc3, &config);
}

void update_knob_trigger(uint16_t value) {
    static uint16_t prev_value = 0U;

    if (value / 50 == prev_value / 50)
        return;
    prev_value = value;

    value *= 4U;
    chart_handler_set_trigger(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, value);
}

void update_knob_scale(uint16_t value) {
    static size_t prev_index = 6U;
    static size_t prev_x_index = 6U;

    const float scales[] = { 50.f, 100.f, 125.f, 250.f, 500.f, 1000.f, 2000.f, 5000.f };
    const float x_scales[] = { 10.f, 50.f, 100.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 50000.f, 100000.f };

    ChartHandlerKnobMode mode = chart_handler_knob_get_mode(&lv_handler.chart_handler);

    switch (mode) {
        case CHART_HANDLER_KNOB_VOLTAGE:
            {
                size_t index = value / 512;
                if (index == prev_index)
                    return; 
                prev_index = index;
                chart_handler_set_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, scales[index]);
            }
            break;
        case CHART_HANDLER_KNOB_TIME:
            {
                size_t index = value / 372.36;
                if (index == prev_x_index)
                    return; 
                prev_x_index = index;
                chart_handler_set_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, x_scales[index]);
            }
            break;
        default:
            break;
    }
}

void update_knob_offset(uint16_t value) {
    static uint16_t prev_value = 0U;

    if (value / 50 == prev_value / 50)
        return;
    prev_value = value;

    ChartHandlerKnobMode mode = chart_handler_knob_get_mode(&lv_handler.chart_handler);
    switch (mode) {
        case CHART_HANDLER_KNOB_VOLTAGE:
            chart_handler_set_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, ADC_VALUE_TO_VOLTAGE(value));
            break;
        case CHART_HANDLER_KNOB_TIME:
            chart_handler_set_x_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, ADC_VALUE_TO_VOLTAGE(value));
            break;
        default:
            break;
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

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
  int32_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
  timeout = 0xFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
  Error_Handler();
  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C4_Init();
  MX_DMA2D_Init();
  MX_CRC_Init();
  MX_TIM7_Init();
  MX_DSIHOST_DSI_Init();
  MX_ADC2_Init();
  MX_LTDC_Init();
  MX_USART1_UART_Init();
  MX_ADC3_Init();
  MX_FMC_Init();
  /* USER CODE BEGIN 2 */

  // Clear SRAM used memory before use
  memset((uint32_t *)LCD_FRAME_BUFFER_0_ADDRESS, 0U, LCD_FRAME_BUFFER_0_WIDTH);
  memset((uint32_t *)LCD_FRAME_BUFFER_1_ADDRESS, 0U, LCD_FRAME_BUFFER_1_WIDTH);
  memset((uint32_t *)CHART_CH1_RAW_DATA_ADDRESS, 0U, CHART_CH1_RAW_DATA_WIDTH);
  memset((uint32_t *)CHART_CH2_RAW_DATA_ADDRESS, 0U, CHART_CH2_RAW_DATA_WIDTH);

  // Init LCD display controller
  if (lcd_init(&hdsi, LCD_INITIAL_BRIGHTNESS) != HAL_OK)
      Error_Handler();

  // Init the touch screen controller
  if (ts_init(&hi2c4, LCD_WIDTH, LCD_HEIGHT, TS_ORIENTATION_SWAP_XY, 2) != HAL_OK)
      Error_Handler();

  // Turn off the LCD and disable touch screen
  // lcd_off();
  // ts_disable();

  // Init lvgl api
  lv_api_init(
      &lv_handler,
      LCD_WIDTH,
      LCD_HEIGHT,
      (void *)LCD_FRAME_BUFFER_1_ADDRESS,
      (void *)LCD_FRAME_BUFFER_0_ADDRESS,
      LCD_FRAME_BUFFER_0_WIDTH
  );

  char msg[128] = { 0 };
  sprintf(msg, "Vertical offset CH1: %.2f\r\n", chart_handler_get_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
  sprintf(msg, "Vertical scale CH1: %.2f\r\n\r\n", chart_handler_get_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);

  // sprintf(msg, "Horizontal offset CH1: %.2f\r\n", chart_handler_get_x_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
  // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
  sprintf(msg, "Horizontal scale CH1: %.2f\r\n\r\n", chart_handler_get_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // Calibrate ADC
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_Delay(10);

  // Start oscilloscope channel conversions
  // BUG: DMA transfert error if using the internal RAM as memory destination
  start_channels_conversion();

  // Start potenziometers ADC
  HAL_ADC_Start(&hadc3);

  uint32_t timestamp = 0U;
  uint32_t knob_t = 0U;
  size_t knob_i = 0U;

  while (1)
  {
    if (HAL_GetTick() - timestamp >= 500U) {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        timestamp = HAL_GetTick();
    }

    // Read knob values
    if (HAL_GetTick() - knob_t >= 10U) {
        select_knob_channel(knob_i);
        HAL_ADC_Start(&hadc3);
        HAL_ADC_PollForConversion(&hadc3, 5);
        uint16_t value = HAL_ADC_GetValue(&hadc3);
        HAL_ADC_Stop(&hadc3);

        switch (knob_i) {
            case 0:
                update_knob_scale(value);
                break;
            case 1:
                update_knob_offset(value);
                break;
            case 2:
                update_knob_trigger(value);
                break;
            default:
                break;
        } 
        ++knob_i;
        if (knob_i >= CHART_KNOB_COUNT)
            knob_i = 0U;
        knob_t = HAL_GetTick();
    }

    lv_api_run(&lv_handler);

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 6;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 1;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_14B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = ENABLE;
  hadc3.Init.NbrOfConversion = 3;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{

  /* USER CODE BEGIN DSIHOST_Init 0 */

  /* USER CODE END DSIHOST_Init 0 */

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_VidCfgTypeDef VidCfg = {0};

  /* USER CODE BEGIN DSIHOST_Init 1 */

  /* USER CODE END DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  PLLInit.PLLNDIV = 100;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV5;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 20;
  PhyTimings.ClockLaneLP2HSTime = 20;
  PhyTimings.DataLaneHS2LPTime = 10;
  PhyTimings.DataLaneLP2HSTime = 10;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 0;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  VidCfg.VirtualChannelID = 0;
  VidCfg.ColorCoding = DSI_RGB888;
  VidCfg.LooselyPacked = DSI_LOOSELY_PACKED_DISABLE;
  VidCfg.Mode = DSI_VID_MODE_BURST;
  VidCfg.PacketSize = 800;
  VidCfg.NumberOfChunks = 0;
  VidCfg.NullPacketSize = 0xFFF;
  VidCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  VidCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  VidCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  VidCfg.HorizontalSyncActive = 5;
  VidCfg.HorizontalBackPorch = 77;
  VidCfg.HorizontalLine = 1977;
  VidCfg.VerticalSyncActive = 2;
  VidCfg.VerticalBackPorch = 14;
  VidCfg.VerticalFrontPorch = 16;
  VidCfg.VerticalActive = 480;
  VidCfg.LPCommandEnable = DSI_LP_COMMAND_ENABLE;
  VidCfg.LPLargestPacketSize = 4;
  VidCfg.LPVACTLargestPacketSize = 4;
  VidCfg.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;
  VidCfg.LPHorizontalBackPorchEnable = DSI_LP_HBP_ENABLE;
  VidCfg.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;
  VidCfg.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;
  VidCfg.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;
  VidCfg.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE;
  VidCfg.FrameBTAAcknowledgeEnable = DSI_FBTAA_DISABLE;
  if (HAL_DSI_ConfigVideoMode(&hdsi, &VidCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */

  /* USER CODE END DSIHOST_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x70B03839;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AH;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AH;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 1;
  hltdc.Init.VerticalSync = 1;
  hltdc.Init.AccumulatedHBP = 35;
  hltdc.Init.AccumulatedVBP = 15;
  hltdc.Init.AccumulatedActiveW = 835;
  hltdc.Init.AccumulatedActiveH = 495;
  hltdc.Init.TotalWidth = 869;
  hltdc.Init.TotalHeigh = 511;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 480;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = 0xD0000000;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 480;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  // Start DSI HOST handler after LTDC initialization to avoid synchronization issues
  if (HAL_DSI_Start(&hdsi) != HAL_OK)
      Error_Handler();

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 199;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/* FMC initialization function */
void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM2 memory initialization sequence
  */
  hsdram2.Instance = FMC_SDRAM_DEVICE;
  /* hsdram2.Init */
  hsdram2.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram2.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hsdram2.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram2.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram2.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram2.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
  hsdram2.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram2.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram2.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram2.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 5;
  SdramTiming.RowCycleDelay = 6;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram2, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  HAL_StatusTypeDef res = HAL_OK;

  char msg[100] = "\r\n*** SDRAM INITIALIZATION ***\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);

  // Configure a clock configuration enable command
  FMC_SDRAM_CommandTypeDef cmd = {
      .CommandMode = FMC_SDRAM_CMD_CLK_ENABLE,
      .CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2,
      .AutoRefreshNumber = 1,
      .ModeRegisterDefinition = 0
  };
  if ((res = HAL_SDRAM_SendCommand(&hsdram2, &cmd, 0xfff)) != HAL_OK)
      Error_Handler();
  HAL_Delay(1); // A minimum of 100 us delay is require for the previous command

  sprintf(msg, "* Enable command: %d\r\n", res);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);
    
  // Configure a PALL (precharge all) command
  cmd.CommandMode = FMC_SDRAM_CMD_PALL;
  if ((res = HAL_SDRAM_SendCommand(&hsdram2, &cmd, 0xfff)) != HAL_OK)
      Error_Handler();

  sprintf(msg, "* PALL command: %d\r\n", res);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);

  // Configure auto refresh command
  cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  cmd.AutoRefreshNumber = 2;
  if ((res = HAL_SDRAM_SendCommand(&hsdram2, &cmd, 0xfff)) != HAL_OK)
      Error_Handler();

  sprintf(msg, "* Auto refresh command: %d\r\n", res);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);

  // Program the external memory mode register
  cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  cmd.ModeRegisterDefinition = (uint32_t)0 | (0 << 3) | (2 << 4) | (0 << 7) | (1 << 9);
  if ((res = HAL_SDRAM_SendCommand(&hsdram2, &cmd, 0xfff)) != HAL_OK)
      Error_Handler();

  sprintf(msg, "* External memory command: %d\r\n", res);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);

  // Set the refresh rate counter
  /* 
   * refresh_rate = [(SDRAM self refresh time / number of row) x  SDRAM CLK] – 20 =
   *              = [(64ms / 4096) * 100MHz] - 20
   *              = 1562.5 - 20 ~=
   *              ~= 1562
   */
  const uint32_t refresh_rate = 1562; // 2323;
  if ((res = HAL_SDRAM_ProgramRefreshRate(&hsdram2, refresh_rate)) != HAL_OK)
      Error_Handler();

  sprintf(msg, "* Refresh rate command: %d\r\n", res);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);
  
  sprintf(msg, "****************************\r\n\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 20);

  HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, GPIO_PIN_RESET);

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DSI_RESET_GPIO_Port, DSI_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : JOY_RIGHT_Pin JOY_LEFT_Pin JOY_UP_Pin JOY_DOWN_Pin
                           JOY_SELECT_Pin */
  GPIO_InitStruct.Pin = JOY_RIGHT_Pin|JOY_LEFT_Pin|JOY_UP_Pin|JOY_DOWN_Pin
                          |JOY_SELECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /*Configure GPIO pin : TOUCH_INTERRUPT_Pin */
  GPIO_InitStruct.Pin = TOUCH_INTERRUPT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TOUCH_INTERRUPT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BACKLIGHT_Pin */
  GPIO_InitStruct.Pin = LCD_BACKLIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BACKLIGHT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin LED4_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : DSI_RESET_Pin */
  GPIO_InitStruct.Pin = DSI_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(DSI_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_TE_Pin */
  GPIO_InitStruct.Pin = LCD_TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_TE_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */

  // Reset DSI HOST controller
  lcd_reset_dsi_controller();

  // Set LCD backlight pin
  HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);

  // Reset peripheral clocks
  __HAL_RCC_LTDC_CLK_ENABLE();
  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_RELEASE_RESET();

  __HAL_RCC_DMA2D_CLK_ENABLE();
  __HAL_RCC_DMA2D_FORCE_RESET();
  __HAL_RCC_DMA2D_RELEASE_RESET();

  __HAL_RCC_DSI_CLK_ENABLE();
  __HAL_RCC_DSI_FORCE_RESET();
  __HAL_RCC_DSI_RELEASE_RESET();

/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // Lock to avoid multiple interrupts during operations
    static bool lock = false;
    if (lock)
        return;
    lock = true;
    
    static char msg[128] = { 0 };

    if (pin == USER_BUTTON_Pin) {
        // Debounce
        static uint32_t last_press_time = 0U;
        uint32_t t = HAL_GetTick();
        if (t - last_press_time >= BUTTON_DEBOUNCE_TIME) {
            last_press_time = t;
            // Stop the acquisition
            chart_handler_toggle_running(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);

            sprintf(msg, "Running: %s\r\n", chart_handler_is_running(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1) ? "true" : "false");
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);

            HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, chart_handler_is_running(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        }
    }
    else if (pin == TOUCH_INTERRUPT_Pin) {
        // Get touch screen info
        TsInfo info;
        if (ts_get_info(&info) == HAL_OK)
            lv_api_update_ts_status(&info);
    }
    else if (pin == JOY_SELECT_Pin) {
        float off = chart_handler_get_x_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);
        off += 50.f;
        chart_handler_set_x_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, off);

        sprintf(msg, "Offset: %.2f\r\n", chart_handler_get_x_offset(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
    }
    else if (pin == JOY_LEFT_Pin) {
        float scale = chart_handler_get_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);
        scale *= 0.5f;
        chart_handler_set_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, scale);

        sprintf(msg, "X scale: %.2f\r\n", chart_handler_get_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
    }
    else if (pin == JOY_UP_Pin) {
        float scale = chart_handler_get_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);
        scale *= 2.0f;
        chart_handler_set_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, scale);

        sprintf(msg, "Scale: %.2f\r\n", chart_handler_get_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
    }
    else if (pin == JOY_DOWN_Pin) {
        float scale = chart_handler_get_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);
        scale *= 0.5f;
        chart_handler_set_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, scale);

        sprintf(msg, "Scale: %.2f\r\n", chart_handler_get_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
    }
    else if (pin == JOY_RIGHT_Pin) {
        float scale = chart_handler_get_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1);
        scale *= 2.0f;
        chart_handler_set_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1, scale);

        sprintf(msg, "X scale: %.2f\r\n", chart_handler_get_x_scale(&lv_handler.chart_handler, CHART_HANDLER_CHANNEL_1));
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 30);
    }

    lock = false;
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef * hadc) {
    UNUSED(hadc);
    HAL_UART_Transmit(&huart1, (uint8_t *)"ADC DMA Error\r\n", 15U, 30);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc) { 
    if (hadc->Instance == hadc2.Instance) {
        // Stop microseconds timer and get counter value
        uint32_t dt = stop_channels_conversion();
        if (dt == 0U)
            dt = 1U;

        chart_handler_update(&lv_handler.chart_handler, dt);

        // Restart timer
        start_channels_conversion();
    }
}

void print(char * msg, size_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 30);
}

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

  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  HAL_UART_Transmit(&huart1, (uint8_t *)"[ERROR]: Error handler called\r\n", 31, 10);

  while (1) {  }
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
