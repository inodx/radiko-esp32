/* Play M3U HTTP Living stream

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "aac_decoder.h"
#include "esp_tls.h"
#include <mbedtls/base64.h>
#include "esp_http_client.h"

#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"

#if __has_include("esp_idf_version.h")
#include "esp_idf_version.h"
#else
#define ESP_IDF_VERSION_VAL(major, minor, patch) 1
#endif

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
#include "esp_netif.h"
#else
#include "tcpip_adapter.h"
#endif

static const char *TAG = "HTTP_LIVINGSTREAM_EXAMPLE";

extern const char radiko_jp_root_cert_pem_start[] asm("_binary_radiko_jp_root_cert_pem_start");
extern const char radiko_jp_root_cert_pem_end[]   asm("_binary_radiko_jp_root_cert_pem_end");


//#define AAC_STREAM_URI "http://open.ls.qingting.fm/live/274/64k.m3u8?format=aac"
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_content 8192

char _auth_token[128];
char _playlist_url[128];
char _audio_url[128];

int _length = 0;
int _offset = 0;
char * content;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    const char * TAG = "EVT_HTTP";
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) 
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            if(strstr((char *)evt->header_key, "X-Radiko-AuthToken") == (char *)evt->header_key)
            {
                ESP_LOGI(TAG, "GOT TOKEN");
                strcpy(_auth_token, evt->header_value);
            }
            else if(strstr((char *)evt->header_key, "X-RADIKO-AUTHTOKEN") == (char *)evt->header_key)
            {
                ESP_LOGI(TAG, "GOT TOKEN");
                strcpy(_auth_token, evt->header_value);
            }

            else if(strstr((char *)evt->header_key, "X-Radiko-KeyOffset") == (char *)evt->header_key)
            {
                ESP_LOGI(TAG, "GOT OFFSET");
                char offset_c[8];
                strcpy(offset_c, evt->header_value);
                _offset = atoi(offset_c);
            }
            else if(strstr((char *)evt->header_key, "X-Radiko-KeyLength") == (char *)evt->header_key)
            {
                ESP_LOGI(TAG, "GOT LENGTH");
                char length_c[8];
                strcpy(length_c, evt->header_value);
                _length = atoi(length_c);
            }
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
	    char * endptr = evt->data + evt->data_len;
	    *endptr = '\0';
	    ESP_LOGI(TAG, "Received data: %s", (char *)evt->data);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) 
            {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) 
                {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } 
                else 
                {
                    if (output_buffer == NULL) 
                    {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) 
                        {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) 
            {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                if(content != NULL)
                {
                    memcpy(content, output_buffer, output_len);
                }
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) 
            {
                ESP_LOGW(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGW(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            else
            {
                if (output_buffer != NULL)
                {
                    if(content != NULL)
                    {
                        memcpy(content, output_buffer, output_len);
                    }
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
            }
            break;
    }
    return ESP_OK;
}

esp_err_t _http_event_handler_2(esp_http_client_event_t *evt)
{
    const char * TAG = "EVT_HTTP";
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) 
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            //ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                memcpy(content + output_len, evt->data, evt->data_len);
                output_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) 
            {
                ESP_LOGW(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGW(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            else
            {
                output_len = 0;
            }
            break;
    }
    return ESP_OK;
}

int _http_stream_event_handle(http_stream_event_msg_t *msg)
{
    if (msg->event_id == HTTP_STREAM_RESOLVE_ALL_TRACKS) {
        return ESP_OK;
    }

    if (msg->event_id == HTTP_STREAM_FINISH_TRACK) {
        return http_stream_next_track(msg->el);
    }
    if (msg->event_id == HTTP_STREAM_FINISH_PLAYLIST) {
        return http_stream_fetch_again(msg->el);
    }
    return ESP_OK;
}

static void get_playlist_url(void)
{
    _auth_token[0] = '\0';
    char token_p[32];
    token_p[0] = '\0';
    const char * TAG = "GET";
    esp_http_client_config_t config = 
    {
        .url = "https://radiko.jp/v2/api/auth1",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = _http_event_handler,
        .cert_pem = radiko_jp_root_cert_pem_start
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_http_client_set_header(client, "User-Agent", "esp32/4");
    esp_http_client_set_header(client, "Accept", "*/*");
    esp_http_client_set_header(client, "X-Radiko-App", "pc_html5");
    esp_http_client_set_header(client, "X-Radiko-App-Version", "0.0.1");
    esp_http_client_set_header(client, "X-Radiko-User", "dummy_user");
    esp_http_client_set_header(client, "X-Radiko-Device", "pc");
    ESP_LOGI(TAG, "Starting auth1");
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                status_code, esp_http_client_get_content_length(client));
        ESP_LOGI(TAG, "Token: %s", _auth_token);        
        const char * auth_key = "bcd151073c03b352e1ef2fd66c32209da9ca0afa";
        strncpy(token_p, (auth_key + _offset), _length);
        token_p[_length] = '\0';
        ESP_LOGI(TAG, "Partial Key: %s", token_p);
    }
    else 
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    if(strlen(token_p) != 0)
    {
        char base64_encoded[32];
        size_t base64_len;
        mbedtls_base64_encode((unsigned char *) base64_encoded, sizeof(base64_encoded),
                                &base64_len, (unsigned char *) token_p, strlen(token_p));
        if(strlen(base64_encoded) > 0)
        {
            ESP_LOGI(TAG, "Base64 encoded: %s", base64_encoded);
        }
        else
        {
            ESP_LOGE(TAG, "Base64 encode failed");
        }
        config.event_handler = _http_event_handler;
        config.url = "https://radiko.jp/v2/api/auth2";
        client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "X-Radiko-AuthToken", _auth_token);
        esp_http_client_set_header(client, "X-Radiko-Partialkey", base64_encoded);
        esp_http_client_set_header(client, "X-Radiko-User", "dummy_user");
        esp_http_client_set_header(client, "X-Radiko-Device", "pc");
        ESP_LOGI(TAG, "Starting auth2");
        esp_err_t err = esp_http_client_perform(client);
        int status_code = esp_http_client_get_status_code(client);
        if (err == ESP_OK) 
        {
            ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                    status_code, esp_http_client_get_content_length(client));
        }
        else 
        {
            ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
        esp_http_client_cleanup(client);

        config.transport_type = HTTP_TRANSPORT_OVER_TCP;
        config.event_handler = _http_event_handler_2;
        config.url = "http://f-radiko.smartstream.ne.jp/TBS/_definst_/simul-stream.stream/playlist.m3u8";
        client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "X-Radiko-AuthToken", _auth_token);
        ESP_LOGI(TAG, "Starting get playlist");

        content = malloc(8192 * sizeof(char));
        err = esp_http_client_perform(client);
        status_code = esp_http_client_get_status_code(client);
        if (err == ESP_OK) 
        {
            int len = esp_http_client_get_content_length(client);
            ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                status_code, len);
            ESP_LOGI(TAG, "Received body: %s", content);
            char * url_start = strstr(content, "http://");
            char * url_end = strstr(content, ".m3u8") + 5;
            int url_len = (url_end - url_start);
            strncpy(_playlist_url, url_start, url_len);

            *(_playlist_url + url_len) = '\0';
            ESP_LOGI(TAG, "Stream url: %s", _playlist_url);
        }
        else 
        {
            ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
        if(content != NULL)
        {
            free(content);
            content = NULL;
        }
        esp_http_client_cleanup(client);
    }
}


