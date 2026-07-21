import time
import RPi.GPIO as GPIO

import config
from components import (
    ActiveBuzzer,
    LCD1602,
    MatrixKeypad,
    UltrasonicSensor,
    iniciar_gpio,
)


def main():
    iniciar_gpio()

    sensor = UltrasonicSensor()
    keypad = MatrixKeypad()
    buzzer = ActiveBuzzer()
    lcd = LCD1602()

    entrada = ""
    erros = 0
    bloqueado = False
    fim_bloqueio = 0.0
    fim_mensagem = 0.0
    proxima_leitura = 0.0
    estado_porta = None
    executando = True

    lcd.show("Digite a senha:", "")

    try:
        while executando:
            agora = time.monotonic()
            buzzer.atualizar()

            if bloqueado and agora >= fim_bloqueio:
                bloqueado = False
                erros = 0
                lcd.show("Digite a senha:", "")

            if agora >= proxima_leitura:
                proxima_leitura = agora + config.PERIODO_SENSOR_S
                distancia = sensor.medir_distancia_cm()
                novo_estado = sensor.estado_porta(distancia)

                if novo_estado != estado_porta:
                    estado_porta = novo_estado
                    print(f"[SENSOR] {estado_porta}")

                    if estado_porta == "ABERTA" and fim_mensagem == 0.0:
                        lcd.show("ALERTA", "Porta aberta")
                        buzzer.tocar(0.3)
                        fim_mensagem = agora + 1.2

            if fim_mensagem and agora >= fim_mensagem:
                fim_mensagem = 0.0

                if not bloqueado:
                    lcd.show("Digite a senha:", "*" * len(entrada))

            key = keypad.get_key()

            if not key:
                time.sleep(0.002)
                continue

            print(f"[TECLADO] {key}")

            if key == "D":
                executando = False
                continue

            if bloqueado:
                restante = max(0, int(fim_bloqueio - agora) + 1)
                lcd.show("BLOQUEADO", f"Aguarde {restante}s")
                continue

            if key.isdigit():
                if len(entrada) < 6:
                    entrada += key

                lcd.show("Digite a senha:", "*" * len(entrada))

            elif key == "*":
                entrada = ""
                lcd.show("Digite a senha:", "")

            elif key == "#":
                if entrada == config.SENHA_CORRETA:
                    erros = 0
                    lcd.show("ACESSO LIBERADO", "Senha correta")
                    buzzer.tocar(0.15)
                    fim_mensagem = agora + 2.0
                else:
                    erros += 1
                    lcd.show(
                        "ACESSO NEGADO",
                        f"Erro {erros}/{config.MAX_ERROS}",
                    )
                    buzzer.tocar(0.6)
                    fim_mensagem = agora + 1.5

                    if erros >= config.MAX_ERROS:
                        bloqueado = True
                        fim_bloqueio = agora + config.TEMPO_BLOQUEIO_S
                        lcd.show("BLOQUEADO", "Aguarde 10s")

                entrada = ""

            time.sleep(0.002)

    finally:
        buzzer.desligar()
        lcd.show("Sistema", "Encerrado")
        lcd.close()
        GPIO.cleanup()


if __name__ == "__main__":
    main()
