idf_component_register(
    SRCS "TaskManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES freertos lcd_control ros_bridge
)
