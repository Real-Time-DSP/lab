/**
******************************************************************************
* @file    BSP/Src/main.c
* @author  MCD Application Team. Modified by Dan Jacobellis
* @brief   Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2019 STMicroelectronics.
* All rights reserved.
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/


//This file contains most of the setup code to perform audio processing (see lab.c and lab.h)
//The main while loop calls the display_spectrum function, which visualizes the input signal.

#include "main.h"

typedef enum
{
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF = 1,
  BUFFER_OFFSET_FULL = 2,
}BUFFER_StateTypeDef;

uint32_t button_state = 0;
uint32_t audio_buffer_offset;
uint32_t t;
uint32_t* systick = (uint32_t*) 0xe000e018;
int16_t play_buffer[FRAME_SIZE] __attribute__((aligned (32)));
int16_t record_buffer[FRAME_SIZE] __attribute__((aligned (32)));
BSP_AUDIO_Init_t  audio_in_init;
BSP_AUDIO_Init_t  audio_out_init;
arm_rfft_fast_instance_f32 fft_inst;
float32_t fft_in[FRAME_SIZE/4];
float32_t fft_out[FRAME_SIZE/4];
float32_t fft_mag[FRAME_SIZE/8];

/*This lookup table maps linear frequency bins to 1/36 octave bins*/
uint8_t FFT_to_CQ_LUT[161] = {
10,10,10,11,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,16,16,16,
17,17,17,18,18,18,19,19,19,20,20,20,21,21,22,22,22,23,23,24,24,25,25,26,26,27,
27,28,28,29,29,30,31,31,32,32,33,34,34,35,36,36,37,38,39,39,40,41,42,42,43,44,
45,46,47,48,49,50,51,51,52,54,55,56,57,58,59,60,61,62,64,65,66,67,69,70,71,73,
74,76,77,79,80,82,83,85,87,88,90,92,94,95,97,99,101,103,105,107,109,111,113,
116,118,120,122,125,127,130,132,135,137,140,143,146,148,151,154,157,160,163,
167,170,173,177,180,183,187,191,194,198,202,206,210,214,218};

