// UDP SOCKET Server with WiFi connection communication via Socket
// Send data from the ESP32 server and receive on the PC by using a Python program.


#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#include "nvs_flash.h"
#include "ping/ping_sock.h"
#include "driver/gpio.h"
#include "my_data.h"

#define PORT 3333
static const char *TAG = "UDP SOCKET SERVER";

static void udp_sender_task(void *pvParameters)
{
    const char *PC_IP = "192.168.1.2";   // PC IP
    const int PC_PORT = 3333;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(PC_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PC_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UDP socket created, sending to %s:%d", PC_IP, PC_PORT);

    int counter = 0;
    while (1) {
        char payload[64];
        snprintf(payload, sizeof(payload),
                 "Hello from ESP32, count=%d", counter++);

        int err = sendto(sock, payload, strlen(payload), 0,
                         (struct sockaddr *)&dest_addr,
                         sizeof(dest_addr));

        if (err < 0) {
            ESP_LOGE(TAG, "Error sending: errno %d", errno);
        } else {
            ESP_LOGI(TAG, "Message sent: %s", payload);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second
    }

    close(sock);
    vTaskDelete(NULL);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();
}

void app_main(void)
{
    wifi_connection();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(udp_sender_task, "udp_sender", 4096, (void *)AF_INET, 5, NULL);
}

