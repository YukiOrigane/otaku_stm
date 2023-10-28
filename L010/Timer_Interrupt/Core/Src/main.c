#include "stm32l0xx.h"

void TIM21_Configuration(void);

int main(void)
{
	// STM32L0xxのクロック設定（デフォルトのHSIクロックを使用）->MSI
	RCC->CR |= RCC_CR_HSION;	// HSI始動
	while(!(RCC->CR&RCC_CR_HSIRDY));	// クロック安定化を待機
	RCC->CFGR |= RCC_CFGR_SW_HSI;	// HSIをクロック源として選択

	// GPIOCクロックを有効化
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

	// PC15を出力に設定
	GPIOC->MODER &= ~GPIO_MODER_MODE15_Msk;
	GPIOC->MODER |= (GPIO_MODE_OUTPUT_PP << GPIO_MODER_MODE15_Pos);

    // TIM21の設定
    TIM21_Configuration();

    while (1)
    {
        // メインの処理をここに追加
    }
}

void TIM21_Configuration(void)
{
    // TIM21のクロックを有効化
    RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;

    // TIM21のプリスケーラを設定して1kHzのタイマークロックを得る
    TIM21->PSC = 15999;  // 16 MHzのクロックから1kHzに設定

    // ARRレジスタを設定して1秒ごとに割り込みを発生させる
    TIM21->ARR = 999;  // 1秒（1000ミリ秒）ごとに割り込みを発生させる

    // タイマ割り込みを有効化
    TIM21->DIER |= TIM_DIER_UIE;

    // TIM21のクロックを有効化
    TIM21->CR1 |= TIM_CR1_CEN;

    // 割り込み設定
    NVIC_EnableIRQ(TIM21_IRQn);
    NVIC_SetPriority(TIM21_IRQn, 0);  // 割り込み優先度を設定
}

void TIM21_IRQHandler(void)
{
    // タイマ割り込み処理
    if (TIM21->SR & TIM_SR_UIF)
    {
        TIM21->SR &= ~TIM_SR_UIF;  // 割り込みフラグをクリア

        GPIOC->ODR ^= GPIO_ODR_OD15;
    }
}
