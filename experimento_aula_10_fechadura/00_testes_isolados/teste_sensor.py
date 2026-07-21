import time
import RPi.GPIO as GPIO

from components import UltrasonicSensor, iniciar_gpio


def main():
    iniciar_gpio()
    sensor = UltrasonicSensor()

    try:
        while True:
            distancia = sensor.medir_distancia_cm()
            estado = sensor.estado_porta(distancia)

            if distancia is None:
                print("Timeout na leitura.")
            else:
                print(f"{distancia:.2f} cm -> {estado}")

            time.sleep(0.5)
    finally:
        GPIO.cleanup()


if __name__ == "__main__":
    main()
