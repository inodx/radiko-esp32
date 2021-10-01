#include "esp_http_client.h"
#include "radiko.h"
#include "mbedtls/base64.h"
#include "expat.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_tls.h"

static const char *TAG = "RADIKO";
extern const char radiko_jp_root_cert_pem_start[] asm("_binary_radiko_jp_root_cert_pem_start");
extern const char radiko_jp_root_cert_pem_end[]   asm("_binary_radiko_jp_root_cert_pem_end");

char * content;
int _length;
int _offset;

typedef enum 
{
  DOCUMENT,
  STATIONS,
  STATION, 
  ID, 
  NAME, 
  ASCII_NAME, 
  HREF,
  LOGO_XSMALL,
  LOGO_SMALL,
  LOGO_MEDIUM,
  LOGO_LARGE,
  LOGO,
  FEED,
  BANNER
} element;

element current = DOCUMENT;


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
            else if(strstr((char *)evt->header_key, "X-Radiko-Authtoken") == (char *)evt->header_key)
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
                *(content + output_len) = '\0';
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


static void XMLCALL
elementStart(void *user_data, const XML_Char *el, const XML_Char *attr[]) 
{
  //printf("element: [%s]\n", el);
  switch (current) {
    case DOCUMENT:
      if (strcmp(el,"stations") == 0)
        current = STATIONS;
      break;
    case STATIONS:
      if (strcmp(el,"station") == 0)
        current = STATION;
      break;
    case STATION:
      if (strcmp(el,"id") == 0)
        current = ID;
      else if (strcmp(el,"name") == 0)
        current = NAME;
      else if (strcmp(el,"ascii_name") == 0)
        current = ASCII_NAME;
      else if (strcmp(el,"href") == 0)
        current = HREF;
      else if (strcmp(el,"logo_xsmall") == 0)
        current = LOGO_XSMALL;
      else if (strcmp(el,"logo_small") == 0)
        current = LOGO_SMALL;
      else if (strcmp(el,"logo_medium") == 0)
        current = LOGO_MEDIUM;
      else if (strcmp(el,"logo_large") == 0)
        current = LOGO_LARGE;
      else if (strcmp(el,"logo") == 0)
        current = LOGO;
      else if (strcmp(el,"feed") == 0)
        current = FEED;
      else if (strcmp(el,"banner") == 0)
        current = BANNER;
      break;
    case ID:
    case NAME:
    case ASCII_NAME:
    case HREF:
      break;
    default:
      current = DOCUMENT;
      break;
  }
}

static void XMLCALL
elementEnd(void *user_data, const XML_Char *el) 
{
  switch (current) {
    case DOCUMENT:
      break;
    case STATIONS:
      current = DOCUMENT;
      break;
    case STATION:
      current = STATIONS;
      printf("\n");
      break;
    case ID:
      current = STATION;
      break;
    case NAME:
      current = STATION;
      break;
    case ASCII_NAME:
      current = STATION;
      break;
    case HREF:
      current = STATION;
      break;
    case LOGO_XSMALL:
      current = STATION;
      break;
    case LOGO_SMALL:
      current = STATION;
      break;
    case LOGO_MEDIUM:
      current = STATION;
      break;
    case LOGO_LARGE:
      current = STATION;
      break;
    case LOGO:
      current = STATION;
      break;
    case FEED:
      current = STATION;
      break;
    case BANNER:
      current = STATION;
      break;
    default:
      current = STATIONS;
      break;
  }
}

static void XMLCALL elementData(void *user_data, const XML_Char *data, int data_size) 
{
    bool use_el = true;
    char type_str[16];
    switch (current) 
    {
        case ID:
            strlcpy(&(stations -> id), data, data_size + 1);
            strcpy(type_str, "ID");
            break;
        case NAME:
            strlcpy(&(stations -> name), data, data_size + 1);
            strcpy(type_str, "NAME");
            break;
        case ASCII_NAME:
            strlcpy(&(stations -> ascii_name), data, data_size + 1);
            strcpy(type_str, "ASCII_NAME");
            break;
        case LOGO_XSMALL:
            strlcpy(&(stations -> logo_xsmall), data, data_size + 1);
            strcpy(type_str, "LOGO_XSMALL");
            stations ++;
            station_count ++;
            break;
        default:
            use_el = false;
            break;
    }
    if(use_el)
    {
        char tmp_str[data_size + 1];
        strlcpy(tmp_str, data, data_size + 1);
        ESP_LOGI("ELEMDATA", "[%s]: %s", type_str, tmp_str);
    }
}

