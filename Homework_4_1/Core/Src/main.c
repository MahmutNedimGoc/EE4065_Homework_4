/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "model_data_q1.h" // Python'dan gelen agirliklar ve resim
#include <math.h>       // pow, sqrt, exp fonksiyonlari icin
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// ---------------------------------------------------------
// Hu Moment Hesaplama Fonksiyonu (OpenCV Mantigiyla)
// ---------------------------------------------------------
void calculate_hu_moments(const uint8_t img[28][28], float hu[7]) {
    // 1. Raw Moments (m_pq) Hesaplama
    // OpenCV'de binaryImage=True kullanildigi icin pixel > 0 ise 1.0 kabul edilir.
    double m00 = 0, m10 = 0, m01 = 0;
    // double m20 = 0, m11 = 0, m02 = 0; // Merkez momentlerde tekrar hesaplayacagiz

    // Agirlik merkezini bulmak icin ilk tur
    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            // Python'daki cv2.moments(img, True) mantigi:
            // Pixel degeri 0 degilse 1 kabul et.
            double val = (img[y][x] > 0) ? 1.0 : 0.0;

            m00 += val;
            m10 += x * val;
            m01 += y * val;
        }
    }

    // Eger resim tamamen siyahsa bolme hatasi olmasin
    if (m00 == 0) {
        for(int i=0; i<7; i++) hu[i] = 0.0f;
        return;
    }

    // 2. Center of Mass (Agirlik Merkezi)
    double cx = m10 / m00;
    double cy = m01 / m00;

    // 3. Central Moments (mu_pq)
    // Formül: sum((x - cx)^p * (y - cy)^q * val)
    double mu20 = 0, mu11 = 0, mu02 = 0;
    double mu30 = 0, mu21 = 0, mu12 = 0, mu03 = 0;

    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            double val = (img[y][x] > 0) ? 1.0 : 0.0;
            double dx = x - cx;
            double dy = y - cy;

            mu20 += dx * dx * val;
            mu11 += dx * dy * val;
            mu02 += dy * dy * val;
            mu30 += dx * dx * dx * val;
            mu21 += dx * dx * dy * val;
            mu12 += dx * dy * dy * val;
            mu03 += dy * dy * dy * val;
        }
    }

    // 4. Normalized Central Moments (nu_pq)
    // Formül: nu_pq = mu_pq / m00^(1 + (p+q)/2)
    double inv_m00_2 = 1.0 / (pow(m00, 2));     // p+q=2 icin
    double inv_m00_2_5 = 1.0 / (pow(m00, 2.5)); // p+q=3 icin

    double nu20 = mu20 * inv_m00_2;
    double nu11 = mu11 * inv_m00_2;
    double nu02 = mu02 * inv_m00_2;
    double nu30 = mu30 * inv_m00_2_5;
    double nu21 = mu21 * inv_m00_2_5;
    double nu12 = mu12 * inv_m00_2_5;
    double nu03 = mu03 * inv_m00_2_5;

    // 5. Hu Moments (h1..h7) Hesaplama
    // OpenCV HuMoments formülleri
    double t1 = nu20 + nu02;
    double t2 = nu20 - nu02;
    double t3 = nu30 - 3 * nu12;
    double t4 = 3 * nu21 - nu03;
    double t5 = nu30 + nu12;
    double t6 = nu21 + nu03;

    hu[0] = (float)t1;
    hu[1] = (float)(t2 * t2 + 4 * nu11 * nu11);
    hu[2] = (float)(t3 * t3 + t4 * t4);
    hu[3] = (float)(t5 * t5 + t6 * t6);
    hu[4] = (float)(t3 * t5 * (t5 * t5 - 3 * t6 * t6) + t4 * t6 * (3 * t5 * t5 - t6 * t6));
    hu[5] = (float)(t2 * (t5 * t5 - t6 * t6) + 4 * nu11 * t5 * t6);
    hu[6] = (float)(t4 * t5 * (t5 * t5 - 3 * t6 * t6) - t3 * t6 * (3 * t5 * t5 - t6 * t6));
}

// ---------------------------------------------------------
// Tahmin Fonksiyonu (Yapay Zeka Modeli)
// ---------------------------------------------------------
float predict_digit(float hu_moments[7]) {
    float sum = 0.0f;
    float normalized_val;

    // Agirliklar ve Normalizasyon ile Hesaplama
    for (int i = 0; i < 7; i++) {
        // (Deger - Ortalama) / StandartSapma
        normalized_val = (hu_moments[i] - MODEL_MEAN[i]) / MODEL_STD[i];

        // Agirlikla carp ve toplama ekle
        sum += normalized_val * MODEL_WEIGHTS[i];
    }

    // Bias Ekle
    sum += MODEL_BIAS;

    // Sigmoid uygula
    return sigmoid(sum);
}

// Sonuclari saklamak icin global degiskenler (Debug ekraninda izlemek icin)
float calculated_hu[7];
float prediction_result = 0.0f;
int detected_class = -1; // 0: SIFIR, 1: SIFIR DEGIL
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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  calculate_hu_moments(sample_image, calculated_hu);

    // 2. Modeli Calistir (Tahmin Et)
  prediction_result = predict_digit(calculated_hu);

    // 3. Karar Ver (Threshold: 0.5)
  if (prediction_result < 0.5f) {
        detected_class = 0; // Bu bir SIFIR rakami
        // Eger bir LED varsa yakabilirsiniz: HAL_GPIO_WritePin(...);
 } else {
        detected_class = 1; // Bu SIFIR DEGIL (Baska bir rakam)
 }
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
