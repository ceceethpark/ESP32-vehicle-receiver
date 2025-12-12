#include "RosBridge.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// 정적 멤버 초기화
RosBridge* RosBridge::instance_ = nullptr;

RosBridge::RosBridge(uart_port_t uart_port, int tx_pin, int rx_pin, int baud_rate)
    : uart_port_(uart_port)
    , tx_pin_(tx_pin)
    , rx_pin_(rx_pin)
    , baud_rate_(baud_rate)
    , initialized_(false)
    , connected_(false)
{
    instance_ = this;
}

RosBridge::~RosBridge() {
    end();
    instance_ = nullptr;
}

esp_err_t RosBridge::initUart() {
    uart_config_t uart_config = {
        .baud_rate = baud_rate_,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    esp_err_t ret = uart_param_config(uart_port_, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = uart_set_pin(uart_port_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = uart_driver_install(uart_port_, 2048, 2048, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "UART initialized on port %d (TX:%d, RX:%d @ %d bps)", 
             uart_port_, tx_pin_, rx_pin_, baud_rate_);
    return ESP_OK;
}

esp_err_t RosBridge::begin(const char* node_name) {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }
    
    // UART 초기화
    esp_err_t ret = initUart();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // micro-ROS allocator 설정
    allocator_ = rcl_get_default_allocator();
    
    // CUSTOM_TRANSPORT 설정
    rmw_uros_set_custom_transport(
        true, // framing 비활성화
        (void*)this,
        transportOpen,
        transportClose,
        transportWrite,
        transportRead
    );
    
    ESP_LOGI(TAG, "Waiting for micro-ROS agent...");
    
    // Agent 연결 대기
    if (!waitForAgent(30000)) {
        ESP_LOGE(TAG, "Failed to connect to micro-ROS agent");
        uart_driver_delete(uart_port_);
        return ESP_FAIL;
    }
    
    connected_ = true;
    ESP_LOGI(TAG, "Connected to micro-ROS agent");
    
    // Support 초기화
    rclc_support_init(&support_, 0, NULL, &allocator_);
    
    // Node 생성
    rclc_node_init_default(&node_, node_name, "", &support_);
    ESP_LOGI(TAG, "Node '%s' created", node_name);
    
    initialized_ = true;
    return ESP_OK;
}

esp_err_t RosBridge::initialize(const char* node_name, const char* topic_name)
{
    esp_err_t ret = begin(node_name);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ROS bridge begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = createPublisher(topic_name);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Publisher creation failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "ROS bridge fully initialized (topic: %s)", topic_name);
    return ESP_OK;
}

void RosBridge::end() {
    if (initialized_) {
        rcl_publisher_fini(&publisher_, &node_);
        rcl_node_fini(&node_);
        rclc_support_fini(&support_);
        initialized_ = false;
        ESP_LOGI(TAG, "RosBridge shutdown");
    }
    
    if (connected_) {
        uart_driver_delete(uart_port_);
        connected_ = false;
    }
}

esp_err_t RosBridge::createPublisher(const char* topic_name) {
    if (!initialized_) {
        ESP_LOGE(TAG, "Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    rcl_ret_t rc = rclc_publisher_init_default(
        &publisher_,
        &node_,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        topic_name
    );
    
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "Failed to create publisher: %d", rc);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Publisher created on topic '%s'", topic_name);
    return ESP_OK;
}

esp_err_t RosBridge::publish(int32_t data) {
    if (!initialized_) {
        ESP_LOGE(TAG, "Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    msg_.data = data;
    rcl_ret_t rc = rcl_publish(&publisher_, &msg_, NULL);
    
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "Failed to publish: %d", rc);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

bool RosBridge::pingAgent(int timeout_ms, int attempts) {
    return rmw_uros_ping_agent(timeout_ms, attempts) == RMW_RET_OK;
}

bool RosBridge::waitForAgent(int timeout_ms) {
    const int ping_interval_ms = 500;
    int elapsed_ms = 0;
    
    while (elapsed_ms < timeout_ms) {
        if (pingAgent(100, 1)) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(ping_interval_ms));
        elapsed_ms += ping_interval_ms;
    }
    
    return false;
}

// ========== CUSTOM_TRANSPORT 콜백 ==========

bool RosBridge::transportOpen(struct uxrCustomTransport* transport) {
    RosBridge* self = (RosBridge*)transport->args;
    ESP_LOGI(TAG, "Transport opened");
    return true;
}

bool RosBridge::transportClose(struct uxrCustomTransport* transport) {
    RosBridge* self = (RosBridge*)transport->args;
    ESP_LOGI(TAG, "Transport closed");
    return true;
}

size_t RosBridge::transportWrite(struct uxrCustomTransport* transport, 
                                  const uint8_t* buf, size_t len, uint8_t* err) {
    RosBridge* self = (RosBridge*)transport->args;
    
    int written = uart_write_bytes(self->uart_port_, (const char*)buf, len);
    if (written < 0) {
        *err = 1;
        return 0;
    }
    
    return (size_t)written;
}

size_t RosBridge::transportRead(struct uxrCustomTransport* transport, 
                                 uint8_t* buf, size_t len, int timeout, uint8_t* err) {
    RosBridge* self = (RosBridge*)transport->args;
    
    int read = uart_read_bytes(self->uart_port_, buf, len, pdMS_TO_TICKS(timeout));
    if (read < 0) {
        *err = 1;
        return 0;
    }
    
    return (size_t)read;
}
