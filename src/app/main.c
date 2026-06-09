/**
 * @file  main.c
 * @brief Firmware entry point.
 *
 * SystemInit (clock tree) runs from the startup code before main(). Here we
 * start the time base and hand control to the application head module.
 */
#include "app/app.h"
#include "hal/hal_time.h"

int main(void) {
    (void) hal_time_init();
    app_run(); /* does not return */
    for (;;) {
    }
}
