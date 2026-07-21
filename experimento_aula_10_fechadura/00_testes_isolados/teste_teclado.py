import time
import RPi.GPIO as GPIO

from components import MatrixKeypad, iniciar_gpio


def main():
    iniciar_gpio()
    keypad = MatrixKeypad()

    print("TESTE ISOLADO: TECLADO")
    print("Pressione D para encerrar.")

    try:
        while True:
            key = keypad.get_key()

            if key:
                print(f"Tecla: {key}")

                if key == "D":
                    break

            time.sleep(0.002)
    finally:
        GPIO.cleanup()


if __name__ == "__main__":
    main()
