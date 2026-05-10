#include "micro_ros_app.h"
#include "ultrasonic.h"

#include "pico/stdlib.h"

int main(void)
{
    ultrasonic_init();
    micro_ros_app_run();
    return 0;
}
