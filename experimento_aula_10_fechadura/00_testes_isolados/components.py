import time
from dataclasses import dataclass

import RPi.GPIO as GPIO
from smbus2 import SMBus

import config


def iniciar_gpio() -> None:
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)


class MatrixKeypad:
    def __init__(self, debounce_s: float = 0.05) -> None:
        self.debounce_s = debounce_s
        self._last_raw = None
        self._stable_key = None
        self._changed_at = time.monotonic()

        for pin in config.ROW_PINS:
            GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

        for pin in config.COL_PINS:
            GPIO.setup(pin, GPIO.IN)

    def _scan_raw(self):
        for col_index, col_pin in enumerate(config.COL_PINS):
            GPIO.setup(col_pin, GPIO.OUT, initial=GPIO.LOW)

            for row_index, row_pin in enumerate(config.ROW_PINS):
                if GPIO.input(row_pin) == GPIO.LOW:
                    GPIO.setup(col_pin, GPIO.IN)
                    return config.KEYS[row_index][col_index]

            GPIO.setup(col_pin, GPIO.IN)

        return None

    def get_key(self):
        raw = self._scan_raw()
        now = time.monotonic()

        if raw != self._last_raw:
            self._last_raw = raw
            self._changed_at = now
            return None

        if now - self._changed_at < self.debounce_s:
            return None

        if raw != self._stable_key:
            self._stable_key = raw
            return raw

        return None


class UltrasonicSensor:
    def __init__(self) -> None:
        GPIO.setup(config.TRIG_PIN, GPIO.OUT, initial=GPIO.LOW)
        GPIO.setup(config.ECHO_PIN, GPIO.IN)
        time.sleep(0.05)

    def medir_distancia_cm(self):
        GPIO.output(config.TRIG_PIN, GPIO.LOW)
        time.sleep(0.000002)

        GPIO.output(config.TRIG_PIN, GPIO.HIGH)
        time.sleep(0.00001)
        GPIO.output(config.TRIG_PIN, GPIO.LOW)

        inicio_espera = time.monotonic()

        while GPIO.input(config.ECHO_PIN) == GPIO.LOW:
            if time.monotonic() - inicio_espera > config.SENSOR_TIMEOUT_S:
                return None

        inicio_pulso = time.monotonic()

        while GPIO.input(config.ECHO_PIN) == GPIO.HIGH:
            if time.monotonic() - inicio_pulso > config.SENSOR_TIMEOUT_S:
                return None

        fim_pulso = time.monotonic()
        duracao = fim_pulso - inicio_pulso

        return duracao * 34300.0 / 2.0

    @staticmethod
    def estado_porta(distancia_cm):
        if distancia_cm is None:
            return "SEM LEITURA"

        if distancia_cm <= config.LIMIAR_FECHADO_CM:
            return "FECHADA"

        return "ABERTA"


@dataclass
class BuzzerState:
    ligado: bool = False
    fim: float = 0.0


class ActiveBuzzer:
    def __init__(self) -> None:
        GPIO.setup(config.BUZZER_PIN, GPIO.OUT, initial=GPIO.LOW)
        self.state = BuzzerState()

    def tocar(self, duracao_s: float) -> None:
        GPIO.output(config.BUZZER_PIN, GPIO.HIGH)
        self.state.ligado = True
        self.state.fim = time.monotonic() + duracao_s

    def atualizar(self) -> None:
        if self.state.ligado and time.monotonic() >= self.state.fim:
            self.desligar()

    def desligar(self) -> None:
        GPIO.output(config.BUZZER_PIN, GPIO.LOW)
        self.state.ligado = False


class LCD1602:
    LCD_BACKLIGHT = 0x08
    ENABLE = 0x04
    MODE_COMMAND = 0x00
    MODE_DATA = 0x01

    def __init__(self) -> None:
        self.bus = SMBus(config.I2C_BUS)
        self.address = self._detectar_endereco()
        self._inicializar()

    def _detectar_endereco(self) -> int:
        for address in config.LCD_ADDRESSES:
            try:
                self.bus.write_byte(address, 0)
                return address
            except OSError:
                pass

        raise RuntimeError(
            "LCD não encontrado em 0x27 ou 0x3F. "
            "Execute: i2cdetect -y 1"
        )

    def _escrever(self, value: int) -> None:
        self.bus.write_byte(self.address, value)

    def _pulse_enable(self, data: int) -> None:
        self._escrever(data | self.ENABLE | self.LCD_BACKLIGHT)
        time.sleep(0.000001)
        self._escrever((data & ~self.ENABLE) | self.LCD_BACKLIGHT)
        time.sleep(0.00005)

    def _write_nibble(self, nibble: int, mode: int) -> None:
        data = (nibble & 0xF0) | mode | self.LCD_BACKLIGHT
        self._escrever(data)
        self._pulse_enable(data)

    def _write_byte(self, value: int, mode: int) -> None:
        self._write_nibble(value & 0xF0, mode)
        self._write_nibble((value << 4) & 0xF0, mode)

    def command(self, value: int) -> None:
        self._write_byte(value, self.MODE_COMMAND)

    def data(self, value: int) -> None:
        self._write_byte(value, self.MODE_DATA)

    def _inicializar(self) -> None:
        time.sleep(0.05)
        self._write_nibble(0x30, self.MODE_COMMAND)
        time.sleep(0.0045)
        self._write_nibble(0x30, self.MODE_COMMAND)
        time.sleep(0.00015)
        self._write_nibble(0x30, self.MODE_COMMAND)
        self._write_nibble(0x20, self.MODE_COMMAND)

        self.command(0x28)
        self.command(0x0C)
        self.command(0x06)
        self.command(0x01)
        time.sleep(0.002)

    def set_cursor(self, col: int, row: int) -> None:
        offsets = [0x00, 0x40]
        self.command(0x80 | (col + offsets[row]))

    def print_text(self, text: str) -> None:
        for char in text:
            self.data(ord(char))

    def show(self, line1: str, line2: str) -> None:
        self.set_cursor(0, 0)
        self.print_text(line1[:16].ljust(16))
        self.set_cursor(0, 1)
        self.print_text(line2[:16].ljust(16))

    def close(self) -> None:
        self.bus.close()