void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
#else
    tcpip_adapter_init();
#endif

    audio_pipeline_handle_t pipeline;
    audio_element_handle_t http_stream_reader, i2s_stream_writer, aac_decoder;

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "[ 1 ] Start audio codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[2.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(TAG, "[2.1] Create http stream to read data");
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.event_handle = _http_stream_event_handle;
    http_cfg.type = AUDIO_STREAM_READER;
    http_cfg.enable_playlist_parser = true;
    http_stream_reader = http_stream_init(&http_cfg);

    ESP_LOGI(TAG, "[2.2] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[2.3] Create aac decoder to decode aac file");
    aac_decoder_cfg_t aac_cfg = DEFAULT_AAC_DECODER_CONFIG();
    aac_decoder = aac_decoder_init(&aac_cfg);

    ESP_LOGI(TAG, "[2.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    audio_pipeline_register(pipeline, aac_decoder,        "aac");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    ESP_LOGI(TAG, "[2.5] Link it together http_stream-->aac_decoder-->i2s_stream-->[codec_chip]");
    const char *link_tag[3] = {"http", "aac", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);


    ESP_LOGI(TAG, "[ 3 ] Start and wait for Wi-Fi network");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    periph_wifi_cfg_t wifi_cfg = 
    {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    esp_periph_start(set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);
    get_playlist_url();
    ESP_LOGI(TAG, "[2.6] Set up  uri (http as http_stream, aac as aac decoder, and default output is i2s)");
    audio_element_set_uri(http_stream_reader, _playlist_url);

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    while (1) 
    {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) aac_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(aac_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from aac decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }

        /* restart stream when the first pipeline element (http_stream_reader in this case) receives stop event (caused by reading errors) */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) http_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int) msg.data == AEL_STATUS_ERROR_OPEN) {
            ESP_LOGW(TAG, "[ * ] Restart stream");
            audio_pipeline_stop(pipeline);
            audio_pipeline_wait_for_stop(pipeline);
            audio_element_reset_state(aac_decoder);
            audio_element_reset_state(i2s_stream_writer);
            audio_pipeline_reset_ringbuffer(pipeline);
            audio_pipeline_reset_items_state(pipeline);
            audio_pipeline_run(pipeline);
            continue;
        }
    }

    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, http_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, aac_decoder);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(http_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(aac_decoder);
    esp_periph_set_destroy(set);
}