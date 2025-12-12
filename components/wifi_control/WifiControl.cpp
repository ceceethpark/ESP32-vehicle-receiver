#include "WifiControl.h"

WifiControl::WifiControl()
    : initialized_(false)
    , started_(false)
    , mode_(WIFI_MODE_STA)
{
}

WifiControl::~WifiControl()
{
    end();
}

esp_err_t WifiControl::begin(wifi_mode_t mode)
{
    if (initialized_) {
        ESP_LOGW(TAG, "WiFi already initialized");
        return ESP_OK;
    }
    
    mode_ = mode;
    
    // Network interface 초기화
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 이벤트 루프 생성
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // WiFi 초기화
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // WiFi 모드 설정
    ret = esp_wifi_set_mode(mode_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        esp_wifi_deinit();
        return ret;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "WiFi initialized (mode: %d)", mode_);
    
    // WiFi 시작
    ret = start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi start failed: %s", esp_err_to_name(ret));
        initialized_ = false;
        esp_wifi_deinit();
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t WifiControl::initialize(wifi_mode_t mode)
{
    return begin(mode);
}

void WifiControl::end()
{
    if (started_) {
        stop();
    }
    
    if (initialized_) {
        esp_wifi_deinit();
        initialized_ = false;
        ESP_LOGI(TAG, "WiFi deinitialized");
    }
}

esp_err_t WifiControl::setMode(wifi_mode_t mode)
{
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = esp_wifi_set_mode(mode);
    if (ret == ESP_OK) {
        mode_ = mode;
        ESP_LOGI(TAG, "WiFi mode set to %d", mode);
    } else {
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

wifi_mode_t WifiControl::getMode() const
{
    return mode_;
}

esp_err_t WifiControl::start()
{
    if (!initialized_) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (started_) {
        ESP_LOGW(TAG, "WiFi already started");
        return ESP_OK;
    }
    
    esp_err_t ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    started_ = true;
    ESP_LOGI(TAG, "WiFi started");
    return ESP_OK;
}

esp_err_t WifiControl::stop()
{
    if (!started_) {
        ESP_LOGW(TAG, "WiFi not started");
        return ESP_OK;
    }
    
    esp_err_t ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    started_ = false;
    ESP_LOGI(TAG, "WiFi stopped");
    return ESP_OK;
}
