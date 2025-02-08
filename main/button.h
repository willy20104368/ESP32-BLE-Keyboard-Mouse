#include "freertos/queue.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"

#define QUEUE_SIZE 10

#define BUTTON_NUMs 5

// debounce
#define DEBOUNCE_TIME_BTN 200
#define EVENT_TIME 150

void init_buttonQueue(void);

void init_buttonSemaphore(void);

// initialize button settings
void init_button(void);

// Recieve button task from queue
void buttonTask(void *pvParameters);

// initialize all settings
void init_button_task(EventGroupHandle_t bt_group_evnet);