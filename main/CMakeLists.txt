idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES rgb_led delay esp_driver_touch_sens
)