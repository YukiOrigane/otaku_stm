#include "stm32l0xx.h"

void I2C_Slave_Configuration(void);
void GPIO_Configuration(void);

uint8_t receivedData;
volatile uint8_t dataReceived = 0;

int main(void)
{
    // STM32L0xxのクロック設定（デフォルトのHSIクロックを使用）
	// STM32L0xxのクロック設定（デフォルトのHSIクロックを使用）->MSI
	RCC->CR |= RCC_CR_HSION;	// HSI始動
	while(!(RCC->CR&RCC_CR_HSIRDY));	// クロック安定化を待機
	RCC->CFGR |= RCC_CFGR_SW_HSI;	// HSIをクロック源として選択

    // GPIOとI2Cの設定
	GPIO_Configuration();
	I2C_Slave_Configuration();

    while (1)
    {
        // メインの処理をここに追加
    	int i = 0;

    	if ((I2C1->ISR & I2C_ISR_ADDR) == I2C_ISR_ADDR){
    		I2C1->ICR |= I2C_ICR_ADDRCF;
    		GPIOC->ODR ^= GPIO_ODR_OD15;
    	}
    	while((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){
    		receivedData = I2C1->RXDR;
    		if (receivedData == 0x10){

    		}
    	}
    }
}

void I2C_Slave_Configuration(void)
{
    // I2C1のクロックを有効化
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // I2C1をリセット
    //RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    //RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    I2C1->CR1 &= ~I2C_CR1_PE;	// PEビットをクリア

    I2C1->TIMINGR = 0x00303D5B;

    // I2C1の設定を有効化
    I2C1->CR1 |= I2C_CR1_PE;

    // アドレス選択をクリア
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;

    // I2C1のスレーブアドレスを設定
    I2C1->OAR1 = (uint32_t)(0x30); // 7ビットアドレス 0x3E (マスク設定: 0)

    // I2C1のアドレスマッチングを有効化
    I2C1->OAR1 |= I2C_OAR1_OA1EN;

    // I2C1のデータ受信割り込み，アドレス一致割り込みを有効化
    //I2C1->CR1 |= I2C_CR1_RXIE;
    //I2C1->CR1 |= I2C_CR1_ADDRIE;

    // 割り込み設定
    //NVIC_EnableIRQ(I2C1_IRQn);
    //NVIC_SetPriority(I2C1_IRQn, 0);
}

void GPIO_Configuration(void)
{
    // GPIOAクロックを有効化
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // PA4 (I2C1_SCL)、PA10 (I2C1_SDA)をI2Cモードに設定
    GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE10);
    GPIOA->MODER |= (GPIO_MODE_AF_OD  << GPIO_MODER_MODE4_Pos) | (GPIO_MODE_AF_OD  << GPIO_MODER_MODE10_Pos);

    // I2C1_SCLとI2C1_SDAのオルタネート機能を設定
    GPIOA->AFR[0] |= (GPIO_AF3_I2C1 << GPIO_AFRL_AFSEL4_Pos);
    GPIOA->AFR[1] |= (GPIO_AF1_I2C1 << GPIO_AFRH_AFSEL10_Pos);

    GPIOA->OTYPER |= (1<<4) | (1<<10);

    GPIOA->OSPEEDR |= (GPIO_SPEED_FREQ_VERY_HIGH << GPIO_OSPEEDER_OSPEED4_Pos);
    GPIOA->OSPEEDR |= (GPIO_SPEED_FREQ_VERY_HIGH << GPIO_OSPEEDER_OSPEED10_Pos);

    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    GPIOC->MODER &= ~GPIO_MODER_MODE15_Msk;
    GPIOC->MODER |= (GPIO_MODE_OUTPUT_PP << GPIO_MODER_MODE15_Pos);
}

void I2C1_IRQHandler(void)
{/*
	if (I2C1->ISR & I2C_ISR_ADDR)
	{
		I2C1->ICR |= I2C_ICR_ADDRCF;
	}
    if (I2C1->ISR & I2C_ISR_RXNE)
    {
    	GPIOC->ODR ^= GPIO_ODR_OD15;
        // データを受信
        receivedData = I2C1->RXDR;
    }*/
}
