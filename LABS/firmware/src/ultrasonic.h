#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdbool.h>

/** GPIO de salida TRIG (3,3 V). Cambiar en `ultrasonic.c` si tu cableado difiere. */
#ifndef ULTRASONIC_TRIG_PIN
#define ULTRASONIC_TRIG_PIN 14u
#endif

/** GPIO de entrada ECHO (3,3 V o con adaptación de nivel desde 5 V). */
#ifndef ULTRASONIC_ECHO_PIN
#define ULTRASONIC_ECHO_PIN 15u
#endif

void ultrasonic_init(void);

/**
 * Una lectura de distancia en metros.
 * @param out_m distancia válida solo si retorna true
 * @return false si timeout, eco inválido o fuera de rango físico
 */
bool ultrasonic_read_distance_m(float *out_m);

#endif