void audio_init(void);
void display_spectrum(float32_t *fft_mag);
void tic(void);
uint32_t toc(void);
static void SystemClock_Config(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void Display_DemoDescription(void);
static void Error_Handler(void);
extern void lab_init(int16_t* output_buffer);
extern int16_t process_left_sample(int16_t input_sample);
extern int16_t process_right_sample(int16_t input_sample);
extern void process_input_buffer(int16_t* input_buffer);
extern void process_output_buffer(int16_t* output_buffer);

int main(void)
{
  MPU_Config();
  CPU_CACHE_Enable();
  HAL_Init();
  SystemClock_Config();
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
  BSP_LED_Init(LED1);
  BSP_LCD_Init(0, LCD_ORIENTATION_PORTRAIT);
  UTIL_LCD_SetFuncDriver(&LCD_Driver);
  Display_DemoDescription();
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  arm_rfft_fast_init_f32(&fft_inst, FRAME_SIZE/4);
  audio_init();
  lab_init(play_buffer);
  #ifdef ENABLE_VISUALIZATION
	  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
	  UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
  #endif

  while (1)
  {
    #ifdef ENABLE_VISUALIZATION
	  display_spectrum(fft_mag);
    #else
	  HAL_Delay(100);
    #endif
  }
}

void audio_init(void)
{
  audio_out_init.Device = AUDIO_OUT_DEVICE_HEADPHONE;
  audio_out_init.ChannelsNbr = 2;
  audio_out_init.SampleRate = SAMPLE_RATE;
  audio_out_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_out_init.Volume = 50;
  audio_in_init.Device = AUDIO_IN_DEVICE_ANALOG_MIC;
  audio_in_init.ChannelsNbr = 2;
  audio_in_init.SampleRate = SAMPLE_RATE;
  audio_in_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_in_init.Volume = 100;
  BSP_AUDIO_IN_OUT_Init_Ex(0, 0, &audio_in_init, &audio_in_init);
  BSP_AUDIO_IN_Record_Ex(0, (uint8_t*) record_buffer, FRAME_SIZE*2);
  BSP_AUDIO_OUT_Play(0, (uint8_t*) play_buffer, FRAME_SIZE*2);
  audio_buffer_offset  = BUFFER_OFFSET_NONE;
}

void tic(void)
{
	t = *systick;
}

uint32_t toc(void)
{
	return t - *systick;
}

void display_spectrum(float32_t *fft_mag)
{
	uint32_t x1, x2;
	float32_t height;

	for(uint32_t i_bar = 0; i_bar<160; i_bar+=1)
	{
		x1 = FFT_to_CQ_LUT[i_bar];
		x2 = FFT_to_CQ_LUT[i_bar+1];
		if (x1 == x2){height = 20*logf(fft_mag[x1]);}
		else
		{
			height = 0;
			for (uint32_t bin = x1; bin<x2; bin+=1)
			{
				height += fft_mag[bin];
			}
			height = 20*logf(height);
		}

		height = 2*height - 200;
		if (height>270){height = 270;}
		UTIL_LCD_DrawVLine(3*i_bar, 0, 272, UTIL_LCD_COLOR_BLACK);
		UTIL_LCD_DrawVLine(3*i_bar, (uint32_t)(272-height), (uint32_t)height, UTIL_LCD_COLOR_BLUE);
	}
}

void display_image(uint32_t* binary_image, uint32_t width, uint32_t height)
{
	uint32_t mask = 1;
	uint32_t i_word = 0;
	uint32_t word = binary_image[i_word];
	uint32_t pixel = word & mask;
	UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
	for (uint32_t x = 0; x < width; x+=1)
	{
		for (uint32_t y = 0; y < height; y+=1)
		{
			if(pixel){
				UTIL_LCD_FillRect(2*x, 2*y, 2, 2, UTIL_LCD_COLOR_WHITE);
			}
			mask <<= 1;
			if (mask == 0){
				i_word += 1;
				mask = 1;
				word = binary_image[i_word];
			}
			pixel = word & mask;
		}
	}
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *) record_buffer, FRAME_SIZE*2);
	#ifdef PROCESS_INPUT_BUFFER
    	process_input_buffer(record_buffer);
	#endif
    for(uint32_t i_sample = 0 ; i_sample < FRAME_SIZE/2; i_sample+=1)
    {
		#ifdef PROCESS_LEFT_CHANNEL
			play_buffer[i_sample] = process_left_sample(record_buffer[i_sample]);
		#elif defined(PERIODIC_LOOKUP_TABLE)
		#else
			play_buffer[i_sample] = record_buffer[i_sample];
		#endif
		i_sample +=1;
		#ifdef PROCESS_RIGHT_CHANNEL
			play_buffer[i_sample] = process_right_sample(record_buffer[i_sample]);
		#elif defined(PERIODIC_LOOKUP_TABLE)
		#else
			play_buffer[i_sample] = record_buffer[i_sample];
		#endif
    }
	#ifdef PROCESS_OUTPUT_BUFFER
    	process_output_buffer(play_buffer);
	#endif
    SCB_CleanDCache_by_Addr((uint32_t *)play_buffer, FRAME_SIZE);
    audio_buffer_offset  = BUFFER_OFFSET_HALF;
}
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    SCB_InvalidateDCache_by_Addr((uint32_t*) &record_buffer[FRAME_SIZE/2], FRAME_SIZE*2);
	#ifdef PROCESS_INPUT_BUFFER
		process_input_buffer(&record_buffer[FRAME_SIZE/2]);
	#endif
	for(uint32_t i_sample = FRAME_SIZE/2; i_sample < FRAME_SIZE; i_sample+=1)
	{
		#ifdef PROCESS_LEFT_CHANNEL
			play_buffer[i_sample] = process_left_sample(record_buffer[i_sample]);
		#elif defined(PERIODIC_LOOKUP_TABLE)
		#else
			play_buffer[i_sample] = record_buffer[i_sample];
		#endif
		i_sample +=1;
		#ifdef PROCESS_RIGHT_CHANNEL
			play_buffer[i_sample] = process_right_sample(record_buffer[i_sample]);
		#elif defined(PERIODIC_LOOKUP_TABLE)
		#else
			play_buffer[i_sample] = record_buffer[i_sample];
		#endif
	}
	#ifdef PROCESS_OUTPUT_BUFFER
		process_output_buffer(&record_buffer[FRAME_SIZE/2]);
	#endif
    SCB_CleanDCache_by_Addr((uint32_t*) &play_buffer[FRAME_SIZE/2], FRAME_SIZE);
    audio_buffer_offset  = BUFFER_OFFSET_FULL;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 520000000 (CPU Clock)
  *            HCLK(Hz)                       = 260000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  130MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  130MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  130MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  130MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 104
  *            PLL_P                          = 1
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 104;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

/* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

/*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
          (GPIO, SPI, FMC, OSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
          The I/O Compensation Cell activation  procedure requires :
        - The activation of the CSI clock
        - The activation of the SYSCFG clock
        - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
*/


  __HAL_RCC_CSI_ENABLE() ;

  __HAL_RCC_SYSCFG_CLK_ENABLE() ;

  HAL_EnableCompensationCell();

}

/**
  * @brief  Display main demo messages
  * @retval None
  */
static void Display_DemoDescription(void)
{
  uint32_t x_size;
  uint32_t y_size;

  UTIL_LCD_SetFont(&UTIL_LCD_DEFAULT_FONT);

  /* Clear the LCD */
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);

  /* Set the LCD Text Color */
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_DARKBLUE);

  /* Display LCD messages */
  UTIL_LCD_DisplayStringAt(0, 10, (uint8_t *)"STM32H735G-DK", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, 35, (uint8_t *)"Stereo DMA Talkthrough", CENTER_MODE);

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  UTIL_LCD_SetFont(&Font12);
  UTIL_LCD_DisplayStringAt(0, y_size - 20, (uint8_t *)"Copyright (c) STMicroelectronics 2019", CENTER_MODE);

  UTIL_LCD_SetFont(&Font16);
  BSP_LCD_FillRect(0, 0, y_size/2 , x_size, 30, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_DisplayStringAt(0, y_size / 2 , (uint8_t *)"Plug input device into blue jack", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, y_size/2 + 15, (uint8_t *)"Plug output device into green jack", CENTER_MODE);
}

/**
  * @brief  Check for user input
  * @param  None
* @retval Input state (1 : active / 0 : Inactive)
  */
uint8_t CheckForUserInput(void)
{
  return button_state;
}

/**
  * @brief  Button Callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if(Button == BUTTON_USER)
  {
    button_state = 1;
  }
}

/**
  * @brief  CPU MPU Config.
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for OctoSPI RAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = OSPI_RAM_WRITE_READ_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
