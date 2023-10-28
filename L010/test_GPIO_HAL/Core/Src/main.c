#include "stm32l0xx_hal.h"

void SystemClock_Config(void);
void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    // GPIOポートCのクロックを有効化
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 出力モード（プッシュプル）
    GPIO_InitStruct.Pull = GPIO_NOPULL; // プルアップ/プルダウンなし
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // GPIOのスピード設定
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    while (1)
    {
        // LEDを点滅させる
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15); // LEDの状態を反転
        HAL_Delay(1000); // 1秒待つ
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
    /**Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_GetVoltageRange() != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    while (1)
    {
        // エラーハンドリングの処理をここに追加
    }
}
