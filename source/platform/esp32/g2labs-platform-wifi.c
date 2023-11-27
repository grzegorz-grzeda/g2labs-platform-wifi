/*
 * MIT License
 *
 * Copyright (c) 2023 G2Labs Grzegorz Grzęda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "g2labs-platform-wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include <stdbool.h>
#include <string.h>

static bool first_time = true;
static volatile bool retry = true;
static char local_ssid[32];
static char local_password[32];

static platform_wifi_state_handler_t on_connect_handler = NULL;
static platform_wifi_state_handler_t on_disconnect_handler = NULL;

static void handle_situation(platform_wifi_state_handler_t handler, bool is_connected) {
    if (handler) {
        handler(is_connected);
    }
}

static void handle_on_connected(void) {
    handle_situation(on_connect_handler, true);
}

static void handle_on_disconnected(void) {
    handle_situation(on_disconnect_handler, false);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("wifi: station started");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        handle_on_disconnected();
        printf("wifi: disconnected");
        if (retry) {
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        printf("wifi: connected, IP=" IPSTR, IP2STR(&event->ip_info.ip));
        handle_on_connected();
    }
}

void platform_wifi_set_on_connected_handler(platform_wifi_state_handler_t on_connected) {
    on_connect_handler = on_connected;
}

void platform_wifi_set_on_disconnected_handler(platform_wifi_state_handler_t on_disconnected) {
    on_disconnect_handler = on_disconnected;
}

void platform_wifi_set(const char* ssid, const char* password) {
    strcpy(local_ssid, ssid);
    strcpy(local_password, password);
}

void platform_wifi_connect(void) {
    retry = true;
    wifi_config_t wifi_config = {
        .sta =
            {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                .sae_h2e_identifier = "",
            },
    };
    strcpy(wifi_config.sta.ssid, local_ssid);
    strcpy(wifi_config.sta.password, local_password);
    if (first_time) {
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        first_time = false;
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
}

void platform_wifi_disconnect(void) {
    retry = false;
    esp_wifi_disconnect();
}
