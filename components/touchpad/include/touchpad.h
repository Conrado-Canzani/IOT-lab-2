#pragma once

#include <stdbool.h>
#include <stdint.h>

// Cantidad de botones touch que vas a usar
#define TOUCHPAD_NUM_BUTTONS 6

// Inicializa los touchpads y hace la calibración
void touchpad_init(void);

// Se llama periódicamente (en el while)
void touchpad_update(void);

// Estado actual (nivel)
bool touchpad_is_pressed(uint8_t index);

// Eventos (flancos)
bool touchpad_was_pressed(uint8_t index);   // cuando pasa de NO tocado → tocado
bool touchpad_was_released(uint8_t index);  // cuando pasa de tocado → NO tocado