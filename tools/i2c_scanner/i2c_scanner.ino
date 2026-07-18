/*
  I2C Scanner para ESP32-C3
  Sube este sketch para comprobar si el ESP32 detecta la pantalla OLED
  en el bus I2C. Abre el Monitor Serie a 115200 baudios despues de subirlo.

  IMPORTANTE: Usa 50kHz de frecuencia I2C, igual que la config de ESPHome
  que funcionaba correctamente.
*/

#include <Wire.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define I2C_FREQ 50000  // 50kHz, igual que ESPHome

void setup() {
  Serial.begin(115200);

  // Esperar a que el USB CDC del ESP32-C3 este listo (max 3 segundos)
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    delay(10);
  }
  delay(500); // margen extra

  Serial.println();
  Serial.println("========================================");
  Serial.println("  I2C Scanner - ESP32-C3");
  Serial.println("========================================");
  Serial.print("SDA: GPIO"); Serial.println(SDA_PIN);
  Serial.print("SCL: GPIO"); Serial.println(SCL_PIN);
  Serial.print("Frecuencia I2C: "); Serial.print(I2C_FREQ); Serial.println(" Hz");
  Serial.println();

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(I2C_FREQ);
}

void loop() {
  byte error, address;
  int devicesFound = 0;

  Serial.println("Escaneando bus I2C...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("  -> Dispositivo encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      if (address == 0x3C || address == 0x3D) {
        Serial.print("  <-- SSD1306 OLED!");
      }
      Serial.println();
      devicesFound++;
    } else if (error == 4) {
      Serial.print("  -> Error desconocido en 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (devicesFound == 0) {
    Serial.println("  No se encontro ningun dispositivo I2C.");
    Serial.println("  Revisa: cableado, alimentacion, soldaduras.");
  } else {
    Serial.print("  ");
    Serial.print(devicesFound);
    Serial.println(" dispositivo(s) encontrado(s).");
  }

  Serial.println();
  delay(3000);
}
