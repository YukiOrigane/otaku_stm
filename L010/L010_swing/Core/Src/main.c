#include <stm32l0xx.h>
#include <stdbool.h>
#include <math.h>

#define I2C_ID 0x80         // I2CのID．大事

#define SERVO_AMPLITUDE 300 // サーボの振幅
#define SERVO_MIDVALUE 1780 // サーボの中央値

//グローバル変数定義
double phase;
double natural_freq;
double couple_intensity;
double dphi = 0.0;   // 位相差

uint8_t phase_prescale;
uint16_t led_color[3];

bool is_servo_move;
bool is_servo_neutral;
bool is_led_flash;
bool is_read_volume;

uint8_t received_data[8];
bool is_i2c_received;	// I2Cの受信チェック
uint8_t received_size = 0;

// 設定関数群
void TIM21_Configuration(void);
void ADC_Configuration(void);
void GPIO_Configuration(void);
void TIM2_Configuration(void);
void EXTI_Configuration(void);
void I2C_Slave_Configuration(void);

int main (void){

	// クロック設定
	RCC->CR |= RCC_CR_HSION;	// HSI始動
	while(!(RCC->CR&RCC_CR_HSIRDY));	// クロック安定化を待機
	RCC->CFGR |= RCC_CFGR_SW_HSI;	// HSIをクロック源として選択

	// 各種機能初期化
	GPIO_Configuration();
	TIM21_Configuration();	// TIM21
	TIM2_Configuration();	// TIM2
	EXTI_Configuration();
	ADC_Configuration();
	I2C_Slave_Configuration();

    // グローバル変数初期化．関数使わない
    phase = 4.71;	// 3/2pi
    dphi = 0.0;
    natural_freq = 0.0;
    couple_intensity = 0.001;
    phase_prescale = 1;
    led_color[0] = 180;
    led_color[1] = 200;
    led_color[2] = 0;
    //is_servo_neutral = true;
    is_servo_move = true;//false;
    is_led_flash = true;//false;
    is_read_volume = true;
    is_i2c_received = false;

    // 機能有効化
    TIM2->CR1 |= TIM_CR1_CEN;

    while((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){
    	uint32_t dat = I2C1->RXDR;
    }

    while(1){
        // サーボ制御
        if (!is_servo_neutral){ // サーボニュートラル位置でない
            // 位相値をもとにサーボ角決定
            // 位相をスケールしてからsinに突っ込む
            // メモリ足りなければ色々考える
        	TIM2->CCR4 = (uint32_t)(sin(phase/phase_prescale)*SERVO_AMPLITUDE + SERVO_MIDVALUE);
        }else{	// サーボニュートラル位置
            // 固定値はミニマムの角度．真上になるように位置合わせ
        	TIM2->CCR4 = (uint32_t)(-1.0*SERVO_AMPLITUDE + SERVO_MIDVALUE);
        }

        // LED制御
        if (is_led_flash){  // 点灯時
        	TIM2->CCR3 = (200-led_color[0])*100;
            TIM2->CCR1 = (200-led_color[1])*100;
            TIM2->CCR2 = (200-led_color[2])*100;
        }else{
        	TIM2->CCR3 = 20000;
            TIM2->CCR1 = 20000;
            TIM2->CCR2 = 20000;
        }

        // I2C受信

        if ((I2C1->ISR & I2C_ISR_ADDR) == I2C_ISR_ADDR){
			I2C1->ICR |= I2C_ICR_ADDRCF;
		}else if ((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){
			uint8_t receivedData = I2C1->RXDR;
			received_data[received_size] = receivedData;
			received_size++;
			if (received_size > 8)
				received_size = 0;
			if (receivedData == 0xFE){	// メッセージの終了
				//GPIOC->ODR ^= GPIO_ODR_OD15;
				if (received_data[0] == 0x01){
					is_servo_move = true;
					is_servo_neutral = false;
				}
				if (received_data[0] == 0x02){
					is_servo_move = false;
					is_servo_neutral = false;
				}
				if (received_data[0] == 0x03){
					is_servo_move = false;
					is_servo_neutral = true;
				}
				if (received_data[0] == 0x10)
					is_led_flash = true;
				if (received_data[0] == 0x11)
					is_led_flash = false;
				if ((received_data[0] == 0x12)&&(received_size>4)){
					led_color[0] = received_data[1];
					led_color[1] = received_data[2];
					led_color[2] = received_data[3];
				}
				if (received_data[0] == 0x20)
					is_read_volume = true;
				if ((received_data[0] == 0x21)&&(received_size>3)){
					is_read_volume = false;
					natural_freq = 0.01*received_data[1] + 0.0001*received_data[2];
					// 直す
				}
				if ((received_data[0] == 0x31)&&(received_size>3)) {  // 固有角速度指定
					couple_intensity = received_data[1]*100 + received_data[2];
				}
				if ((received_data[0] == 0x32)&&(received_size>2)) {  // 位相のプリスケーラ指定．遅くなる
					phase_prescale = received_data[1];
				}
				received_size = 0;
			}
		}
        // I2C受信
        /*
		if ((I2C1->ISR & I2C_ISR_ADDR) == I2C_ISR_ADDR){
			I2C1->ICR |= I2C_ICR_ADDRCF;

		}else if ((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){    // 連続受信スタート
			uint8_t receivedData = I2C1->RXDR;    // 最初はアドレスのはず

			while(receivedData != 0xFE){            // 終了命令が来るまで受け取り継続

				while(!((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE));
				uint8_t receivedData = I2C1->RXDR;
				received_data[received_size] = receivedData;
				received_size++;
				if (received_size>8){
					break;
				}
			}
			GPIOC->ODR ^= GPIO_ODR_OD15;   // STATUSピン反転
			if (received_data[0] == 0x01)
				is_servo_move = true;
			if (received_data[0] == 0x02)
				is_servo_move = false;
			if (received_data[0] == 0x10)
				is_led_flash = true;
			if (received_data[0] == 0x11)
				is_led_flash = false;
			if ((received_data[0] == 0x12)&&(received_size>4)){
				led_color[0] = received_data[1];
				led_color[1] = received_data[2];
				led_color[2] = received_data[3];
			}
			if (received_data[0] == 0x20)
				is_read_volume = true;
			if ((received_data[0] == 0x21)&&(received_size>3)){
				is_read_volume = false;
				natural_freq = received_data[1]*100 + received_data[2];
				// 直す
			}
			if ((received_data[0] == 0x31)&&(received_size>3)) {  // 固有角速度指定
				couple_intensity = received_data[1]*100 + received_data[2];
			}
			if ((received_data[0] == 0x32)&&(received_size>2)) {  // 位相のプリスケーラ指定．遅くなる
				phase_prescale = received_data[1];
			}
			received_size = 0;
		}*/

        // ADC読み出し
        if (is_read_volume){
			ADC1->CR |= ADC_CR_ADSTART;	// ADC変換の開始
			while (!(ADC1->ISR & ADC_ISR_EOC));// 変換が完了するまで待つ
			uint8_t ADC_ReadValue = ADC1->DR;	// ADCの値を読み取り
			// TODO : 読み取り値の利用
			natural_freq = 0.00628 + 0.012*ADC_ReadValue/256;	// 2*3.14*90/(60*1000)
        }
    }
}

/******************** 割り込み処理 ************************/
// タイマ割り込み処理
void TIM21_IRQHandler(void)
{
    if (TIM21->SR & TIM_SR_UIF)
    {
        TIM21->SR &= ~TIM_SR_UIF;  // 割り込みフラグをクリア
        // 制御周期の更新
        if (is_servo_move){
        	phase += natural_freq + couple_intensity * dphi;
			if (phase > 6.28){  // mod発生
				phase = 0;
				//dphi = 0;
				GPIOC->ODR ^= GPIO_ODR_OD15;   // STATUSピン反転
			}
        }

    }

}

// I2C受信割り込み処理

void I2C1_IRQHandler(void)
{/*
	GPIOC->ODR ^= GPIO_ODR_OD15;
	static uint8_t received_size = 0;
    if (I2C1->ISR & I2C_ISR_RXNE)
    {
        // データを受信
    	received_data[received_size++] = I2C1->RXDR;
    }
    if (I2C1->ISR & I2C_ISR_STOPF)
	{
		// ストップ条件を受信
		I2C1->ICR = I2C_ICR_STOPCF; // ストップフラグをクリア
		received_size = 0;			// 受信サイズ初期化
		is_i2c_received = true;		// 受信したことを通知
	}*/
}

// 拍によるピン変化割り込み
void EXTI4_15_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PIF14)
    {
        // EXTI14の割り込みフラグをクリア
        EXTI->PR |= EXTI_PR_PIF14;
        // 拍データの受信
        dphi = -phase;
    }
}

