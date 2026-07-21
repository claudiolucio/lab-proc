# Configuração centralizada
# Numeração BCM, exatamente como nos códigos Freenove fornecidos.

# Teclado matricial 4x4
KEYS = [
    ["1", "2", "3", "A"],
    ["4", "5", "6", "B"],
    ["7", "8", "9", "C"],
    ["*", "0", "#", "D"],
]

ROW_PINS = [16, 20, 21, 26]
COL_PINS = [19, 13, 6, 5]

# HC-SR04
TRIG_PIN = 14
ECHO_PIN = 15
MAX_DISTANCE_CM = 220
SENSOR_TIMEOUT_S = 0.03
LIMIAR_FECHADO_CM = 8.0
PERIODO_SENSOR_S = 0.5

# Buzzer ativo
BUZZER_PIN = 12

# LCD 16x2 via PCF8574
I2C_BUS = 1
LCD_ADDRESSES = [0x27, 0x3F]

# Aplicação
SENHA_CORRETA = "1234"
MAX_ERROS = 3
TEMPO_BLOQUEIO_S = 10
