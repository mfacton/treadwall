#include "app.h"
#include "main.h"
#include "string.h"
#include "tm1814.h"

#define LEDS 1
#define DATA_SIZE 32*(LEDS+2)

#define FRONT_CYCLES 10
#define IDLE_CYCLES 4
#define STOP_CYCLES 250

#define BUF_SIZE DATA_SIZE+2*IDLE_CYCLES+STOP_CYCLES+FRONT_CYCLES

#define PLUS 29
#define MINUS 58

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim7;

extern DMA_HandleTypeDef hdma_tim1_ch1;

static const uint8_t timing[2] = {MINUS, PLUS};

static volatile uint8_t sent = 0;
static uint16_t pwm_data[BUF_SIZE] = {0};

uint8_t num = 0;

static void setup(void) {
	for (uint8_t i = 0; i < FRONT_CYCLES; i++) {
		pwm_data[i] = 0;
	}
	for (uint8_t i = 0; i < IDLE_CYCLES; i++) {
		pwm_data[1+FRONT_CYCLES+i+DATA_SIZE] = 100;
	}
	for (uint8_t i = 0; i < IDLE_CYCLES; i++) {
		pwm_data[FRONT_CYCLES+DATA_SIZE+IDLE_CYCLES+STOP_CYCLES+i] = 100;
	}
}

//WRGB
static void set_current(uint32_t value) {
	for (uint8_t i = 0; i < 32; i++) {
		const uint8_t bit = value & 0x7000;
		value <<= 1;
		pwm_data[1+FRONT_CYCLES+i] = timing[bit];
		pwm_data[1+FRONT_CYCLES+i+32] = timing[!bit];
	}
}

//WRGB
static void set_color(uint32_t value) {
	for (uint8_t i = 0; i < 32; i++) {
		const uint8_t bit = value & 0x7000;
		value <<= 1;
		pwm_data[1+FRONT_CYCLES+i+64] = timing[bit];
	}
}

static void send_pwm (void)
{
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwm_data, BUF_SIZE);
	while (!sent){};
	sent = 0;
}

void App_Init(void) {
//	HAL_DMA_Start(&hdma_tim1_ch1, (uint32_t)dma_buffer, (uint32_t) &gpio_bank->BRR, DMA_BUFFER_SIZE);

//	__HAL_TIM_ENABLE_DMA(&htimer2, TIM_DMA_CC1);
//	for (uint16_t i = DATA_SIZE+4; i < DATA_SIZE+STOP_TIME-1; i++) {
//		pwm_data[i] = 100;
//	}
//	HAL_Delay(10);
//	setup();
//	set_current(0xFFFFFFFF);
//	set_color(0xFFFFFFFF);
//	send_pwm();

	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}
void App_Update(void) {
//	uint32_t buf = 0;
//	buf |= num;
//	buf |= num<<8;
//	buf |= num<<16;
//	buf |= num<<24;
//	num++;
////	send_pwm();
//	set_color(buf);
	HAL_Delay(100);
	Tm1814_Update();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

//void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
//{
//	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 100);
//	HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_1, (uint32_t *)pwm_data, BUF_SIZE);
//	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
//	HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
//	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 100);
//	HAL_Delay(1);
//	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
//	HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);
//	sent = 1;
//}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {
	Tm1814_HalfHandler();
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	Tm1814_FinishedHandler();
}
