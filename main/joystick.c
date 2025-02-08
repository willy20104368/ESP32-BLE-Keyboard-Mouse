#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

#include "joystick.h"
#include "config.h"
#include "esp_hidd_prf_api.h" // hid api

#define JOY_TAG "JOYSTICK"


static EventGroupHandle_t device_event;
 
static adc_oneshot_unit_handle_t unit_handle;

extern uint16_t hid_conn_id;
 
void init_joystick(EventGroupHandle_t bt_group_event) {
    
    // group event
    device_event = bt_group_event;

    // adc
    adc_oneshot_unit_init_cfg_t unit_initer = {
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,     // default clk
        .ulp_mode = ADC_ULP_MODE_DISABLE,       // no ULP
        .unit_id = ADC_UNIT_1                   // use ADC1
    }; 
    adc_oneshot_new_unit(&unit_initer, &unit_handle);
 
    adc_oneshot_chan_cfg_t channel_initer = {
        .atten = ADC_ATTEN_DB_12,               // 12dB 
        .bitwidth = ADC_BITWIDTH_12             // 12bit
    };
    adc_oneshot_config_channel(unit_handle, JOYSTICK_X, &channel_initer);
    adc_oneshot_config_channel(unit_handle, JOYSTICK_Y, &channel_initer);

    // button
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << JOYSTICK_BUTTON)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // pullup
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE  // disable trigger
    };
    gpio_config(&io_conf);

}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void joystickTask(void *pvParameters) {

    int x, y;

    uint8_t current_state = 1; // pull up (left button on mouse)
    uint8_t previous_state = current_state;
    uint32_t btn_time = 0;
    uint32_t current_time;

    while(1) {

        // block until bt is connected
        while(xEventGroupWaitBits(device_event, BT_CONNECTED, pdFALSE, pdFALSE, portMAX_DELAY)){
            adc_oneshot_read(unit_handle, JOYSTICK_X, &x);
            adc_oneshot_read(unit_handle, JOYSTICK_Y, &y);

            current_state = gpio_get_level(JOYSTICK_BUTTON);
            
            int xDeviation  = x - CENTERX;
            int yDeviation  = y - CENTERY;
            
            int DeltaX = 0, DeltaY = 0;

            if(abs(xDeviation) > MARGIN){
                DeltaX = map(xDeviation, -CENTERX, MAX_VAL-CENTERX, -MOVE_SPEED, MOVE_SPEED);
            }

            if(abs(yDeviation) > MARGIN){
                DeltaY = map(yDeviation, -CENTERY, MAX_VAL-CENTERY, -MOVE_SPEED, MOVE_SPEED);
            }


            if((DeltaX != 0 || DeltaY != 0)){
                xEventGroupSetBits(device_event, DEVICE_USED); 
                // screem scroll
                if(current_state == 0){
                    esp_hidd_send_mouse_value(hid_conn_id, 0, 0, 0, -DeltaY / MOVE_SCROLL_RATIO, DeltaX / MOVE_SCROLL_RATIO);
                }
                // mouse move
                else
                    esp_hidd_send_mouse_value(hid_conn_id, 0, DeltaX, DeltaY, 0, 0);
            }
            else{
                current_time = esp_timer_get_time() / 1000;
                if(btn_time == 0){
                    btn_time = current_time; // first trigger
                    previous_state = current_state;
                    xEventGroupSetBits(device_event, DEVICE_USED); 
                }
                else if(current_time - btn_time >= DEBOUNCE_TIME_JOY && previous_state != current_state){
                    if(previous_state == 0)
                        esp_hidd_send_mouse_value(hid_conn_id, 0, 0, 0, 0, 0); // release left button 
                    else
                        esp_hidd_send_mouse_value(hid_conn_id, LEFT_BUTTON, 0, 0, 0, 0); // press button
                    btn_time = 0;
                    previous_state = current_state;
                    xEventGroupSetBits(device_event, DEVICE_USED); 
                }                
            }
                
            vTaskDelay(INTERVAL / portTICK_PERIOD_MS);
        }
        
    }
}

