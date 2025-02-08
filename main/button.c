#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <inttypes.h>

#include "button.h" // button pin settings
#include "config.h"
#include "esp_hidd_prf_api.h" // hid api

#define TAG "HID_KeyBoard"


static uint8_t keycodes[] = {
    0x00, // release
    0x04, // 'a'
    0x05, // 'b'
    0x06, // 'c'
    0x07, // 'd'
    0x08, // 'e'
    0x09, // 'f'
    0x0A, // 'g'
    0x0B, // 'h'
    0x0C, // 'i'
    0x0D, // 'j'
    0x0E, // 'k'
    0x0F, // 'l'
    0x10, // 'm'
    0x11, // 'n'
    0x12, // 'o'
    0x13, // 'p'
    0x14, // 'q'
    0x15, // 'r'
    0x16, // 's'
    0x17, // 't'
    0x18, // 'u'
    0x19, // 'v'
    0x1A, // 'w'
    0x1B, // 'x'
    0x1C, // 'y'
    0x1D  // 'z'
    // add more keycodes(up to 32)
};

static QueueHandle_t buttonQueue;   

static SemaphoreHandle_t buttonSemaphore;

static EventGroupHandle_t device_event;

extern uint16_t hid_conn_id;

// extern volatile uint32_t last_time;

int keycode_len = sizeof(keycodes) / sizeof(uint8_t);




void IRAM_ATTR button_isr_handler(void *pvParameters) {

    if(xEventGroupGetBitsFromISR(device_event) & BT_CONNECTED){
        uint32_t gpio_num = (uint32_t) pvParameters;
        xQueueSendFromISR(buttonQueue, &gpio_num, NULL);
        xSemaphoreGiveFromISR(buttonSemaphore, NULL);
    }
    
}

void init_button(void){

    // set gpio, 
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << BUTTON_1)|
                        (1ULL << BUTTON_2) |
                        (1ULL << BUTTON_3) |
                        (1ULL << BUTTON_4) |
                        (1ULL << BUTTON_5)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // pullup
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE  // negaedge trigger
    };
    gpio_config(&io_conf);
    
    // set ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_1, button_isr_handler, (void*)1);
    gpio_isr_handler_add(BUTTON_2, button_isr_handler, (void*)2);
    gpio_isr_handler_add(BUTTON_3, button_isr_handler, (void*)3);
    gpio_isr_handler_add(BUTTON_4, button_isr_handler, (void*)4);
    gpio_isr_handler_add(BUTTON_5, button_isr_handler, (void*)5);

}

void buttonTask(void *pvParameters) {
    
    ESP_LOGI(TAG, "start buttonTask for %d buttons\n", BUTTON_NUMs);
    uint32_t btn_count[BUTTON_NUMs] = {0}; //5 buttons
    uint32_t button;
    uint32_t event = 0;
    uint32_t current_time;
    uint32_t eariest; // record time of first button
    bool finished = true; // btn_count
    bool no_empty = false; //queue

    while(1){
      //block avoid polling
      if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY)) {
        while ((no_empty = xQueueReceive(buttonQueue, &button, 0)) || !finished) {
            // get current time
            current_time = esp_timer_get_time() / 1000;
            eariest = current_time;
            // last_time = current_time; // update last event time
            event = 0;
            xEventGroupSetBits(device_event, DEVICE_USED); 
            // first time tigger
            if (no_empty && (btn_count[button - 1] == 0)) {
                btn_count[button - 1] = current_time;
                finished = false;
            }
            
            // check event
            for(int i = 0; i < BUTTON_NUMs; ++i){
                if((current_time - btn_count[i] >= DEBOUNCE_TIME_BTN) && (btn_count[i] != 0)){
                    eariest = eariest < btn_count[i] ? eariest : btn_count[i]; 
                    event |= (1UL << i);
                    btn_count[i] = 0; //reset
                }
            }
            // detect multiple buttons
            if(event > 0){
                bool all_done = true;
                for(int i = 0; i < BUTTON_NUMs; ++i){
                    if((btn_count[i] - eariest <= EVENT_TIME) && (btn_count[i] != 0)){
                        event |= (1UL << i);
                        btn_count[i] = 0; //reset
                    }
                    //still have events in btn_count?
                    all_done &= (btn_count[i] == 0);
                }
                finished |= all_done;
            } 

            if(event > 0 && event < keycode_len + 1){
                //send one key hid report
                esp_hidd_send_keyboard_value(hid_conn_id, 0, &keycodes[event], 1); //press
                esp_hidd_send_keyboard_value(hid_conn_id, 0, &keycodes[0], 1); // release
            }

        }
      }
    }
}

void init_buttonQueue(void){

    buttonQueue = xQueueCreate(QUEUE_SIZE, sizeof(uint32_t));

    if(buttonQueue == NULL){
        ESP_LOGE(TAG, "Failed to initialize buttonQueue");
    }

    ESP_LOGI(TAG, "Initialize button queue\n");

}

void init_buttonSemaphore(void){

    buttonSemaphore = xSemaphoreCreateBinary();

    if(buttonSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize Semaphore");
    }


    ESP_LOGI(TAG, "Initialize button Semaphore\n");
}

void init_button_task(EventGroupHandle_t bt_group_event){

    device_event = bt_group_event;

    init_button();
    init_buttonQueue();
    init_buttonSemaphore();
}