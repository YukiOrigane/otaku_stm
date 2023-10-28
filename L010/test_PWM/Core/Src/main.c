#include "stm32l0xx.h"

void TIM2_Configuration(void);

void delay_ms(uint32_t ms) {
    // SysTick タイマのカウンタを設定
    SysTick->LOAD = 16000 * ms - 1;  // 16 MHzのクロックで1ミリ秒待つ場合

    // タイマを開始
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;

    // タイマが0になるまで待つ
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);

    // タイマを停止
    SysTick->CTRL = 0;
}

int main(void)
{
    // STM32L0xxのクロック設定（デフォルトのHSIクロックを使用）->MSI
	RCC->CR |= RCC_CR_HSION;	// HSI始動
	while(!(RCC->CR&RCC_CR_HSIRDY));	// クロック安定化を待機
	RCC->CFGR |= RCC_CFGR_SW_HSI;	// HSIをクロック源として選択

	// RCC（Reset and Clock Control）クロックを有効化
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

	GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE0_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE1_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE1_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE2_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE3_Pos);
	//GPIOA->MODER &= ~(3U << (2 * 0));  // ビット0をクリア（リセット）
	//GPIOA->MODER |= (2U << (2 * 0));   // PA0をAlternate Functionモードに設定
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL0_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL1_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL2_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL3_Pos);
	//GPIOA->AFR[0] |= (2U << (4 * 0));     // PA0のAlternate FunctionをAF2 (TIM2_CH1) に設定


	//GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk;
	//GPIOA->MODER |= (2 << GPIO_MODER_MODE0_Pos);
	//GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
	//GPIOA->MODER |= (2 << GPIO_MODER_MODE3_Pos);

	GPIOC->MODER &= ~GPIO_MODER_MODE15_Msk;
	GPIOC->MODER |= (GPIO_MODE_OUTPUT_PP << GPIO_MODER_MODE15_Pos);


    // TIM2の設定
    TIM2_Configuration();

    // TIM2を有効化
    TIM2->CR1 |= TIM_CR1_CEN;

    while (1)
    {
        // メインの処理をここに追加
    	TIM2->CCR4 = 1480;
    	GPIOC->BSRR |= GPIO_BSRR_BS_15;
    	for(int i = 0; i<10000; i++);
    	TIM2->CCR4 = 1600;
    	GPIOC->BRR |= GPIO_BRR_BR_15;
    	for(int i = 0; i<10000; i++);
    	//delay_ms(1000);
    }
}

void TIM2_Configuration(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // TIM2のクロック周波数を設定
    // 16 MHzクロックを使用する場合、プリスケーラを16に設定して1 MHzのタイマークロックを得る
    TIM2->PSC = 16 - 1;

    // PWMの設定
    TIM2->ARR = 19999;   // ARRレジスタを設定して周期を設定 (20Hz)
    TIM2->CCR1 = 5000;   // CCR1レジスタを設定してデューティーサイクルを設定
    TIM2->CCR2 = 10000;   // CCR1レジスタを設定してデューティーサイクルを設定
    TIM2->CCR3 = 15000;   // CCR1レジスタを設定してデューティーサイクルを設定

    // TIM2のチャンネル1をPWMモードに設定
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;  // PWMモード1
    TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;  // PWMモード1
    TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;  // PWMモード1
    TIM2->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;  // PWMモード1

    // TIM2のチャンネル1の出力を有効化
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

    // TIM2をPWMモードに設定
    TIM2->CR1 |= TIM_CR1_ARPE;  // 自動リロードプリロードを有効化
    TIM2->EGR |= TIM_EGR_UG;    // タイマー更新イベントを生成
    TIM2->CR1 |= TIM_CR1_CEN;   // TIM2を有効化
}
