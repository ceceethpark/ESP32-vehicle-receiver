idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "." "include"
    REQUIRES can_control espnow_control ros_bridge button_control led_control lcd_control
)
