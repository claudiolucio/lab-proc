import time
import RPi.GPIO as GPIO

import config
from components import MatrixKeypad, UltrasonicSensor, iniciar_gpio


def main():
    iniciar_gpio()
    sensor = UltrasonicSensor()
    keypad = MatrixKeypad()

    proxima_leitura = 0.0
    estado_porta = None
    executando = True

    try:
        while executando:
            agora = time.monotonic()

            if agora >= proxima_leitura:
                proxima_leitura = agora + config.PERIODO_SENSOR_S
                distancia = sensor.medir_distancia_cm()
                novo_estado = sensor.estado_porta(distancia)

                if novo_estado != estado_porta:
                    estado_porta = novo_estado
                    print(f"[SENSOR] {estado_porta}")

            key = keypad.get_key()

            if key:
                print(f"[TECLADO] {key}")

                if key == "D":
                    executando = False

            time.sleep(0.002)
    finally:
        GPIO.cleanup()


if __name__ == "__main__":
    main()
