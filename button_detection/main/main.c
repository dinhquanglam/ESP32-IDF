#include<button.h>

#define BUTTON_1 2
#define BUTTON_2 4
button_event_t ev;
QueueHandle_t button_events = button_init(PIN_BIT(BUTTON_1) | PIN_BIT(BUTTON_2));
void app_main() {
while (1) {
    if (xQueueReceive(button_events, &ev, 1000) {
        if ((ev.pin == BUTTON_1) && (ev.event == BUTTON_DOWN)) {
            printf( "hello");
        }
        if ((ev.pin == BUTTON_2) && (ev.event == BUTTON_DOWN)) {
            // ...
        }
    }
}
}
