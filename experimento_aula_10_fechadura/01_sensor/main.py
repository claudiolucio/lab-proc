import time
import RPi.GPIO as GPIO

from components import UltrasonicSensor, iniciar_gpio


def main():
    iniciar_gpio()
    sensor = UltrasonicSensor()
    estado_anterior = None

    try:
        while True:
            distancia = sensor.medir_distancia_cm()
            estado = sensor.estado_porta(distancia)

            if estado != estado_anterior:
                print(f"[SENSOR] {estado}")
                estado_anterior = estado

            time.sleep(0.3)
    finally:
        GPIO.cleanup()


if __name__ == "__main__":
    main()
