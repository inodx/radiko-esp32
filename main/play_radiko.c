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
#include "esp_http_client.h"
#include "radiko.h"
#include "esp_peripherals.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "periph_button.h"
#include "board.h"
#include "periph_wifi.h"
#include "board.h"
#include "audio_alc.h"
#include "input_key_service.h"
#include "pwm_stream.h"
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

//#define AAC_STREAM_URI "http://open.ls.qingting.fm/live/274/64k.m3u8?format=aac"
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_content 8192

audio_pipeline_handle_t pipeline;
audio_element_handle_t http_stream_reader, i2s_stream_writer, aac_decoder;


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


audio_element_handle_t alc_el;

int player_volume=-10;

#include "driver/gpio.h"
#include "ws2812_control.h"
#define BLINK_GPIO 10

struct led_state new_state;
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
	//for M5 StickC
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 0);

    ws2812_control_init();
    new_state.leds[0] = 0x005000;
	ws2812_write_leds(new_state);
    

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "[ 1 ] Start audio codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
//    audio_hal_set_volume(board_handle->audio_hal, 50);

    int current_station = 5; 
//    audio_hal_get_volume(board_handle->audio_hal, &player_volume);

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
#ifdef CONFIG_PLAY_OUTPUT_PWM
    pwm_stream_cfg_t pwm_cfg = PWM_STREAM_CFG_DEFAULT();
    pwm_cfg.pwm_config.gpio_num_left = CONFIG_PWM_LEFT_OUTPUT_GPIO_NUM;
    pwm_cfg.pwm_config.gpio_num_right = CONFIG_PWM_RIGHT_OUTPUT_GPIO_NUM;
    i2s_stream_writer = pwm_stream_init(&pwm_cfg);
#else
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();//I2S_STREAM_CFG_DEFAULT();//I2S_STREAM_INTERNAL_DAC_CFG_DEFAULT
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
#endif

    ESP_LOGI(TAG, "[2.3] Create aac decoder to decode aac file");
    aac_decoder_cfg_t aac_cfg = DEFAULT_AAC_DECODER_CONFIG();
    aac_decoder = aac_decoder_init(&aac_cfg);

    ESP_LOGI(TAG, "[2.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    audio_pipeline_register(pipeline, aac_decoder,        "aac");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    alc_volume_setup_cfg_t alc_cfg = DEFAULT_ALC_VOLUME_SETUP_CONFIG();
    alc_el = alc_volume_setup_init(&alc_cfg);
    audio_pipeline_register(pipeline, alc_el, "alc");
    ESP_LOGI(TAG, "[2.5] Link it together http_stream-->aac_decoder-->ALC-->i2s_stream-->[codec_chip]");
    const char *link_tag[4] = {"http", "aac", "alc", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 4);

//    ESP_LOGI(TAG, "[2.5] Link it together http_stream-->aac_decoder-->i2s_stream-->[codec_chip]");
//    const char *link_tag[3] = {"http", "aac", "i2s"};
//    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[ 3 ] Create and start input key service");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[ 3 ] Start and wait for Wi-Fi network");

    periph_wifi_cfg_t wifi_cfg = 
    {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    esp_periph_start(set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    new_state.leds[0] = 0x0050;//blue
	ws2812_write_leds(new_state);
    auth();
    get_station_list();
    generate_playlist_url(&stations[current_station]);
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
        if (ret != ESP_OK) 
        {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if ((msg.source_type == PERIPH_ID_BUTTON )
            && (msg.cmd == PERIPH_BUTTON_RELEASE || msg.cmd == PERIPH_BUTTON_LONG_RELEASE)) {

            if ((int) msg.data == get_input_play_id()) 
            {
                ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
                audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
                switch (el_state) 
                {
                    case AEL_STATE_INIT :
                        ESP_LOGI(TAG, "[ * ] Starting audio pipeline");
                        audio_pipeline_run(pipeline);
                        break;
                    case AEL_STATE_RUNNING :
                        ESP_LOGI(TAG, "[ * ] Pausing audio pipeline");
                        audio_pipeline_pause(pipeline);
                        break;
                    case AEL_STATE_PAUSED :
                        ESP_LOGI(TAG, "[ * ] Resuming audio pipeline");
                        audio_pipeline_resume(pipeline);
                        break;
                    case AEL_STATE_FINISHED :
                        ESP_LOGI(TAG, "[ * ] Rewinding audio pipeline");
                        audio_pipeline_reset_ringbuffer(pipeline);
                        audio_pipeline_reset_elements(pipeline);
                        audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
                        audio_pipeline_run(pipeline);
                        break;
                    default :
                        ESP_LOGI(TAG, "[ * ] Not supported state %d", el_state);
                }
            } 
            else if ((int) msg.data == get_input_set_id()) 
            {
            	if (msg.cmd == PERIPH_BUTTON_LONG_RELEASE) {
           			player_volume-=10;
           			if (player_volume<=-50) player_volume=0;
                	alc_volume_setup_set_volume(alc_el, player_volume);
            		ESP_LOGI(TAG, "[ set ] PERIPH_BUTTON_LONG_RELEASE VOLUME DOWN");
            		continue;
            		}
            	
            	ESP_LOGI(TAG, "[ set ] PERIPH_BUTTON_RELEASE CHANGE STATION");
                ESP_LOGI(TAG, "[ * ] Stopping audio pipeline");

                audio_pipeline_stop(pipeline);
                audio_pipeline_wait_for_stop(pipeline);

                current_station ++;
                if(current_station < station_count)
                {
                    generate_playlist_url(stations + current_station);
                }
                else
                {
                    generate_playlist_url(stations);
                    current_station = 0;
                }
                ESP_LOGI(TAG, "Setting playlist for the Station : %s", ((stations + current_station) -> name));
                audio_element_set_uri(http_stream_reader, _playlist_url);

                ESP_LOGW(TAG, "[ * ] Restart stream");
                audio_element_reset_state(aac_decoder);
                audio_element_reset_state(i2s_stream_writer);
                audio_element_reset_state(alc_el);
                audio_pipeline_reset_ringbuffer(pipeline);
                audio_pipeline_reset_items_state(pipeline);
                audio_pipeline_run(pipeline);
            } 

        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) aac_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) 
        {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(aac_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from aac decoder, sample_rates=%d, bits=%d, ch=%d",
                    music_info.sample_rates, music_info.bits, music_info.channels);


            audio_element_setinfo(i2s_stream_writer, &music_info);
          	new_state.leds[0] = 0x500000;//green
    		ws2812_write_leds(new_state);
            alc_volume_setup_set_channel(alc_el, music_info.channels);
            alc_volume_setup_set_volume(alc_el, player_volume);

#ifdef CONFIG_PLAY_OUTPUT_PWM
			pwm_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
#else
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
#endif
            continue;
        }

        /* restart stream when the first pipeline element (http_stream_reader in this case) receives stop event (caused by reading errors) */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) http_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int) msg.data == AEL_STATUS_ERROR_OPEN) 
        {
            ESP_LOGW(TAG, "[ * ] Restart stream");
            audio_pipeline_stop(pipeline);
            audio_pipeline_wait_for_stop(pipeline);
            audio_element_reset_state(alc_el);
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

	audio_pipeline_unregister(pipeline, alc_el);
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
    audio_element_deinit(alc_el);
    audio_element_deinit(http_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(aac_decoder);
    esp_periph_set_destroy(set);
}

