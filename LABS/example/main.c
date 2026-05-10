/**
 * Firmware mínimo: comprueba transporte USB-serial + micro-ROS Agent + publicación.
 * Tópico: /pico_connection_test (std_msgs/msg/Int32), nodo: pico_link_check.
 */

#include "pico_uart_transports.h"
#include "pico/stdlib.h"

#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rmw_microros/rmw_uros.h>
#include <std_msgs/msg/int32.h>

static rcl_publisher_t publisher;
static std_msgs__msg__Int32 msg;

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
    (void)last_call_time;
    if (timer == NULL) {
        return;
    }
    (void)rcl_publish(&publisher, &msg, NULL);
    msg.data++;
}

int main(void)
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
    if (rmw_uros_ping_agent(timeout_ms, attempts) != RCL_RET_OK) {
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
    rclc_node_init_default(&node, "pico_link_check", "", &support);

    rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/pico_connection_test");

    rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(1000), timer_callback);

    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_timer(&executor, &timer);

    msg.data = 0;
    while (true) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    }
}
