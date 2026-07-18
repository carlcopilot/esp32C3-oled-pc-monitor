/*
  ESP32 OLED PC Monitor
  Recibe estadisticas de CPU, RAM y GPU por USB serie (JSON) desde el PC
  y las muestra en una pantalla OLED SSD1306 128x64 (I2C).

  Librerias necesarias (instalar desde el Gestor de Librerias de Arduino IDE):
    - Adafruit GFX Library
    - Adafruit SSD1306
    - ArduinoJson (por Benoit Blanchon)

  Conexion I2C (igual que en la config de ESPHome):
    SDA -> GPIO8
    SCL -> GPIO9

  Placa en Arduino IDE: ESP32C3 Dev Module
  USB CDC On Boot: Enabled
  Frecuencia I2C: 50kHz (igual que ESPHome)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
#define SDA_PIN 8
#define SCL_PIN 9
#define I2C_FREQ 50000  // 50kHz, igual que ESPHome

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float cpu = 0, ram = 0, gpu = 0, gpuTemp = 0;
float ramGb = 0, vramGb = 0, vramTotGb = 0;
unsigned long lastDataMillis = 0;
const unsigned long TIMEOUT_MS = 5000; // si no llegan datos en 5s, avisa "sin datos"

String inputBuffer = "";

void setup() {
  Serial.begin(115200);

  // Esperar a que el USB CDC del ESP32-C3 este listo (max 3 segundos)
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    delay(10);
  }
  delay(500); // margen extra

  Serial.println();
  Serial.println("==============================");
  Serial.println("  ESP32 OLED PC Monitor");
  Serial.println("==============================");

  // Inicializar I2C a 50kHz
  Serial.print("[INIT] I2C -> SDA=GPIO");
  Serial.print(SDA_PIN);
  Serial.print(", SCL=GPIO");
  Serial.print(SCL_PIN);
  Serial.print(", Freq=");
  Serial.print(I2C_FREQ);
  Serial.println(" Hz");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(I2C_FREQ);

  Serial.println("[INIT] Wire.begin() OK");

  // Verificar que la pantalla responde en el bus I2C antes de inicializarla
  Wire.beginTransmission(OLED_ADDRESS);
  byte i2cError = Wire.endTransmission();
  if (i2cError == 0) {
    Serial.print("[INIT] Pantalla detectada en 0x");
    Serial.println(OLED_ADDRESS, HEX);
  } else {
    Serial.print("[ERROR] No se detecto pantalla en 0x");
    Serial.print(OLED_ADDRESS, HEX);
    Serial.print(" (error I2C: ");
    Serial.print(i2cError);
    Serial.println(")");
    Serial.println("[ERROR] Revisa cableado, alimentacion, soldaduras.");
    Serial.println("[ERROR] Continuando de todos modos...");
  }

  // Inicializar display con reintentos
  Serial.println("[INIT] Llamando display.begin()...");

  bool displayOk = false;
  for (int intento = 1; intento <= 3; intento++) {
    Serial.print("[INIT] Intento ");
    Serial.print(intento);
    Serial.print("/3... ");

    if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println("OK!");
      displayOk = true;
      break;
    } else {
      Serial.println("FALLO");
      delay(500);
    }
  }

  if (!displayOk) {
    Serial.println("[ERROR] display.begin() fallo tras 3 intentos.");
    Serial.println("[ERROR] Posibles causas:");
    Serial.println("  - Pantalla no responde en I2C (cableado?)");
    Serial.println("  - Fallo de asignacion de memoria (framebuffer 1024 bytes)");
    Serial.println("  - Direccion I2C incorrecta (probar 0x3D?)");
    Serial.println("[ERROR] El sketch seguira intentando cada 5 segundos...");

    // Reintento infinito pero con logs, no silencioso
    while (true) {
      delay(5000);
      Serial.println("[RETRY] Reintentando display.begin()...");
      if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("[RETRY] OK! Pantalla inicializada.");
        displayOk = true;
        break;
      } else {
        Serial.println("[RETRY] Sigue fallando.");
      }
    }
  }

  // Pantalla inicializada - mostrar mensaje de espera
  Serial.println("[INIT] Mostrando pantalla de espera...");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Esperando datos...");
  display.setCursor(0, 16);
  display.println("Conecta el script");
  display.setCursor(0, 24);
  display.println("pc_monitor_serial.py");
  display.display();
  Serial.println("[INIT] Setup completado. Esperando datos JSON por Serial...");
}

void loop() {
  // Leer datos entrantes por serie linea a linea
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      parseAndShow(inputBuffer);
      inputBuffer = "";
    } else if (c != '\r') {  // ignorar \r
      inputBuffer += c;
    }
  }

  // Si llevamos demasiado tiempo sin datos, mostrar aviso
  if (millis() - lastDataMillis > TIMEOUT_MS && lastDataMillis != 0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Sin conexion");
    display.println("con el PC...");
    display.display();
  }
}

void parseAndShow(const String &jsonLine) {
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, jsonLine);

  if (error) {
    Serial.print("[WARN] JSON invalido: ");
    Serial.println(error.c_str());
    return;
  }

  cpu = doc["cpu"] | 0.0;
  ram = doc["ram"] | 0.0;
  ramGb = doc["ram_gb"] | 0.0;
  gpu = doc["gpu"] | 0.0;
  gpuTemp = doc["gpu_temp"] | 0.0;
  vramGb = doc["vram_gb"] | 0.0;
  vramTotGb = doc["vram_tot"] | 0.0;
  lastDataMillis = millis();

  drawScreen();
}

void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // 5 lineas con espaciado de 13px (caben en 64px: 0,13,26,39,52)
  display.setCursor(0, 0);
  display.print("CPU:  ");
  display.print(cpu, 0);
  display.println(" %");

  display.setCursor(0, 13);
  display.print("RAM:  ");
  display.print(ram, 0);
  display.print("%  ");
  display.print(ramGb, 1);
  display.println("G");

  display.setCursor(0, 26);
  display.print("GPU:  ");
  display.print(gpu, 0);
  display.println(" %");

  display.setCursor(0, 39);
  display.print("Temp: ");
  display.print(gpuTemp, 0);
  display.println(" C");

  display.setCursor(0, 52);
  display.print("VRAM: ");
  display.print(vramGb, 1);
  display.print("/");
  display.print(vramTotGb, 1);
  display.println(" G");

  display.display();
}
