/* GPIO configuration */

/* Joystick gpio */
#define JOYSTICK_X ADC_CHANNEL_0 // gpio36
#define JOYSTICK_Y ADC_CHANNEL_3 // gpio39
#define JOYSTICK_BUTTON 33 //gpio33


/* buttons gpio*/
#define BUTTON_1 32
#define BUTTON_2 16
#define BUTTON_3 17
#define BUTTON_4 21
#define BUTTON_5 22

/* RTC PIN*/
#define RTC_PIN 33


/* EVENT configuration*/
#define BT_CONNECTED (1 << 0) // bt connected: set bit, disconnect: clear bit
#define DEVICE_USED (1 << 1) // device is used

#define IDLE_TIME 30000 // 30ms 