void parse_xml(char * input)
{
    stations = malloc(sizeof(station_t) * 32);
    if(stations != NULL)
    {
        station_t * st_p = stations;
        XML_Parser parser = XML_ParserCreate(NULL);
        XML_SetElementHandler(parser, elementStart, elementEnd);
        XML_SetCharacterDataHandler(parser, elementData);

        if (XML_Parse(parser, input, strlen(input), 1) == XML_STATUS_ERROR) 
        {
            ESP_LOGE("PARSE XML", "Parsing error");
        }    
        stations = st_p;
        for(int i = 0; i < station_count; i ++)
        {
            ESP_LOGI("RESULTS", "[%d]: %s", i, (st_p -> id));
            st_p ++;
        }
        XML_ParserFree(parser);
    }
}

void get_station_list()
{
    const char * TAG = "GET ST LIST";
    esp_http_client_config_t config = 
    {
        .url = "",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = _http_event_handler_2,
        .cert_pem = radiko_jp_root_cert_pem_start
    };
    char tmp_url[128];
    sprintf(tmp_url, "https://radiko.jp/v2/station/list/%s.xml", _region_id);
    config.url = tmp_url;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    ESP_LOGI(TAG, "Allocating xml buffer");
    content = malloc(38400 * sizeof(char));
    ESP_LOGI(TAG, "Starting https request");
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        int len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
            status_code, len);
        ESP_LOGD(TAG, "XML buffer string length = %d", strlen(content));
        ESP_LOGD(TAG, "Received body: %s", content);
        parse_xml(content);
    }
    else 
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    if(content != NULL)
    {
        ESP_LOGI(TAG, "Removing buffer");
        free(content);
        content = NULL;
    }
    esp_http_client_cleanup(client);    
}

void generate_playlist_url(station_t * st)
{
    esp_http_client_config_t config = 
    {
        .url = "",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = _http_event_handler,
        .cert_pem = radiko_jp_root_cert_pem_start
    };
    config.transport_type = HTTP_TRANSPORT_OVER_TCP;
    config.event_handler = _http_event_handler_2;
    char tmp_url[256];
    sprintf(tmp_url, "http://f-radiko.smartstream.ne.jp/%s/_definst_/simul-stream.stream/playlist.m3u8", (st -> id));
    ESP_LOGI(TAG, "TMP_URL: %s", tmp_url);
    config.url = tmp_url;
    ESP_LOGW(TAG, "URL: %s", config.url);
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "X-Radiko-AuthToken", _auth_token);
    ESP_LOGI(TAG, "Starting get playlist");

    content = malloc(8192 * sizeof(char));
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
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

void auth(void)
{
    station_count = 0;
    _length = 0;
    _offset = 0;
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
        const char * auth_key = CONFIG_AUTH_KEY;
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
        config.event_handler = _http_event_handler_2;
        config.url = "https://radiko.jp/v2/api/auth2";
        client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "X-Radiko-AuthToken", _auth_token);
        esp_http_client_set_header(client, "X-Radiko-Partialkey", base64_encoded);
        esp_http_client_set_header(client, "X-Radiko-User", "dummy_user");
        esp_http_client_set_header(client, "X-Radiko-Device", "pc");
        ESP_LOGI(TAG, "Starting auth2");
        content = malloc(128 * sizeof(char));
        esp_err_t err = esp_http_client_perform(client);
        int status_code = esp_http_client_get_status_code(client);
        if (err == ESP_OK) 
        {
            ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                    status_code, esp_http_client_get_content_length(client));
            char * endptr = strstr(content, ",");
            *endptr = '\0';
            strcpy(_region_id, content);
            ESP_LOGI(TAG, "Region: %s", content);
            free(content);
        }
        else 
        {
            ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
        esp_http_client_cleanup(client);
    }
}
