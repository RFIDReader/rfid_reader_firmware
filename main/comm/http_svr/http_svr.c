//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_http_server.h>
#include <cJSON.h>
#include <esp_log.h>
#include <esp_netif.h>


#include "http_svr.h"
#include "types.h"
#include "comm_if.h"
#include "utils.h"
#include "tasks_common.h"

#define BUF_SIZE 2048


static const char TAG[] = "http_server";

static httpd_handle_t http_server_handle = NULL;
TaskHandle_t ws_task_handle = NULL;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
extern const uint8_t index_css_start[] asm("_binary_index_css_gz_start");
extern const uint8_t index_css_end[] asm("_binary_index_css_gz_end");
extern const uint8_t index_js_start[] asm("_binary_index_js_gz_start");
extern const uint8_t index_js_end[] asm("_binary_index_js_gz_end");

struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
    httpd_ws_frame_t ws_pkt;
};

static void http_svr_enable_cors(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "240");
}

static esp_err_t http_svr_allow_cors(httpd_req_t *req) {
    ESP_LOGI(TAG, "http_svr_allow_cors");
    http_svr_enable_cors(req);
    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t http_svr_resolve_ip_handler(httpd_req_t *req) {
    http_svr_enable_cors(req);

    esp_netif_ip_info_t ip_info;
    esp_netif_t *esp_netif = esp_netif_get_default_netif();
    esp_netif_get_ip_info(esp_netif, &ip_info);

    char redirect_url[16];
    sprintf(redirect_url, IPSTR, IP2STR(&ip_info.ip));

    httpd_resp_set_status(req, "301 Moved Permanently");
    httpd_resp_set_hdr(req, "Location", redirect_url);

    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t http_svr_index_handler(httpd_req_t *req) {
    return httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
}

static esp_err_t http_svr_favicon_handler(httpd_req_t *req) {
    return httpd_resp_send(req, (const char *) favicon_ico_start, favicon_ico_end - favicon_ico_start);
}

static esp_err_t http_svr_index_css_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *) index_css_start, index_css_end - index_css_start);
}

static esp_err_t http_svr_index_js_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *) index_js_start, index_js_end - index_js_start);
}

static esp_err_t http_svr_rest_handler(httpd_req_t *req) {
    http_svr_enable_cors(req);
    size_t total_body_len = req->content_len;
    size_t cur_body_len = 0;

    if (total_body_len >= BUF_SIZE) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too long");
        return ESP_FAIL;
    }

    char buf[BUF_SIZE] = {0};

    while (cur_body_len < total_body_len) {
        size_t rec_body_len = httpd_req_recv(req, buf + cur_body_len, total_body_len);
        if (rec_body_len <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_body_len += rec_body_len;
    }
    if (cur_body_len != total_body_len) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Incomplete data received");
        return ESP_FAIL;
    }
    buf[total_body_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL || !cJSON_IsArray(root)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid or non-array body");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    int body_len = cJSON_GetArraySize(root);
    if (body_len <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Empty array");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    uint8_t *body = (uint8_t *) malloc(body_len * sizeof(uint8_t));
    if (body == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failure");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    for (int i = 0; i < body_len; ++i) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (item != NULL && cJSON_IsNumber(item)) {
            body[i] = (uint8_t) item->valueint;
        } else {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unexpected type or missing element");
            free(body);
            cJSON_Delete(root);
            return ESP_FAIL;
        }
    }

    // process the body
    msg_t msg = {body, body_len};
    comm_if_post(&msg);

    msg_t response;
    if (xQueueReceive(http_msg_queue, &response, pdMS_TO_TICKS(33)) == pdPASS) {
        cJSON *resp, *element;
        resp = cJSON_CreateArray();

        for (uint16_t i = 0; i < response.len; ++i) {
            element = cJSON_CreateNumber(response.data[i]);
            cJSON_AddItemToArray(resp, element);
        }

        char *data = cJSON_Print(resp);
        ssize_t data_len = (ssize_t) strlen(data);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, data, data_len);

        cJSON_free(data);
        cJSON_Delete(resp);
    } else {
        ESP_LOGW(TAG, "No data received within 33ms.");
        httpd_resp_send_err(req, HTTPD_408_REQ_TIMEOUT, "request timeout");
    }

    free(body);
    cJSON_Delete(root);
    return ESP_OK;
}

static void delete_ws_task() {
    TaskHandle_t temp = ws_task_handle;
    ws_task_handle = NULL;
    vTaskDelete(temp);
}

static void ws_async_send(void *arg) {
    struct async_resp_arg *resp_arg = arg;
    esp_err_t ret = httpd_ws_send_frame_async(resp_arg->hd, resp_arg->fd, &resp_arg->ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame_async failed with %d", ret);
    }
}

static void ws_task(void *pvParameters) {
    struct async_resp_arg *resp_arg = pvParameters;
    for (;;) {
        msg_t response;

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        if (xQueueReceive(http_msg_queue, &response, pdMS_TO_TICKS(60 * 1000)) == pdPASS) {
            char *hex_str;
            size_t hex_str_len = 0;
            uint8_arr_to_hex_str(response.data, response.len, &hex_str, &hex_str_len);

            ws_pkt.payload = (uint8_t *) hex_str;
            ws_pkt.len = hex_str_len;
            resp_arg->ws_pkt = ws_pkt;

            httpd_queue_work(resp_arg->hd, ws_async_send, resp_arg);
        } else {
            ESP_LOGW(TAG, "No data received from comm_interface within 60 secs.");

            char *data = "TIMEOUT";
            size_t len = strlen(data);

            ws_pkt.payload = (uint8_t *) data;
            ws_pkt.len = len;
            resp_arg->ws_pkt = ws_pkt;

            httpd_queue_work(resp_arg->hd, ws_async_send, resp_arg);

            // Currently there is no way to determine if socket is disconnected
            // So, in case of time out occurs
            // We assume that it is due to:
            // - Disconnection of websocket from client side
            ESP_LOGW(TAG, "Deleting ws_task");
            delete_ws_task();
        }
    }
}

