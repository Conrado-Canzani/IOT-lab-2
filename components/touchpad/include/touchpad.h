/*#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdbool.h>
#include <stdint.h>

void touchpad_init(void);

bool touchpad_is_pressed(uint8_t button_index);

#endif*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Inicializa el controlador touch con los canales 1, 2, 3, 5, 6 y 11.
 *        Realiza el escaneo inicial para calibrar los benchmarks y umbrales.
 */
void touchpad_init(void);

/**
 * @brief Consulta si un botón está siendo presionado (polling, sin FreeRTOS).
 *
 * @param button_index  Índice lógico del botón (0 a 5), correspondiente a los
 *                      canales {1, 2, 3, 5, 6, 11} en ese orden.
 * @return true  si el canal está activo (presionado)
 * @return false si no está presionado o el índice es inválido
 */
bool touchpad_is_pressed(uint8_t button_index);