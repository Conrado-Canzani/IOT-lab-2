#include <stdio.h>
#include "rgb_led.h"
#include "delay.h"

#define BLINK_PERIOD_MS 500 //para poder modificarse desde main.c sin tocar las librerías

void app_main(void)
{
    // Inicializar LED
    rgb_led_init();

    while (1)
    {
        printf("Rojo\n");
        rgb_led_set_red();
        delay_ms(BLINK_PERIOD_MS);

        printf("Apagado\n");
        rgb_led_off();
        delay_ms(BLINK_PERIOD_MS);

        printf("Verde\n");
        rgb_led_set_green();
        delay_ms(BLINK_PERIOD_MS);

        printf("Apagado\n");
        rgb_led_off();
        delay_ms(BLINK_PERIOD_MS);

        printf("Azul\n");
        rgb_led_set_blue();
        delay_ms(BLINK_PERIOD_MS);

        printf("Apagado\n");
        rgb_led_off();
        delay_ms(BLINK_PERIOD_MS);
    }
}