static esp_err_t http_svr_ws_handler(httpd_req_t *req) {
    http_svr_enable_cors(req);
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_rec_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_rec_frame failed with %d", ret);
            free(buf);
            return ret;
        }
    }

    if (ws_task_handle == NULL) {
        ESP_LOGI(TAG, "Creating ws_task");
        struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
        if (resp_arg == NULL) {
            return ESP_ERR_NO_MEM;
        }
        resp_arg->hd = req->handle;
        resp_arg->fd = httpd_req_to_sockfd(req);
        xTaskCreatePinnedToCore(ws_task,
                                "ws_tsk",
                                TEMP_TASK_STACK_SIZE,
                                resp_arg,
                                TEMP_TASK_PRIORITY,
                                &ws_task_handle,
                                TEMP_TASK_CORE_ID);
    }

    uint8_t *body;
    size_t body_len;
    hex_str_to_uint8_arr((char *) ws_pkt.payload, ws_pkt.len, &body, &body_len);

    msg_t msg = {body, body_len};
    comm_if_post(&msg);

    free(buf);
    return ret;
}

static httpd_handle_t http_server_configure(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.core_id = HTTP_SERVER_TASK_CORE_ID;

    config.task_priority = HTTP_SERVER_TASK_PRIORITY;

    config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

    config.max_uri_handlers = 15;

    config.recv_wait_timeout = 1;
    config.send_wait_timeout = 1;

    ESP_LOGI(TAG, "starting http server on port %d", config.server_port);

    if (httpd_start(&http_server_handle, &config) == ESP_OK) {
        ESP_LOGI(TAG, "registering uri handlers");

        httpd_uri_t resolve_ip = {
                .uri = "/resolve",
                .method = HTTP_GET,
                .handler = http_svr_resolve_ip_handler,
                .user_ctx = NULL,
        };

        httpd_uri_t _cors = {
                .uri = "/",
                .method = HTTP_OPTIONS,
                .handler = http_svr_allow_cors,
                .user_ctx = NULL,
        };

        httpd_uri_t _rest = {
                .uri = "/rest",
                .method = HTTP_OPTIONS,
                .handler = http_svr_allow_cors,
                .user_ctx = NULL,
        };

        httpd_uri_t rest = {
                .uri = "/rest",
                .method = HTTP_POST,
                .handler = http_svr_rest_handler,
                .user_ctx = NULL,
        };

        httpd_uri_t ws = {
                .uri = "/ws",
                .method = HTTP_GET,
                .handler = http_svr_ws_handler,
                .user_ctx = NULL,
                .is_websocket = true
        };

        httpd_uri_t index = {
                .uri = "/",
                .method = HTTP_GET,
                .handler = http_svr_index_handler,
                .user_ctx = NULL
        };

        /// [start] route has same handler as index router
        /// try replacing index route uri with pattern to avoid repeated (hardcoded) routes

        httpd_uri_t terminal = {
                .uri = "/terminal",
                .method = HTTP_GET,
                .handler = http_svr_index_handler,
                .user_ctx = NULL
        };

        httpd_uri_t settings = {
                .uri = "/settings",
                .method = HTTP_GET,
                .handler = http_svr_index_handler,
                .user_ctx = NULL
        };

        httpd_uri_t diagnostics = {
                .uri = "/diagnostics",
                .method = HTTP_GET,
                .handler = http_svr_index_handler,
                .user_ctx = NULL
        };

        /// [end] route has same handler as index router

        httpd_uri_t favicon = {
                .uri = "/favicon.ico",
                .method = HTTP_GET,
                .handler = http_svr_favicon_handler,
                .user_ctx = NULL
        };

        httpd_uri_t index_css = {
                .uri = "/index.css",
                .method = HTTP_GET,
                .handler = http_svr_index_css_handler,
                .user_ctx = NULL
        };

        httpd_uri_t index_js = {
                .uri = "/index.js",
                .method = HTTP_GET,
                .handler = http_svr_index_js_handler,
                .user_ctx = NULL
        };

        httpd_register_uri_handler(http_server_handle, &resolve_ip);
        httpd_register_uri_handler(http_server_handle, &_cors);
        httpd_register_uri_handler(http_server_handle, &_rest);
        httpd_register_uri_handler(http_server_handle, &rest);
        httpd_register_uri_handler(http_server_handle, &ws);
        httpd_register_uri_handler(http_server_handle, &index);
        httpd_register_uri_handler(http_server_handle, &terminal);
        httpd_register_uri_handler(http_server_handle, &settings);
        httpd_register_uri_handler(http_server_handle, &diagnostics);
        httpd_register_uri_handler(http_server_handle, &favicon);
        httpd_register_uri_handler(http_server_handle, &index_css);
        httpd_register_uri_handler(http_server_handle, &index_js);

        return http_server_handle;
    }
    return NULL;
}

void http_svr_start(void) {
    if (http_server_handle == NULL) {
        http_server_handle = http_server_configure();
    }
}

void http_svr_stop(void) {
    if (http_server_handle) {
        httpd_stop(http_server_handle);
        ESP_LOGI(TAG, "http server stopped");
        http_server_handle = NULL;
    }
}