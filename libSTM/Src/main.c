/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 24/03/2015 16:08:54
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h" /* defines SD_Driver as external */
#include "i2c.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

//#include "radioIn.h"


/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

uint8_t retSD;    /* Return value for SD */
char SD_Path[4];  /* SD logical drive path */

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */

/* USER CODE BEGIN PV */
#define LED1	GPIO_PIN_4
#define LED2	GPIO_PIN_5
#define LED3	GPIO_PIN_10
#define LED4	GPIO_PIN_2

#define LED1_Port	GPIOC
#define LED2_Port	GPIOC
#define LED3_Port	GPIOC
#define LED4_Port	GPIOB
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#define main mcu_init
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SDIO_SD_Init();
  MX_SPI2_Init();
  MX_TIM10_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM5_Init();     //Input Capture CH1 and CH2 timer base
  MX_TIM3_Init();       //PWM Output CH1 to CH4

// Led Test. Turn On
  HAL_GPIO_WritePin(LED1_Port, LED1, RESET);
  HAL_GPIO_WritePin(LED2_Port, LED2, RESET);
  HAL_GPIO_WritePin(LED3_Port, LED3, RESET);
  HAL_GPIO_WritePin(LED4_Port, LED4, RESET);
  HAL_Delay(1000);
  //Turn Off
  HAL_GPIO_WritePin(LED1_Port, LED1, SET);
  HAL_GPIO_WritePin(LED2_Port, LED2, SET);
  HAL_GPIO_WritePin(LED3_Port, LED3, SET);
  HAL_GPIO_WritePin(LED4_Port, LED4, SET);

  /* USER CODE BEGIN 2 */
    radioIn_init();     //elgarbe**************************************************
    start_pwm_outputs();
      MPU6000_init16();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}

/* USER CODE BEGIN 4 */
extern int tsirq;

/* USER CODE END 4 */

void StartDefaultTask(void const * argument)
{
    FRESULT res;                                            /* FatFs function common result code */
	uint8_t wtext[] = "Matrix Pilot with FatFs support";    /* File write buffer */
    uint32_t byteswritten;                                  /* File write counts */
    /*## FatFS: Link the SD driver ###########################*/
    //  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path);
    if(FATFS_LinkDriver(&SD_Driver, SD_Path) == 0)
    {
        /*##-2- Register the file system object to the FatFs module ##############*/
        res=f_mount(&SDFatFs, (TCHAR const*)SD_Path, 0);
        if(res == FR_OK)
        {
            /*##-4- Create and Open a new text file object with write access #####*/
            res=f_open(&MyFile, "MP_Nucleo.TXT", FA_CREATE_ALWAYS | FA_WRITE);
            if(res == FR_OK)
            {
                /*##-5- Write data to the text file ################################*/
                res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
                if((byteswritten == 0) || (res == FR_OK))
                {
                    /*##-6- Close the open text file #################################*/
                    f_close(&MyFile);
                }
            }
        }
    }
    /*##-11- Unlink the micro SD disk I/O driver ###############################*/
    FATFS_UnLinkDriver(SD_Path);

int16_t pw[8];

  /* USER CODE BEGIN 5 */

	setvbuf(stdout, NULL, _IONBF, 0);

	printf("MatrixPilot STM32-nucleo\r\n");

	matrixpilot_init();
	udb_init_GPS();

  /* Infinite loop */
  for(;;)
  {
		static int i = 0;
		matrixpilot_loop();
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_Delay(25); // should give us very roughly 40Hz

//		if (++i > 5){
        // Read the inputs
        radioIn_getInput(pw, 8);
        // Update the servos
        set_pwm_outputs(pw);
//        i=0;
//		}

		if (tsirq)
		{
//			i = 0;
			tsirq = 0;
			printf("#");
//			fflush(stdout);
		}
		udb_run();
//		udb_heartbeat_40hz_callback(); // Run at 40Hz
//		udb_heartbeat_callback(); // Run at HEARTBEAT_HZ

//		osDelay(1);
  }

  /* USER CODE END 5 */

}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/