/*************** 各種設定 ***********************/
void GPIO_Configuration(void)
{
    // GPIOAとGPIOCクロックを有効化
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;

    // PA0, PA1, PA2, PA3のPWM出力設定
    GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE0_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE1_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE1_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE2_Pos);
	GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
	GPIOA->MODER |= (GPIO_MODE_AF_PP << GPIO_MODER_MODE3_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL0_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL1_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL2_Pos);
	GPIOA->AFR[0] |= (GPIO_AF2_TIM2 << GPIO_AFRL_AFSEL3_Pos);

	// PA5をアナログ入力に
	GPIOA->MODER |= GPIO_MODER_MODE5;	// アナログモードに設定
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;	// プルダウンを無効化

	// PA4 (I2C1_SCL)、PA10 (I2C1_SDA)をI2Cモードに設定
	GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE10);
	GPIOA->MODER |= (GPIO_MODE_AF_OD  << GPIO_MODER_MODE4_Pos) | (GPIO_MODE_AF_OD  << GPIO_MODER_MODE10_Pos);

	// I2C1_SCLとI2C1_SDAのオルタネート機能を設定
	GPIOA->AFR[0] |= (GPIO_AF3_I2C1 << GPIO_AFRL_AFSEL4_Pos);
	GPIOA->AFR[1] |= (GPIO_AF1_I2C1 << GPIO_AFRH_AFSEL10_Pos);

	GPIOA->OTYPER |= (1<<4) | (1<<10);

	GPIOA->OSPEEDR |= (GPIO_SPEED_FREQ_VERY_HIGH << GPIO_OSPEEDER_OSPEED4_Pos);
	GPIOA->OSPEEDR |= (GPIO_SPEED_FREQ_VERY_HIGH << GPIO_OSPEEDER_OSPEED10_Pos);

    // PC14を入力に設定 (トリガー)
    GPIOC->MODER &= ~(3U << GPIO_MODER_MODE14_Pos);  // ビット28-29をクリア（リセット）
    // PC15を出力に設定（デバッグ）
    GPIOC->MODER &= ~GPIO_MODER_MODE15_Msk;
    GPIOC->MODER |= (GPIO_MODE_OUTPUT_PP << GPIO_MODER_MODE15_Pos);
}

