idf_component_register(
    SRCS "src/led_strip_rmt_ws2812.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_rmt
)