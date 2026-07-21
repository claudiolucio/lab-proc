import time
import RPi.GPIO as GPIO

from components import ActiveBuzzer, iniciar_gpio


def main():
    iniciar_gpio()
    buzzer = ActiveBuzzer()

    try:
        buzzer.tocar(0.15)

        while buzzer.state.ligado:
            buzzer.atualizar()
            time.sleep(0.005)

        time.sleep(0.5)

        buzzer.tocar(0.6)

        while buzzer.state.ligado:
            buzzer.atualizar()
            time.sleep(0.005)
    finally:
        buzzer.desligar()
        GPIO.cleanup()


if __name__ == "__main__":
    main()
