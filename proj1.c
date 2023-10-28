// Usage of "send" vs "httpd_resp_send" commands via WiFi connection 
// by using HTTP 0.9 or HTTP 1.1 protocols

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

static esp_err_t post_handler(httpd_req_t *HTTP_request) 
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    httpd_resp_send(HTTP_request, "POST Response from ESP32 - HTTP Server", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t post_handler_HTTP09(httpd_req_t *HTTP_request)
{
    int s; // Socket fd
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    s = httpd_req_to_sockfd(HTTP_request);
    char *data_to_send = "Response from ESP32 HTTP 0.9";
    int data_len = strlen(data_to_send);
    send(s, data_to_send, data_len, 0);

    return ESP_OK;
}

static esp_err_t post_handler_HTTP11(httpd_req_t *HTTP_request)
{
    int s; // Socket fd
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    s = httpd_req_to_sockfd(HTTP_request);
    char *data_to_send = "HTTP/1.1 200 OK\r\nContent-Length: 55\r\n\r\nResponse from ESP32 HTTP 1.1";
    int data_len = strlen(data_to_send);
    send(s, data_to_send, data_len, 0);

    return ESP_OK;
}

void server_initiation()
{
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;
    httpd_start(&server_handle, &server_config);
    httpd_uri_t uri_post = {
        .uri = "/",
        .method = HTTP_POST,
        .handler = post_handler,
        // .handler = post_handler_HTTP09,
        // .handler = post_handler_HTTP11,
        .user_ctx = NULL};
    httpd_register_uri_handler(server_handle, &uri_post);
}

void app_main(void)
{
    wifi_connection();
    server_initiation();
}