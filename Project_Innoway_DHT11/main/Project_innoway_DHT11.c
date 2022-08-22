
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_timer.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"
#include "driver/uart.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_tls.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_timer.h"
#include <inttypes.h>
#include "driver/gpio.h"
#include <sys/param.h>
#include <esp_http_server.h>
#include <stdbool.h>
#include "esp_sntp.h"

#include "dht11.h"


#define EXAMPLE_ESP_WIFI_SSID      "Lamcuong"
#define EXAMPLE_ESP_WIFI_PASS      "0963875959"
#define EXAMPLE_ESP_MAXIMUM_RETRY  500
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
static const char *TAG1 = "wifi station";
static EventGroupHandle_t s_wifi_event_group;

void ConnectWiFi(void);
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
void delay(uint32_t time);


void ConnectMQTT(void *arg);
void Button(void *arg);
void StartLed(void *arg);


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);


char JSON[100];
char Str_ND[100];
char Str_DA[100];
unsigned long last1 = 0;

void Button_1(void);

int btn = 0;// 1 => nhietdo, 2 do am
int btn_Start = 0;
#define NUT 18

#define LED_ND 19
#define LED_DA 5
int TT_LED_ND = 0;
int TT_LED_DA = 0;

void DataJsonNhietDo(void);

void DataJsonDoAm(void);

void DataJsonNhietDoDoAm(void);

cJSON *str_json;
int Start_led_ND = 0;
int Dem_Start_led_ND = 0;
unsigned long lastLed_ND = 0;

int Start_led_DA = 0;
int Dem_Start_led_DA = 0;
unsigned long lastLed_DA = 0;

int Start_led_NDDA = 0;
int Dem_Start_led_NDDA = 0;
unsigned long lastLed_NDDA = 0;

int btn1 = 0;
static const char *TAG = "MQTT_EXAMPLE";

