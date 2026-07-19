# ESP32 OLED PC Monitor 🖥️🔋

![Vista previa del monitor OLED 1](images/preview.jpg)
![Vista previa del monitor OLED 2](images/preview2.jpg)

Este proyecto permite mostrar estadísticas en tiempo real del uso de **CPU, RAM, GPU (NVIDIA) y VRAM** de un PC con Windows en una pantalla OLED SSD1306 (128x64, I2C) controlada por un microcontrolador **ESP32-C3** conectado a través de USB (sin depender de WiFi).

Para la versión en inglés, consulta el archivo [README.md](README.md).

## 🚀 Características
- **Datos mostrados**:
  - Uso de CPU (%)
  - Uso de RAM (% y cantidad usada en GB)
  - Uso de GPU (%)
  - Temperatura de GPU (°C)
  - Uso de VRAM (GB usados / GB totales)
- **Protección OLED**:
  - Apagado automático de la pantalla tras 30 segundos sin recibir datos para evitar burn-in.
  - La pantalla se enciende instantáneamente al volver a recibir datos.
  - Tiempo configurable mediante la constante `SCREEN_OFF_MS` en el sketch.
- **Robustez**:
  - Auto-reconexión del script Python si el cable USB se desconecta.
  - Gestión mejorada del inicio del puerto serie (USB CDC) en placas ESP32-C3.
  - Reintentos continuos con feedback en caso de fallos de inicialización I2C de la pantalla OLED.

---

## 🛠️ Requisitos de Hardware
1. Placa de desarrollo **ESP32-C3** (ej. ESP32-C3 Super Mini o similar).
2. Pantalla **OLED SSD1306** de 128x64 píxeles (I2C, dirección por defecto `0x3C`).
3. Cable de datos Micro-USB o USB-C.
4. Conexiones físicas (probadas y validadas a 50 kHz):
   - **SDA** -> **GPIO 8** (ESP32-C3)
   - **SCL** -> **GPIO 9** (ESP32-C3)
   - **VCC** -> **3.3V**
   - **GND** -> **GND**

---

## 💻 Instalación y Configuración

### 1. Preparación del ESP32-C3 (Arduino IDE)
Asegúrate de instalar las siguientes librerías desde el Gestor de Librerías del Arduino IDE:
- **Adafruit SSD1306**
- **Adafruit GFX Library**
- **ArduinoJson** (versión 6 o 7)

#### Parámetros de subida recomendados en Arduino IDE:
- **Placa**: *ESP32C3 Dev Module* (o la correspondiente a tu placa).
- **USB CDC On Boot**: *Enabled* (imprescindible en placas C3 para habilitar la comunicación serie nativa por USB).
- Abre el sketch principal `esp32_oled_pc_monitor.ino` y súbelo al ESP32.

> 💡 **Nota**: Si tienes dudas sobre si la pantalla está bien cableada, puedes subir el sketch auxiliar situado en `tools/i2c_scanner/i2c_scanner.ino`. Abre el Monitor Serie a `115200` baudios para confirmar que encuentra tu dispositivo en la dirección `0x3C`.

---

### 2. Configuración del PC (Python)
Necesitas tener Python instalado en tu ordenador.

1. Instala las dependencias necesarias mediante pip:
   ```bash
   pip install -r requirements.txt
   ```
   *(Instala las librerías `psutil`, `pyserial` y `nvidia-ml-py`)*.

2. Abre el archivo `pc_monitor_serial.py` y edita la variable `SERIAL_PORT` para asignar el puerto COM correspondiente a tu placa (puedes consultarlo en el *Administrador de dispositivos* de Windows bajo la sección de "Puertos (COM y LPT)"):
   ```python
   SERIAL_PORT = "COM6"  # <-- Cambia esto por tu puerto real
   ```

3. Ejecuta el script:
   ```bash
   python pc_monitor_serial.py
   ```

El script detectará automáticamente tu tarjeta gráfica NVIDIA e iniciará la transmisión de datos cada 2 segundos. La pantalla OLED mostrará la información en tiempo real de inmediato.

> 💡 **Nota**: Si el script Python no está en ejecución (o deja de enviar datos), la pantalla OLED se apagará automáticamente a los 30 segundos para evitar el burn-in. Se enciende de nuevo en cuanto recibe datos. Puedes ajustar el tiempo modificando el valor de `SCREEN_OFF_MS` en `esp32_oled_pc_monitor.ino`.

---

### 3. Ejecución automática al iniciar Windows (Persistencia)
Si quieres que el script se ejecute automáticamente en segundo plano cada vez que enciendas tu PC:

1. Haz doble clic en el archivo **`register_startup.bat`** (si falla, haz clic derecho y selecciona **Ejecutar como administrador**).
2. Esto creará una tarea programada en Windows que ejecuta `pc_monitor_serial.py` en segundo plano usando `pythonw.exe` (de esta forma corre de manera invisible sin abrir ninguna ventana de terminal negra).
3. Si en algún momento deseas desactivar el inicio automático, haz doble clic en **`unregister_startup.bat`**.

---

## 📐 Atribución de la Carcasa Impresa en 3D
La carcasa impresa en 3D utilizada para este proyecto es un remix del modelo:
- **ZMK Nice Nano 128x64 OLED Dongle** por James
- Enlace original: [Modelo en Printables 1443110](https://www.printables.com/model/1443110-zmk-nice-nano-128x64-oled-dongle-xiao-ble-remix)
- Licencia: [Creative Commons Attribution (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/)


