/*
 * LEDControl_M.h
 *
 * Created: 18.05.2019 10:17:44
 *  Author: root
 */ 


#ifndef LEDCONTROL_M_H_
#define LEDCONTROL_M_H_

void CCLED_ON(void);
void CCLED_OFF(void);
void CHLED_ON(uint8_t index);
void CHLED_OFF(uint8_t index);
void BVLED_ON(void);
void BVLED_OFF(void);
void CDLED_ON(uint8_t index);
void CDLED_OFF(uint8_t index);
void USDLED_ON(uint8_t index);
void USDLED_OFF(uint8_t index);

#endif /* LEDCONTROL_M_H_ */