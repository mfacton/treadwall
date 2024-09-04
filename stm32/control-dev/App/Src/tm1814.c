#include "tm1814.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_ch1;
extern TIM_HandleTypeDef htim7;

// LED color buffer
static uint8_t rgb_arr[TM1814_BYTES] = {0};

// LED write buffer
static volatile uint32_t wr_buf[TM1814_BUF_SIZE] = {0};
static volatile uint_fast8_t wr_buf_p = 0;

void Tm1814_SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  rgb_arr[4 * index] = g;
  rgb_arr[4 * index + 1] = r;
  rgb_arr[4 * index + 2] = b;
  rgb_arr[4 * index + 3] = w;
}

void Tm1814_SetColorAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for(uint_fast8_t i = 0; i < TM1814_PIXELS; ++i) {
	  Tm1814_SetColor(i, r, g, b, w);
  }
}

void Tm1814_Update(void) {
	rgb_arr[0] = 0b01001111;
  if(wr_buf_p != 0 || hdma_tim1_ch1.State != HAL_DMA_STATE_READY) {
    // Ongoing transfer, cancel!
    for(uint8_t i = 0; i < TM1814_BUF_SIZE; ++i) wr_buf[i] = 0;
    wr_buf_p = 0;
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
    return;
  }
  // Ooh boi the first data buffer half (and the second!)
  for(uint_fast8_t i = 0; i < 8; ++i) {
    wr_buf[i     ] = TM1814_LOW << (((rgb_arr[0] << i) & 0x80) > 0);
    wr_buf[i +  8] = TM1814_LOW << (((rgb_arr[1] << i) & 0x80) > 0);
    wr_buf[i + 16] = TM1814_LOW << (((rgb_arr[2] << i) & 0x80) > 0);
    wr_buf[i + 24] = TM1814_LOW << (((rgb_arr[3] << i) & 0x80) > 0);
    wr_buf[i + 32] = TM1814_LOW << (((rgb_arr[4] << i) & 0x80) > 0);
    wr_buf[i + 40] = TM1814_LOW << (((rgb_arr[5] << i) & 0x80) > 0);
    wr_buf[i + 48] = TM1814_LOW << (((rgb_arr[6] << i) & 0x80) > 0);
    wr_buf[i + 56] = TM1814_LOW << (((rgb_arr[7] << i) & 0x80) > 0);
  }

  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)wr_buf, TM1814_BUF_SIZE);
  wr_buf_p = 2; // Since we're ready for the next buffer
}

void Tm1814_HalfHandler(void) {
  // DMA buffer set from LED(wr_buf_p) to LED(wr_buf_p + 1)
  if(wr_buf_p < TM1814_PIXELS) {
    // We're in. Fill the even buffer
    for(uint_fast8_t i = 0; i < 8; ++i) {
      wr_buf[i     ] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p    ] << i) & 0x80) > 0);
      wr_buf[i +  8] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 1] << i) & 0x80) > 0);
      wr_buf[i + 16] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 2] << i) & 0x80) > 0);
      wr_buf[i + 24] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 3] << i) & 0x80) > 0);
    }
    wr_buf_p++;
  } else if (wr_buf_p < TM1814_PIXELS + 2) {
    // Last two transfers are resets. 64 * 1.25 us = 80 us == good enough reset
    // First half reset zero fill
    for(uint8_t i = 0; i < TM1814_BUF_SIZE / 2; ++i) wr_buf[i] = 0;
    wr_buf_p++;
  }
}

void Tm1814_FinishedHandler(void) {
  // DMA buffer set from LED(wr_buf_p) to LED(wr_buf_p + 1)
  if(wr_buf_p < TM1814_PIXELS) {
    // We're in. Fill the odd buffer
    for(uint_fast8_t i = 0; i < 8; ++i) {
      wr_buf[i + 32] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p    ] << i) & 0x80) > 0);
      wr_buf[i + 40] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 1] << i) & 0x80) > 0);
      wr_buf[i + 48] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 2] << i) & 0x80) > 0);
      wr_buf[i + 56] = TM1814_LOW << (((rgb_arr[4 * wr_buf_p + 3] << i) & 0x80) > 0);
    }
    wr_buf_p++;
  } else if (wr_buf_p < TM1814_PIXELS + 2) {
    // Second half reset zero fill
    for(uint8_t i = TM1814_BUF_SIZE / 2; i < TM1814_BUF_SIZE; ++i) wr_buf[i] = 0;
    ++wr_buf_p;
  } else {
    // We're done. Lean back and until next time!
    wr_buf_p = 0;
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
  }
}
