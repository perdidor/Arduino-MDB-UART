/*
 * LEDControl.h
 *
 * Created: 11.05.2019 11:00:39
 *  Author: root
 */ 


#ifndef LEDCONTROL_H_
#define LEDCONTROL_H_

#define CASHLESS1_OFF() PORTD &= ~(1 << PORTD7)
#define CASHLESS1_ON() PORTD |= (1 << PORTD7)
#define CASHLESS2_OFF() PORTB &= ~(1 << PORTB4)
#define CASHLESS2_ON() PORTB |= (1 << PORTB4)
#define USD1_OFF() PORTD &= ~(1 << PORTD6)
#define USD1_ON() PORTD |= (1 << PORTD6)
#define USD2_OFF() PORTD &= ~(1 << PORTD5)
#define USD2_ON() PORTD |= (1 << PORTD5)
#define USD3_OFF() PORTD &= ~(1 << PORTD4)
#define USD3_ON() PORTD |= (1 << PORTD4)

void ControlExtUartSpeedLEDs(void);




#endif /* LEDCONTROL_H_ */