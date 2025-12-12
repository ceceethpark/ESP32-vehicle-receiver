#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include <cstring>

// Forward declarations
class LcdControl;
class RosBridge;

class TaskManager {
public:
    // Task 컨텍스트 구조체
    struct UiTaskContext {
        LcdControl* lcd;
        uint32_t update_interval_ms;
    };
    
    struct RosTaskContext {
        RosBridge* ros;
        LcdControl* lcd;
    };
    
    // Task ID 정의
    enum TaskId {
        TASK_UI,
        TASK_ROS,
        TASK_CAN_RX,
        TASK_ESPNOW_RX,
        TASK_MAX
    };
    
    // Task 설정 구조체
    struct TaskConfig {
        const char* name;
        TaskFunction_t function;
        uint32_t stack_size;
        UBaseType_t priority;
        void* parameter;
        
        TaskConfig()
            : name(nullptr)
            , function(nullptr)
            , stack_size(2048)
            , priority(5)
            , parameter(nullptr)
        {}
        
        TaskConfig(const char* n, TaskFunction_t f, uint32_t stack, UBaseType_t prio, void* param = nullptr)
            : name(n)
            , function(f)
            , stack_size(stack)
            , priority(prio)
            , parameter(param)
        {}
    };
    
    // Task 정보 구조체
    struct TaskInfo {
        TaskHandle_t handle;
        const char* name;
        uint32_t stack_size;
        UBaseType_t priority;
        bool is_running;
        
        TaskInfo()
            : handle(nullptr)
            , name(nullptr)
            , stack_size(0)
            , priority(0)
            , is_running(false)
        {}
    };
    
    // 생성자/소멸자
    TaskManager();
    ~TaskManager();
    
    // Task 관리
    esp_err_t createTask(TaskId id, const TaskConfig& config);
    esp_err_t deleteTask(TaskId id);
    esp_err_t suspendTask(TaskId id);
    esp_err_t resumeTask(TaskId id);
    
    // Task 상태 조회
    bool isTaskRunning(TaskId id) const;
    TaskHandle_t getTaskHandle(TaskId id) const;
    const char* getTaskName(TaskId id) const;
    uint32_t getTaskStackHighWaterMark(TaskId id) const;
    
    // Task 통계
    void printTaskStats() const;
    void printAllTaskStats() const;  // FreeRTOS 전체 Task 통계
    
    // Task 일괄 관리
    void deleteAllTasks();
    void suspendAllTasks();
    void resumeAllTasks();
    
    // 특수 Task 생성 헬퍼
    esp_err_t createUiTask(LcdControl* lcd, uint32_t update_interval_ms, uint32_t stack_size, UBaseType_t priority);
    esp_err_t createRosTask(RosBridge* ros, LcdControl* lcd, uint32_t stack_size, UBaseType_t priority);
    
private:
    TaskInfo tasks_[TASK_MAX];
    
    // 내장 Task 함수
    static void uiTaskFunction(void* parameter);
    static void rosTaskFunction(void* parameter);
    
    // 로그 태그
    static constexpr const char* TAG = "TaskManager";
    
    // Helper 함수
    bool isValidTaskId(TaskId id) const;
    const char* taskIdToString(TaskId id) const;
};

#endif // TASK_MANAGER_H
