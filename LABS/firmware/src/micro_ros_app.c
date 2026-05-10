#include "micro_ros_app.h"

#include "pico/stdlib.h"
#include "pico_uart_transports.h"
#include "ultrasonic.h"

#include <rcl/error_handling.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rmw_microros/rmw_uros.h>
#include <sensor_msgs/msg/range.h>
#include <stdbool.h>
#include <string.h>

#ifndef sensor_msgs__msg__Range__ULTRASOUND
#define sensor_msgs__msg__Range__ULTRASOUND 0u
#endif

static rcl_publisher_t publisher;
static sensor_msgs__msg__Range range_msg;

static float median3(float a, float b, float c)
{
    float x = a;
    float y = b;
    float z = c;
    if (x > y) {
        const float t = x;
        x = y;
        y = t;
    }
    if (y > z) {
        const float t = y;
        y = z;
        z = t;
    }
    if (x > y) {
        const float t = x;
        x = y;
        y = t;
    }
    return y;
}

static bool median_distance_m(float *out_m)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    const bool v0 = ultrasonic_read_distance_m(&s0);
    sleep_ms(2);
    const bool v1 = ultrasonic_read_distance_m(&s1);
    sleep_ms(2);
    const bool v2 = ultrasonic_read_distance_m(&s2);

    const int n = (v0 ? 1 : 0) + (v1 ? 1 : 0) + (v2 ? 1 : 0);
    if (n == 0) {
        return false;
    }
    if (n == 1) {
        *out_m = v0 ? s0 : (v1 ? s1 : s2);
        return true;
    }
    if (n == 2) {
        if (v0 && v1) {
            *out_m = (s0 + s1) * 0.5f;
        } else if (v0 && v2) {
            *out_m = (s0 + s2) * 0.5f;
        } else {
            *out_m = (s1 + s2) * 0.5f;
        }
        return true;
    }
    *out_m = median3(s0, s1, s2);
    return true;
}

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
    (void)last_call_time;
    if (timer == NULL) {
        return;
    }

    float dist_m = 0.0f;
    const bool ok = median_distance_m(&dist_m);

    const uint64_t t_us = time_us_64();
    range_msg.header.stamp.sec = (int32_t)(t_us / 1000000ULL);
    range_msg.header.stamp.nanosec = (uint32_t)((t_us % 1000000ULL) * 1000ULL);

    range_msg.range = ok ? dist_m : -1.0f;

    (void)rcl_publish(&publisher, &range_msg, NULL);
}

void micro_ros_app_run(void)
{
    rmw_uros_set_custom_transport(
        true,
        NULL,
        pico_serial_transport_open,
        pico_serial_transport_close,
        pico_serial_transport_write,
        pico_serial_transport_read);

    const int timeout_ms = 1000;
    const uint8_t attempts = 120;
    const rcl_ret_t ping = rmw_uros_ping_agent(timeout_ms, attempts);
    if (ping != RCL_RET_OK) {
        while (true) {
            sleep_ms(500);
        }
    }

    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    rcl_node_t node;
    rcl_timer_t timer;
    rclc_executor_t executor;

    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "pico_sonar_node", "", &support);

    rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
        "/ultrasonic/range");

    memset(&range_msg, 0, sizeof(range_msg));
    static char frame_id[] = "ultrasonic_frame";
    range_msg.header.frame_id.data = frame_id;
    range_msg.header.frame_id.size = sizeof(frame_id) - 1u;
    range_msg.header.frame_id.capacity = sizeof(frame_id);

    range_msg.radiation_type = sensor_msgs__msg__Range__ULTRASOUND;
    range_msg.field_of_view = 0.26f;
    range_msg.min_range = 0.02f;
    range_msg.max_range = 4.0f;
    range_msg.range = -1.0f;

    rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(200), timer_callback);
    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_timer(&executor, &timer);

    while (true) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    }
}
