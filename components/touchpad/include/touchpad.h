#pragma once

#include <stdint.h>
#include <stdbool.h>


//inicializa el controlador touch con los canales 1, 2, 3, 5, 6 y 11.
//realiza el escaneo inicial para calibrar los benchmarks y umbrales.

void touchpad_init(void);


//consulta si un botón está siendo presionado (polling, sin FreeRTOS).
 
//button_index  Índice lógico del botón (0 a 5), correspondiente a los
//canales {1, 2, 3, 5, 6, 11} en ese orden.
//true  si el canal está activo (presionado)
//false si no está presionado o el índice es inválido
 
bool touchpad_is_pressed(uint8_t button_index);