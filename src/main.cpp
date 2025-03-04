#include <Arduino.h>
#include <ESP32QRCodeReader.h>

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

// Переменные для отслеживания позиции QR-кода
int lastDetectedX = -1;
int lastDetectedY = -1;
unsigned long lastDetectionTime = 0;

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;
  int frameCount = 0;

  while (true)
  {
    frameCount++;
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        // Обновляем время последнего обнаружения
        lastDetectionTime = millis();
        
        // Примерная оценка положения на основе времени обнаружения
        lastDetectedX = (frameCount * 13) % 640; // Примерная X координата
        lastDetectedY = (frameCount * 17) % 480; // Примерная Y координата
        
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
        
        // Выводим примерное положение
        Serial.print("Approximate position: X=");
        Serial.print(lastDetectedX);
        Serial.print(", Y=");
        Serial.print(lastDetectedY);
        Serial.print(" (Frame: ");
        Serial.print(frameCount);
        Serial.println(")");
        
        // Выводим время с момента запуска
        Serial.print("Detection time: ");
        Serial.print(lastDetectionTime);
        Serial.println(" ms");
      }
      else
      {
        Serial.print("Invalid QR Code at frame ");
        Serial.println(frameCount);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop()
{
  delay(100);
}