#define topicPub "messages/90c8f03c-8cd1-4d59-8352-12a1e8d53ad8/topic" // ESP GUI DATA => WEB SUB TOPIC
#define topicSub "messages/90c8f03c-8cd1-4d59-8352-12a1e8d53ad8/topic/warning" // ESSP NHAN => WEB GUI DATA + messages/90c8f03c-8cd1-4d59-8352-12a1e8d53ad8/topic/warning
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, topicPub, "Hello Server!!!", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);


            msg_id = esp_mqtt_client_subscribe(client, topicSub, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            printf("MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
        	ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        	printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        	printf("DATA=%.*s\r\n", event->data_len, event->data);


        			//.. hien thi ra terminal warning

        			// printf("Messager MQTT: %.*s\r\n",event->data_len,event->data);

        				char *bufData = calloc(event->data_len + 1, sizeof(char)); //cap phat vung nho du chua data_len

        				snprintf(bufData, event->data_len + 1,  "%s", event->data);

        			//	printf("%s\n",bufData);



        				str_json = cJSON_Parse(bufData);

        				if(!str_json)
        				{
        					printf("Data JSON ERROR!!!\n");

        				}
        				else
        				{
        					//{"warning":"1"}
        					if(cJSON_GetObjectItem(str_json, "warningND"))
        					{
        						if(Start_led_ND == 0)
        						{
        							lastLed_ND = esp_timer_get_time()/1000; // bien thoi gian
        							Start_led_ND = 1; // bat dau dem so lan led chuyen trang thai
        							Dem_Start_led_ND = 0; // so lan chuyen trang thai
        						}

        					}
        					if(cJSON_GetObjectItem(str_json, "warningDA"))
        					{
        					    if(Start_led_DA == 0)
        					     {
        					        lastLed_DA = esp_timer_get_time()/1000;
        					        Start_led_DA = 1;
        					        Dem_Start_led_DA = 0;
        					     }

        					}

        					if(cJSON_GetObjectItem(str_json, "warningNDDA"))
        					{
        						//bat led nhiet va do do am
        					   if(Start_led_NDDA == 0)
        					   {
        					      lastLed_NDDA = esp_timer_get_time()/1000;
        					       Start_led_NDDA= 1;
        					       Dem_Start_led_NDDA = 0;
        					      // printf(">>>>Led Nhiet do Do am\r\n");
        					       	if(TT_LED_ND == 0)
        					       	{
        					       		TT_LED_ND = 1;
        					       		gpio_set_level(LED_ND, 1);
        					       		printf(">>>>LED ND ON\r\n");
        					       	}

        					       	if(TT_LED_DA == 0)
        					       	{
        					       			TT_LED_DA = 1;
        					       			gpio_set_level(LED_DA, 1);
        					       		printf(">>>>LED DA ON\r\n");
        					       	}
        					     }

        					   }

        				}

        				free(bufData);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
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
}

void StartLed(void *arg)
{
		gpio_pad_select_gpio(LED_ND);
	    gpio_set_direction(LED_ND, GPIO_MODE_OUTPUT);
		gpio_set_level(LED_ND, 0);
		TT_LED_ND = 0;

		gpio_pad_select_gpio(LED_DA);
		gpio_set_direction(LED_DA, GPIO_MODE_OUTPUT);
		gpio_set_level(LED_DA, 0);
		TT_LED_DA = 0;
		delay(200);

	while(1)
	{

		if(Start_led_ND == 1)
		{
			if(esp_timer_get_time()/1000 - lastLed_ND >= 500)
			{
				Dem_Start_led_ND++;
				if(Dem_Start_led_ND <= 10) // 2 lan : bat-> tat
				{
					// dao trang thai led
					printf(">>>>Led ND\r\n");
					if(TT_LED_ND == 0)
					{
						TT_LED_ND = 1;
						gpio_set_level(LED_ND, 1);
						printf(">>>>LED ND ON\r\n");

						//gpio_set_level(LED_ND, !gpio_get_level(LED_ND));
					}
					else if(TT_LED_ND == 1)
					{
						TT_LED_ND = 0;
						gpio_set_level(LED_ND, 0);
						printf(">>>>LED ND OFF\r\n");
					}

				}
				else
				{
					// khi nhieu hon 5 lan --> tat led
					Start_led_ND = 0;
					gpio_set_level(LED_ND, 0);

					TT_LED_ND = 0;
					Dem_Start_led_ND = 0;
				}
				lastLed_ND = esp_timer_get_time()/1000;
			}
		}

		delay(100);
		if(Start_led_DA == 1)
				{
					if(esp_timer_get_time()/1000 - lastLed_DA >= 500)
					{
						Dem_Start_led_DA++;
						if(Dem_Start_led_DA <= 20) //20 lan bat tat = 10 lan bat
						{
							// dao trang thai led

							if(TT_LED_DA == 0)
							{
								TT_LED_DA = 1;
								gpio_set_level(LED_DA, 1);
								printf(">>>>LED DA ON\r\n");
							}
							else if(TT_LED_DA == 1)
							{
								TT_LED_DA = 0;
								gpio_set_level(LED_DA, 0);
								printf(">>>>LED DA OFF\r\n");
							}
						}
						else
						{ // tat led
							Start_led_DA = 0;
							TT_LED_DA = 0;
							gpio_set_level(LED_DA, 0);
							Dem_Start_led_DA = 0;
						}
						lastLed_DA = esp_timer_get_time()/1000;

					}
				}

		delay(100);
		if(Start_led_NDDA == 1)
				{
					if(esp_timer_get_time()/1000 - lastLed_NDDA >= 5000)
					{



							Start_led_NDDA = 0;

							TT_LED_ND = 0;
							gpio_set_level(LED_ND, 0);
							printf(">>>>LED ND - DA OFF\r\n");
							TT_LED_DA = 0;
							gpio_set_level(LED_DA, 0);
							printf(">>>>LED DA - DA OFF\r\n");




						lastLed_NDDA = esp_timer_get_time()/1000;
					}
				}

	}
	vTaskDelete(NULL);
	delay(100);
}
void ConnectMQTT(void *arg)
{

		// mqtt://user:pass@ address server : port

		esp_mqtt_client_config_t mqtt_cfg = {

	        .uri = "mqtt://dqlam:tPOLldgihpFm42wy6DC5ni7m9dpPpFox@116.101.122.190:1883",
	    };

		esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
	    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
	    esp_mqtt_client_start(client);


		last1 = esp_timer_get_time()/1000;



		while(1)
		{

			if(MQTT_EVENT_CONNECTED)
			{

				if(btn_Start == 1)
				{
					if(btn == 1)
					{
						printf("Send nhiet do!!!\n");
						DataJsonNhietDo();
						esp_mqtt_client_publish(client, topicPub, JSON, 0, 1, 0);
					}
					if(btn == 2)
					{
						printf("Send do am!!!\n");
						DataJsonDoAm();
						esp_mqtt_client_publish(client, topicPub, JSON, 0, 1, 0);
					}
					 if(btn1 == 1)
					{

						printf("Send nhiet do + do am!!!\n");
						DataJsonNhietDoDoAm();
						esp_mqtt_client_publish(client, topicPub, JSON, 0, 1, 0);
						btn1 = 0;
					}
					btn_Start = 0;
				}

			}

			delay(50);
		}
		vTaskDelete(NULL);

}
void Button(void *arg)
{
	gpio_pad_select_gpio(NUT);
	gpio_set_direction(NUT, GPIO_MODE_INPUT);
	gpio_set_pull_mode(NUT, GPIO_PULLUP_ONLY);
	while(1)
	{

		Button_1();
		delay(200);

	}
	vTaskDelete(NULL);
	delay(500);

}
void Button_1(void)
{
	if(gpio_get_level(NUT) == 0)
	{
		delay(300);
		last1 = esp_timer_get_time()/1000;
		while(1)
		{

			if(gpio_get_level(NUT) == 1)
			{
				btn_Start = 1;
				if( esp_timer_get_time()/1000 - last1 >= 2000)
				{
					if(btn1 == 0)
					{
						btn1 = 1;
						btn = 0;
					}


				}
				else
				{
					if(btn == 0)
					{
						btn = 1;

					}
					else if(btn == 1)
					{
						btn = 2;

					}
					else if(btn == 2)
					{
						btn = 1;
					}
				}
				delay(300);
				break;
			}

		}
	}

}

void DataJsonNhietDo(void)
{
	for(int i = 0 ; i < 100; i++)
		{
			Str_ND[i] = 0;
			JSON[i] = 0;
		}

	sprintf(Str_ND, "%d", DHT11_read().temperature);


	strcat(JSON,"{\"ND\":\"");
	strcat(JSON,Str_ND);
	strcat(JSON,"\"}");
	printf("DataJson nhiet do: %s\n", JSON);


}
void DataJsonDoAm(void)
{
	for(int i = 0 ; i < 100; i++)
		{
			Str_DA[i] = 0;

			JSON[i] = 0;
		}
	sprintf(Str_DA, "%d", DHT11_read().humidity);


		strcat(JSON,"{\"DA\":\"");
		strcat(JSON,Str_DA);
		strcat(JSON,"\"}");
		printf("DataJson do am: %s\n", JSON);


}
void DataJsonNhietDoDoAm(void)
{
	for(int i = 0 ; i < 100; i++)
		{
			Str_ND[i] = 0;
			Str_DA[i] = 0;
			JSON[i] = 0;
		}
	sprintf(Str_ND, "%d", DHT11_read().temperature);
	sprintf(Str_DA, "%d", DHT11_read().humidity);

	strcat(JSON,"{\"ND\":\"");
	strcat(JSON,Str_ND);
	strcat(JSON,"\",");


	strcat(JSON,"\"DA\":\"");
	strcat(JSON,Str_DA);
	strcat(JSON,"\"}");

	printf("DataJson nhiet do do am: %s\n", JSON);

	delay(200);
}





void delay(uint32_t time)
{
	vTaskDelay(time / portTICK_PERIOD_MS);
}

void ConnectWiFi(void)
{
	s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,

			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG1, "wifi_init_sta finished.");


    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);


    if (bits & WIFI_CONNECTED_BIT)
	{
        ESP_LOGI(TAG1, "connected to ap SSID:%s password:%s",EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
	else if (bits & WIFI_FAIL_BIT)
	{
        ESP_LOGI(TAG1, "Failed to connect to SSID:%s, password:%s",EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
	else
	{
        ESP_LOGE(TAG1, "UNEXPECTED EVENT");
    }


    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
        esp_wifi_connect();
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
		{
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG1, "retry to connect to the AP");
        }
		else
		{
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG1,"connect to the AP fail");
    }
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG1, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    DHT11_init(GPIO_NUM_4);delay(100);

	ConnectWiFi();

	delay(500);

	 xTaskCreate(ConnectMQTT, "ConnectMQTT", 2048*2, NULL, 3, NULL);

	 xTaskCreate(Button, "Button", 2048*2, NULL, 3, NULL);

	 xTaskCreate(StartLed, "StartLed", 2048*2, NULL, 3, NULL);


	while(1)
	{
		delay(100);
	}

}