void TIM21_Configuration(void)
{
    // TIM21のクロックを有効化
    RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;

    // TIM21のプリスケーラを設定して1kHzのタイマークロックを得る
    TIM21->PSC = 16 - 1;  // 16 MHzのクロックから1kHzに設定

    // ARRレジスタを設定して0.001秒ごとに割り込みを発生させる
    TIM21->ARR = 1000 - 1;  // 1ミリ秒ごとに割り込みを発生させる

    // タイマ割り込みを有効化
    TIM21->DIER |= TIM_DIER_UIE;

    // TIM21のクロックを有効化
    TIM21->CR1 |= TIM_CR1_CEN;

    // 割り込み設定
    NVIC_EnableIRQ(TIM21_IRQn);
    NVIC_SetPriority(TIM21_IRQn, 0);  // 割り込み優先度を設定
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

void I2C_Slave_Configuration(void)
{
	// I2C1のクロックを有効化
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	I2C1->CR1 &= ~I2C_CR1_PE;	// PEビットをクリア

	I2C1->TIMINGR = 0x00303D5B;

	// I2C1の設定を有効化
	I2C1->CR1 |= I2C_CR1_PE;

	// アドレス選択をクリア
	I2C1->OAR1 &= ~I2C_OAR1_OA1EN;

	// I2C1のスレーブアドレスを設定
	I2C1->OAR1 = (uint32_t)(I2C_ID); // 7ビットアドレス 0x3E (マスク設定: 0)

	// I2C1のアドレスマッチングを有効化
	I2C1->OAR1 |= I2C_OAR1_OA1EN;
}

void EXTI_Configuration(void)
{
    // EXTIコントローラのクロックを有効化
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // EXTI14にPC14を接続
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI14;
    SYSCFG->EXTICR[3] |= (2U << SYSCFG_EXTICR4_EXTI14_Pos);

    // EXTI14のトリガータイプを設定 (立ち上がりエッジ)
    EXTI->RTSR |= EXTI_RTSR_RT14;

    // EXTI14の割り込みを有効化
    EXTI->IMR |= EXTI_IMR_IM14;

    // 割り込み設定
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
}

