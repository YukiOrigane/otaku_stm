#include "stm32l0xx.h"

void ADC_Configuration(void);
void GPIO_Configuration(void);

uint16_t ADC_ReadValue = 0;

int main(void)
{
    // STM32L0xxのクロック設定（デフォルトのHSIクロックを使用）

    // GPIOとADCの設定
    GPIO_Configuration();
    ADC_Configuration();

    while (1)
    {
        // ADC変換の開始
        ADC1->CR |= ADC_CR_ADSTART;

        // 変換が完了するまで待つ
        while (!(ADC1->ISR & ADC_ISR_EOC))
        {
            // 何もしない
        }

        // ADCの値を読み取り
        ADC_ReadValue = ADC1->DR;
    }
}

void ADC_Configuration(void)
{
    // ADC1のクロックを有効化
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

    // ADCのキャリブレーションを有効化
    ADC1->CR = ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
    {
        // キャリブレーションが完了するまで待つ
    }

    // ADCを設定
    ADC1->CFGR1 |= ADC_CFGR1_RES_1; // 8ビットの分解能を設定
    ADC1->CHSELR |= ADC_CHSELR_CHSEL5; // PA5をADCの入力チャンネルとして選択
    ADC1->CR |= ADC_CR_ADEN; // ADCを有効化
}

void GPIO_Configuration(void)
{
    // GPIOAクロックを有効化
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // PA5をアナログモードに設定
    GPIOA->MODER |= GPIO_MODER_MODE5;

    // PA5のプルダウンを無効化
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;
}
