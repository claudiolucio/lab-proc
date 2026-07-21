# Fechadura eletrônica em Python

Versão Python com integração incremental e pinagem BCM igual à dos
códigos Freenove fornecidos.

## Pinagem BCM

| Componente | Sinal | BCM |
|---|---|---:|
| Teclado | linhas | 16, 20, 21, 26 |
| Teclado | colunas | 19, 13, 6, 5 |
| HC-SR04 | TRIG | 14 |
| HC-SR04 | ECHO | 15 |
| Buzzer ativo | sinal | 18 |
| LCD I2C | SDA | 2 |
| LCD I2C | SCL | 3 |

Todas as configurações estão em `config.py`.

## Dependências

```bash
sudo apt update
sudo apt install python3-pip python3-smbus i2c-tools
python3 -m pip install -r requirements.txt
```

Habilite o I2C:

```bash
sudo raspi-config
```

Verifique o endereço do LCD:

```bash
i2cdetect -y 1
```

## Testes isolados

Execute a partir da raiz do projeto:

```bash
sudo python3 00_testes_isolados/teste_teclado.py
sudo python3 00_testes_isolados/teste_sensor.py
sudo python3 00_testes_isolados/teste_lcd.py
sudo python3 00_testes_isolados/teste_buzzer.py
```

## Testes incrementais

```bash
sudo python3 01_sensor/main.py
sudo python3 02_sensor_teclado/main.py
sudo python3 03_sensor_teclado_lcd/main.py
sudo python3 04_sistema_completo/main.py
```

## Sistema completo

- senha: `1234`;
- `#`: confirmar;
- `*`: limpar;
- `D`: encerrar;
- três erros: bloqueio de 10 segundos.

## Atenção elétrica

O ECHO do HC-SR04 opera em aproximadamente 5 V. Use divisor resistivo
para reduzir o sinal para 3,3 V antes do GPIO BCM 15.
