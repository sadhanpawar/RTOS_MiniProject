#ifndef RTOS_H_

#include <stdint.h>

#define YELLOW_LED PORTC,4
#define RED_LED PORTC,5
#define GREEN_LED PORTC,6
#define ORANGE_LED PORTC,7


#define PUSH_BUTTON_1 PORTB,0
#define PUSH_BUTTON_2 PORTB,1
#define PUSH_BUTTON_3 PORTE,4
#define PUSH_BUTTON_4 PORTE,5
#define PUSH_BUTTON_5 PORTE,1
#define PUSH_BUTTON_6 PORTD,2


//asm
extern uint32_t* getPsp(void);
extern void setPsp(uint32_t* ptr);
extern void setAspPsp(void);
extern void setAspMsp(void);
extern void setTmplUnP(void);
extern void setTmplP(void);
extern void setPendSv(void);
extern uint32_t* getMsp(void);

void rtosInit(void);
void initPbs_leds(void);
void test_pb_leds(void);
void check_interrupts(void);
uint8_t getPbs(void);

#endif /* RTOS_H_ */
