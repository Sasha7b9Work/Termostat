// 2022/10/13 16:23:33 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware.h"
#include <cstring>


#define BUF_SIZE (1024)


static void MainTask(void *cookie)
{
    static int counter = 0;

    while (1)
    {
        GPIO::Set(GPIO_NUM_2, counter++ % 2);

        UART0::Send("Test message");

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void app_main()
{
    GPIO::Init(GPIO_NUM_2);

    UART0::Init();

    xTaskCreate(MainTask, "MainTask", 1024, NULL, 5, NULL);
}

#ifdef __cplusplus
}
#endif
