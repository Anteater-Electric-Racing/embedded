#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <FreeRTOS.h>
#include <semphr.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

float objectTemp = 0.0f;
SemaphoreHandle_t tempMutex;

void tempTask(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        float t = mlx.readObjectTempC();

        xSemaphoreTake(tempMutex, portMAX_DELAY);
        objectTemp = t;
        xSemaphoreGive(tempMutex);

        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz polling
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    if (!mlx.begin()) {
        Serial.println("MLX90614 not found!");
        while (1);
    }

    tempMutex = xSemaphoreCreateMutex();
    xTaskCreate(tempTask, "TempTask", 2048, NULL, 1, NULL);
}

float getTireTemp() {
    xSemaphoreTake(tempMutex, portMAX_DELAY);
    float t = objectTemp;
    xSemaphoreGive(tempMutex);
    return t;
}
