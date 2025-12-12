#include "TaskManager.h"
#include "LcdControl.h"
#include "RosBridge.h"

TaskManager::TaskManager()
{
    // 모든 Task 정보 초기화
    for (int i = 0; i < TASK_MAX; i++) {
        tasks_[i] = TaskInfo();
    }
    ESP_LOGI(TAG, "TaskManager initialized");
}

TaskManager::~TaskManager()
{
    deleteAllTasks();
    ESP_LOGI(TAG, "TaskManager destroyed");
}

esp_err_t TaskManager::createTask(TaskId id, const TaskConfig& config)
{
    if (!isValidTaskId(id)) {
        ESP_LOGE(TAG, "Invalid task ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (tasks_[id].is_running) {
        ESP_LOGW(TAG, "Task %s already running", config.name);
        return ESP_ERR_INVALID_STATE;
    }
    
    if (config.function == nullptr || config.name == nullptr) {
        ESP_LOGE(TAG, "Invalid task config: function or name is null");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Task 생성
    BaseType_t ret = xTaskCreate(
        config.function,
        config.name,
        config.stack_size,
        config.parameter,
        config.priority,
        &tasks_[id].handle
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task %s", config.name);
        return ESP_FAIL;
    }
    
    // Task 정보 저장
    tasks_[id].name = config.name;
    tasks_[id].stack_size = config.stack_size;
    tasks_[id].priority = config.priority;
    tasks_[id].is_running = true;
    
    ESP_LOGI(TAG, "Task created: %s (stack: %lu, priority: %d)", 
             config.name, config.stack_size, config.priority);
    
    return ESP_OK;
}

esp_err_t TaskManager::deleteTask(TaskId id)
{
    if (!isValidTaskId(id)) {
        ESP_LOGE(TAG, "Invalid task ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!tasks_[id].is_running) {
        ESP_LOGW(TAG, "Task %s not running", taskIdToString(id));
        return ESP_ERR_INVALID_STATE;
    }
    
    if (tasks_[id].handle != nullptr) {
        const char* name = tasks_[id].name;
        vTaskDelete(tasks_[id].handle);
        tasks_[id].handle = nullptr;
        tasks_[id].is_running = false;
        ESP_LOGI(TAG, "Task deleted: %s", name);
    }
    
    return ESP_OK;
}

esp_err_t TaskManager::suspendTask(TaskId id)
{
    if (!isValidTaskId(id)) {
        ESP_LOGE(TAG, "Invalid task ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!tasks_[id].is_running || tasks_[id].handle == nullptr) {
        ESP_LOGW(TAG, "Task %s not running", taskIdToString(id));
        return ESP_ERR_INVALID_STATE;
    }
    
    vTaskSuspend(tasks_[id].handle);
    ESP_LOGI(TAG, "Task suspended: %s", tasks_[id].name);
    
    return ESP_OK;
}

esp_err_t TaskManager::resumeTask(TaskId id)
{
    if (!isValidTaskId(id)) {
        ESP_LOGE(TAG, "Invalid task ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!tasks_[id].is_running || tasks_[id].handle == nullptr) {
        ESP_LOGW(TAG, "Task %s not running", taskIdToString(id));
        return ESP_ERR_INVALID_STATE;
    }
    
    vTaskResume(tasks_[id].handle);
    ESP_LOGI(TAG, "Task resumed: %s", tasks_[id].name);
    
    return ESP_OK;
}

bool TaskManager::isTaskRunning(TaskId id) const
{
    if (!isValidTaskId(id)) {
        return false;
    }
    return tasks_[id].is_running;
}

TaskHandle_t TaskManager::getTaskHandle(TaskId id) const
{
    if (!isValidTaskId(id)) {
        return nullptr;
    }
    return tasks_[id].handle;
}

const char* TaskManager::getTaskName(TaskId id) const
{
    if (!isValidTaskId(id)) {
        return "INVALID";
    }
    return tasks_[id].name ? tasks_[id].name : "UNKNOWN";
}

uint32_t TaskManager::getTaskStackHighWaterMark(TaskId id) const
{
    if (!isValidTaskId(id) || !tasks_[id].is_running || tasks_[id].handle == nullptr) {
        return 0;
    }
    
    return uxTaskGetStackHighWaterMark(tasks_[id].handle);
}

void TaskManager::printTaskStats() const
{
    ESP_LOGI(TAG, "=== Task Manager Statistics ===");
    
    for (int i = 0; i < TASK_MAX; i++) {
        if (tasks_[i].is_running) {
            uint32_t hwm = getTaskStackHighWaterMark(static_cast<TaskId>(i));
            uint32_t used = tasks_[i].stack_size - hwm;
            float usage = (float)used / tasks_[i].stack_size * 100.0f;
            
            ESP_LOGI(TAG, "Task: %-20s | Stack: %5lu/%5lu (%5.1f%%) | Priority: %d | State: Running",
                     tasks_[i].name,
                     used,
                     tasks_[i].stack_size,
                     usage,
                     tasks_[i].priority);
        } else {
            ESP_LOGI(TAG, "Task: %-20s | State: Stopped", taskIdToString(static_cast<TaskId>(i)));
        }
    }
    
    ESP_LOGI(TAG, "================================");
}

void TaskManager::printAllTaskStats() const
{
    ESP_LOGI(TAG, "=== FreeRTOS All Tasks ===");
    
    // Task 개수 확인
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    ESP_LOGI(TAG, "Total tasks: %d", task_count);
    
    // Task 상태 배열 할당
    TaskStatus_t* task_status_array = new TaskStatus_t[task_count];
    if (task_status_array == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate memory for task status");
        return;
    }
    
    // Task 상태 가져오기
    uint32_t total_runtime;
    UBaseType_t array_size = uxTaskGetSystemState(task_status_array, task_count, &total_runtime);
    
    // Task 정보 출력
    for (UBaseType_t i = 0; i < array_size; i++) {
        ESP_LOGI(TAG, "%-20s | Priority: %2d | Stack HWM: %5d | State: %d",
                 task_status_array[i].pcTaskName,
                 task_status_array[i].uxCurrentPriority,
                 task_status_array[i].usStackHighWaterMark,
                 task_status_array[i].eCurrentState);
    }
    
    delete[] task_status_array;
    ESP_LOGI(TAG, "==========================");
}

void TaskManager::deleteAllTasks()
{
    ESP_LOGI(TAG, "Deleting all tasks...");
    
    for (int i = 0; i < TASK_MAX; i++) {
        if (tasks_[i].is_running) {
            deleteTask(static_cast<TaskId>(i));
        }
    }
    
    ESP_LOGI(TAG, "All tasks deleted");
}

void TaskManager::suspendAllTasks()
{
    ESP_LOGI(TAG, "Suspending all tasks...");
    
    for (int i = 0; i < TASK_MAX; i++) {
        if (tasks_[i].is_running) {
            suspendTask(static_cast<TaskId>(i));
        }
    }
    
    ESP_LOGI(TAG, "All tasks suspended");
}

void TaskManager::resumeAllTasks()
{
    ESP_LOGI(TAG, "Resuming all tasks...");
    
    for (int i = 0; i < TASK_MAX; i++) {
        if (tasks_[i].is_running) {
            resumeTask(static_cast<TaskId>(i));
        }
    }
    
    ESP_LOGI(TAG, "All tasks resumed");
}

bool TaskManager::isValidTaskId(TaskId id) const
{
    return id >= 0 && id < TASK_MAX;
}

const char* TaskManager::taskIdToString(TaskId id) const
{
    switch (id) {
        case TASK_UI:        return "TASK_UI";
        case TASK_ROS:       return "TASK_ROS";
        case TASK_CAN_RX:    return "TASK_CAN_RX";
        case TASK_ESPNOW_RX: return "TASK_ESPNOW_RX";
        default:             return "UNKNOWN";
    }
}

// UI Task 함수
void TaskManager::uiTaskFunction(void* parameter)
{
    UiTaskContext* ctx = static_cast<UiTaskContext*>(parameter);
    uint32_t lcd_counter = 0;
    
    ESP_LOGI(TAG, "UI Task started");
    
    while (true) {
        // LCD 업데이트
        if (lcd_counter >= ctx->update_interval_ms) {
            ctx->lcd->renderUI();
            lcd_counter = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms 틱
        lcd_counter += 10;
    }
}

// micro-ROS Task 함수
void TaskManager::rosTaskFunction(void* parameter)
{
    RosTaskContext* ctx = static_cast<RosTaskContext*>(parameter);
    
    ESP_LOGI(TAG, "micro-ROS Task started");
    
    while (true) {
        // 연결 상태 체크 및 재연결
        if (!ctx->ros->isConnected()) {
            ESP_LOGW(TAG, "micro-ROS disconnected, trying to reconnect...");
            ctx->lcd->updateConnectionStatus(false);
            
            if (ctx->ros->waitForAgent(5000)) {
                ESP_LOGI(TAG, "Reconnected to micro-ROS agent");
                ctx->lcd->updateConnectionStatus(true);
            }
        }
        
        // TODO: Subscribe 콜백 처리
        
        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms 주기
    }
}

// UI Task 생성 헬퍼
esp_err_t TaskManager::createUiTask(LcdControl* lcd, uint32_t update_interval_ms, 
                                    uint32_t stack_size, UBaseType_t priority)
{
    static UiTaskContext ui_ctx;
    ui_ctx.lcd = lcd;
    ui_ctx.update_interval_ms = update_interval_ms;
    
    TaskConfig config("ui_task", uiTaskFunction, stack_size, priority, &ui_ctx);
    return createTask(TASK_UI, config);
}

// ROS Task 생성 헬퍼
esp_err_t TaskManager::createRosTask(RosBridge* ros, LcdControl* lcd, 
                                     uint32_t stack_size, UBaseType_t priority)
{
    static RosTaskContext ros_ctx;
    ros_ctx.ros = ros;
    ros_ctx.lcd = lcd;
    
    TaskConfig config("microros_task", rosTaskFunction, stack_size, priority, &ros_ctx);
    return createTask(TASK_ROS, config);
}
}
