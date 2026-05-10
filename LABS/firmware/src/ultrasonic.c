#include "ultrasonic.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <math.h>

/** Eco máximo ~30 ms equivale a ~5 m ida y vuelta; margen para timeout. */
#define ULTRASONIC_TIMEOUT_US 35000u

/** HC-SR04: ~2 cm mínimo práctico. */
#define DIST_MIN_M 0.02f

/** Límite superior razonable para demo (sensor típico ~4 m). */
#define DIST_MAX_M 4.0f

/** Velocidad del sonido a 20 °C en m/s (documentado en docs/wiring.md). */
#define SPEED_OF_SOUND_M_S 343.0f

static void pulse_trig(void)
{
    gpio_put(ULTRASONIC_TRIG_PIN, 0);
    sleep_us(2);
    gpio_put(ULTRASONIC_TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(ULTRASONIC_TRIG_PIN, 0);
}

void ultrasonic_init(void)
{
    gpio_init(ULTRASONIC_TRIG_PIN);
    gpio_set_dir(ULTRASONIC_TRIG_PIN, GPIO_OUT);
    gpio_put(ULTRASONIC_TRIG_PIN, 0);

    gpio_init(ULTRASONIC_ECHO_PIN);
    gpio_set_dir(ULTRASONIC_ECHO_PIN, GPIO_IN);
    gpio_pull_down(ULTRASONIC_ECHO_PIN);
}

bool ultrasonic_read_distance_m(float *out_m)
{
    if (!out_m) {
        return false;
    }

    pulse_trig();

    const uint64_t deadline = time_us_64() + ULTRASONIC_TIMEOUT_US;

    while (gpio_get(ULTRASONIC_ECHO_PIN) == 0) {
        if (time_us_64() > deadline) {
            return false;
        }
    }

    const uint64_t pulse_start = time_us_64();

    while (gpio_get(ULTRASONIC_ECHO_PIN) != 0) {
        if (time_us_64() > deadline) {
            return false;
        }
    }

    const uint64_t pulse_end = time_us_64();
    const uint64_t pulse_us = pulse_end - pulse_start;

    const float seconds = (float)pulse_us / 1000000.0f;
    const float dist = (seconds * SPEED_OF_SOUND_M_S) * 0.5f;

    if (!isfinite(dist) || dist < DIST_MIN_M || dist > DIST_MAX_M) {
        return false;
    }

    *out_m = dist;
    return true;
}
