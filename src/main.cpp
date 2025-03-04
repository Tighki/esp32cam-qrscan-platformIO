#include <Arduino.h>
#include <ESP32QRCodeReader.h>

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

// Структура для хранения информации о QR-коде
struct QRCodeInfo {
    int x;
    int y;
    unsigned long timestamp;
    String payload;
    bool isValid;
} lastQRCode;

// Константы
const int TASK_STACK_SIZE = 8 * 1024;  // Увеличиваем размер стека
const int SCAN_INTERVAL = 50;          // Уменьшаем интервал сканирования
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

// Настройки камеры
void setupCamera() {
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);     // Увеличиваем яркость (-2 to 2)
        s->set_contrast(s, 1);       // Увеличиваем контраст (-2 to 2)
        s->set_saturation(s, 0);     // Нормальная насыщенность (-2 to 2)
        s->set_whitebal(s, 1);       // Включаем автоматический баланс белого
        s->set_exposure_ctrl(s, 1);  // Включаем автоматическую экспозицию
        s->set_gainceiling(s, GAINCEILING_2X); // Устанавливаем усиление
    }
}

void onQrCodeTask(void *pvParameters)
{
    struct QRCodeData qrCodeData;
    unsigned long lastScanTime = 0;
    int consecutiveFailures = 0;
    const int MAX_FAILURES = 10;

    while (true)
    {
        unsigned long currentTime = millis();
        
        if (currentTime - lastScanTime >= SCAN_INTERVAL)
        {
            lastScanTime = currentTime;

            if (reader.receiveQrCode(&qrCodeData, SCAN_INTERVAL))
            {
                if (qrCodeData.valid)
                {
                    consecutiveFailures = 0;
                    
                    lastQRCode.timestamp = currentTime;
                    lastQRCode.payload = String((char *)qrCodeData.payload);
                    lastQRCode.isValid = true;
                    
                    lastQRCode.x = FRAME_WIDTH / 2;
                    lastQRCode.y = FRAME_HEIGHT / 2;

                    Serial.println("QR Code detected:");
                    Serial.printf("Position: X=%d, Y=%d\n", lastQRCode.x, lastQRCode.y);
                    Serial.printf("Payload: %s\n", lastQRCode.payload.c_str());
                    Serial.printf("Time: %lu ms\n", lastQRCode.timestamp);
                    Serial.println("------------------------");
                }
                else
                {
                    consecutiveFailures++;
                    if (consecutiveFailures >= MAX_FAILURES) {
                        ESP_LOGI("QR", "Too many failures, reinitializing camera...");
                        reader.begin();
                        setupCamera();
                        consecutiveFailures = 0;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Initializing QR Code Reader...");

    // Инициализация структуры
    lastQRCode.x = -1;
    lastQRCode.y = -1;
    lastQRCode.timestamp = 0;
    lastQRCode.isValid = false;

    // Настройка камеры
    reader.setup();
    setupCamera();
    
    // Запуск на втором ядре
    reader.beginOnCore(1);
    
    // Создание задачи с увеличенным размером стека
    xTaskCreate(
        onQrCodeTask,
        "onQrCode",
        TASK_STACK_SIZE,
        NULL,
        4,  // Высокий приоритет
        NULL
    );

    Serial.println("Setup completed!");
}

void loop()
{
    // Основной цикл может использоваться для других задач
    delay(100);
}