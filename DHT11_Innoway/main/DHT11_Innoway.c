/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#include "dht11.h"

#define LED_GPIO 19
#define BTN_GPIO 18

static const char *TAG = "MQTT_EXAMPLE";


static void log_error_if_nonzero(const char * message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic", "CONNECTED", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);



           // msg_id = esp_mqtt_client_unsubscribe(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic");
            //ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic", "SUBSCRIBED", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);



            break;
       case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);


            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;



    }
    return ESP_OK;
}



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);

       esp_mqtt_event_handle_t event = event_data;
       esp_mqtt_client_handle_t client = event->client;


       int temNum = DHT11_read().temperature;
       //printf("value: %d", temNum);
       char tem[20] = "Temperature: ";
       char temStr[20];
       itoa(temNum,temStr,10);
       strcat(tem,temStr);

       int humNum = DHT11_read().humidity;
       //printf("value: %d", humNum);
       char hum[20] = "Humidity: ";
       char humStr[20];
       itoa(humNum,humStr,10);
       strcat(hum,humStr);

       vTaskDelay(5000 / portTICK_PERIOD_MS);
       esp_mqtt_client_publish(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic",tem, 0, 0, 0);

       esp_mqtt_client_publish(client, "messages/502b8cf4-d404-4d1c-9f24-2462b4758645/topic",hum, 0, 0, 0);
       //printf("Temperature is %d \n", DHT11_read().temperature);






       }


static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
		.username = "dqlam",
		.password = "tPOLldgihpFm42wy6DC5ni7m9dpPpFox"


    };


    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void app_main(void *event_data)
{
	 gpio_pad_select_gpio(LED_GPIO);
	 gpio_pad_select_gpio(BTN_GPIO);

	    /* Set the GPIO as a push/pull output */
	 gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

	 gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
	 gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);

	 //while(1){
	 DHT11_init(GPIO_NUM_4);
	 //printf("Temperature is %d \n", DHT11_read().temperature);
	 //printf("Humidity is %d\n", DHT11_read().humidity);
	 //vTaskDelay(2000 / portTICK_PERIOD_MS);
	// }


    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

}
