#include <stdio.h>

#define LEFT_BUTTON 0x01
#define RIGHT_BUTTON 0x02
#define MIDDLE_BUTTON 0x04

#define DEBOUNCE_TIME_JOY 100 //joystick button debounce time

#define CENTERX 1930 // joystick x adc value
#define CENTERY 1850 // joystick y adc value

#define MAX_VAL 4095 // joystick adc max value

#define MARGIN 300
#define MOVE_SPEED 6  // mouse move speed
#define MOVE_SCROLL_RATIO 2 //scrolling speed

#define INTERVAL 10 // adc reading interval(Lower for smoother movement)


void init_joystick(EventGroupHandle_t bt_group_event);

void joystickTask(void *pvParameter);

long map(long x, long in_min, long in_max, long out_min, long out_max);