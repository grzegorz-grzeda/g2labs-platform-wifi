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
#include <stddef.h>

static platform_wifi_state_handler_t on_connect_handler = NULL;
static platform_wifi_state_handler_t on_disconnect_handler = NULL;

static void handle_situation(platform_wifi_state_handler_t handler, void* data) {
    if (handler) {
        handler(data);
    }
}

static void handle_on_connected(void) {
    handle_situation(on_connect_handler, NULL);
}

static void handle_on_disconnected(void) {
    handle_situation(on_disconnect_handler, NULL);
}

void platform_wifi_set_on_connected_handler(platform_wifi_state_handler_t on_connected) {
    on_connect_handler = on_connected;
}

void platform_wifi_set_on_disconnected_handler(platform_wifi_state_handler_t on_disconnected) {
    on_disconnect_handler = on_disconnected;
}

void platform_wifi_set(const char* ssid, const char* password) {}

void platform_wifi_connect(void) {
    handle_on_connected();
}

void platform_wifi_disconnect(void) {
    handle_on_disconnected();
}
