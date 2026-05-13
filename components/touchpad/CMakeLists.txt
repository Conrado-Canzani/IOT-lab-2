idf_component_register(
    SRCS "touchpad.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_touch_sens
)