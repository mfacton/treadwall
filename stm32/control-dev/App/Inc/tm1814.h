#ifndef INC_TM1814_H_
#define INC_TM1814_H_

#include "inttypes.h"

#define TM1814_HIGH (66)
#define TM1814_LOW (33)

#define TM1814_BPP (4)
#define TM1814_PIXELS (8)
#define TM1814_BYTES (TM1814_BPP * TM1814_PIXELS)

#define TM1814_BUF_SIZE (TM1814_BPP * 16)

void Tm1814_SetCurrent(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

void Tm1814_SetColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void Tm1814_SetColorAll(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

void Tm1814_Update(void);

void Tm1814_HalfHandler(void);
void Tm1814_FinishedHandler(void);

#endif
