import time
import RPi.GPIO as GPIO

import config
from components import LCD1602, MatrixKeypad, UltrasonicSensor, iniciar_gpio


def main():
    iniciar_gpio()
    sensor = UltrasonicSensor()
    keypad = MatrixKeypad()
    lcd = LCD1602()

    entrada = ""
    proxima_leitura = 0.0
    estado_porta = None
    executando = True

    lcd.show("Etapa 3", "Inicializando")

    try:
        while executando:
            agora = time.monotonic()

            if agora >= proxima_leitura:
                proxima_leitura = agora + config.PERIODO_SENSOR_S
                distancia = sensor.medir_distancia_cm()
                novo_estado = sensor.estado_porta(distancia)

                if novo_estado != estado_porta:
                    estado_porta = novo_estado
                    lcd.show("Porta:", estado_porta)

            key = keypad.get_key()

            if not key:
                time.sleep(0.002)
                continue

            if key == "D":
                executando = False
            elif key == "*":
                entrada = ""
                lcd.show("Digite a senha:", "")
            elif key.isdigit():
                if len(entrada) < 6:
                    entrada += key

                lcd.show("Digite a senha:", "*" * len(entrada))
            elif key == "#":
                lcd.show("Entrada recebida", "*" * len(entrada))
                print(f"[ENTRADA] {entrada}")
                entrada = ""

            time.sleep(0.002)
    finally:
        lcd.show("Sistema", "Encerrado")
        lcd.close()
        GPIO.cleanup()


if __name__ == "__main__":
    main()
