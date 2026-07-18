"""
PC Monitor Serial - envia estadisticas de CPU, RAM y GPU (NVIDIA) al ESP32
por puerto serie USB en formato JSON.

Requisitos (instalar con pip):
    pip install -r requirements.txt

Antes de ejecutar:
    1. Conecta el ESP32 por USB.
    2. Abre el Administrador de dispositivos de Windows > Puertos (COM y LPT)
       y anota el puerto COM que aparece al conectar el ESP32 (ej. COM6).
    3. Cambia la variable SERIAL_PORT mas abajo con ese puerto.
"""

import json
import time

import psutil
import serial

try:
    import pynvml
    pynvml.nvmlInit()
    gpu_handle = pynvml.nvmlDeviceGetHandleByIndex(0)
    NVML_OK = True
except Exception as e:
    print(f"Aviso: no se pudo inicializar NVML ({e}). Se enviara GPU=0.")
    NVML_OK = False

# --- CONFIGURACION ---
SERIAL_PORT = "COM6"      # Puerto USB del ESP32-C3
BAUD_RATE = 115200
UPDATE_INTERVAL = 2        # segundos entre actualizaciones
RECONNECT_DELAY = 3        # segundos entre reintentos de conexion


def get_gpu_stats():
    if not NVML_OK:
        return 0, 0, 0, 0
    util = pynvml.nvmlDeviceGetUtilizationRates(gpu_handle)
    temp = pynvml.nvmlDeviceGetTemperature(gpu_handle, pynvml.NVML_TEMPERATURE_GPU)
    mem = pynvml.nvmlDeviceGetMemoryInfo(gpu_handle)
    vram_used = round(mem.used / (1024 ** 2))    # MB
    vram_total = round(mem.total / (1024 ** 2))   # MB
    return util.gpu, temp, vram_used, vram_total


def connect_serial():
    """Intenta conectar al puerto serie, reintentando indefinidamente."""
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            # Desactivar DTR para evitar resets del ESP32 al reconectar
            ser.dtr = False
            time.sleep(0.1)
            ser.dtr = True
            time.sleep(2)  # esperar a que el ESP32 reinicie tras abrir el puerto
            print(f"Conectado a {SERIAL_PORT}.")
            return ser
        except serial.SerialException as e:
            print(f"No se pudo abrir {SERIAL_PORT}: {e}")
            print(f"Reintentando en {RECONNECT_DELAY}s...")
            time.sleep(RECONNECT_DELAY)


def main():
    print(f"Conectando a {SERIAL_PORT} @ {BAUD_RATE} baudios...")
    ser = connect_serial()
    print("Enviando datos... (Ctrl+C para detener)")

    sent_count = 0
    try:
        while True:
            cpu = psutil.cpu_percent(interval=None)
            mem = psutil.virtual_memory()
            ram = mem.percent
            ram_used_gb = round(mem.used / (1024 ** 3), 1)
            ram_total_gb = round(mem.total / (1024 ** 3), 1)
            gpu, gpu_temp, vram_used, vram_total = get_gpu_stats()

            data = {
                "cpu": round(cpu, 1),
                "ram": round(ram, 1),
                "ram_gb": ram_used_gb,
                "gpu": round(gpu, 1),
                "gpu_temp": round(gpu_temp, 1),
                "vram_gb": round(vram_used / 1024, 1),
                "vram_tot": round(vram_total / 1024, 1),
            }

            line = json.dumps(data) + "\n"

            try:
                ser.write(line.encode("utf-8"))
                sent_count += 1
                print(f"[{sent_count}] Enviado: {line.strip()}")
            except serial.SerialException as e:
                print(f"\nError de escritura: {e}")
                print("Reconectando...")
                try:
                    ser.close()
                except Exception:
                    pass
                time.sleep(RECONNECT_DELAY)
                ser = connect_serial()
                print("Reconectado. Reanudando envio de datos...")

            time.sleep(UPDATE_INTERVAL)
    except KeyboardInterrupt:
        print("\nDetenido por el usuario.")
    finally:
        try:
            ser.close()
        except Exception:
            pass
        if NVML_OK:
            pynvml.nvmlShutdown()


if __name__ == "__main__":
    main()
