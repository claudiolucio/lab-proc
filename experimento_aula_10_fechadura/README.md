# Fechadura Eletrônica — Controle Isolado dos Componentes (PCS3732 · Aula 10)

Código C++ para **Raspberry Pi 3** (SoC Broadcom BCM2837) que implementa o
controle/interface **isolado** de cada componente do sistema, seguindo a
**Regra de Ouro**: *nunca integre um componente que não passou em seu próprio
teste unitário.*

Cada arquivo é um programa autônomo que valida um único periférico antes de
qualquer integração.

## Componentes e requisitos cobertos

| Programa            | Componente            | Interface        | Requisito |
|---------------------|-----------------------|------------------|-----------|
| `teste_teclado`     | Teclado matricial 4x4 | GPIO (varredura) | RF1       |
| `teste_lcd_i2c`     | Display LCD 16x2      | I2C (SDA/SCL)    | RF2       |
| `teste_sensor`      | Sensor HC-SR04        | GPIO             | RF3       |
| `teste_buzzer`      | Buzzer                | GPIO / PWM       | Feedback  |

## Dependências

- Raspberry Pi OS com I2C habilitado (`sudo raspi-config` → Interface → I2C).
- Biblioteca **pigpio**:

```bash
sudo apt-get update
sudo apt-get install pigpio libpigpio-dev i2c-tools
```

## Compilação

```bash
make            # compila todos os testes em ./build
make clean      # remove binários
```

## Execução

O pigpio exige acesso a `/dev/mem`, portanto rode com `sudo`:

```bash
sudo ./build/teste_teclado
sudo ./build/teste_lcd_i2c
sudo ./build/teste_sensor
sudo ./build/teste_buzzer
```

## Validação isolada de cada componente

**Teclado (RF1)** — cada toque deve gerar **um único** evento (debounce por
pull-up interno + confirmação temporal). Se aparecerem eventos duplicados,
aumente `DEBOUNCE_MS`.

**LCD via I2C (RF2)** — antes de rodar, confirme o endereço do display:

```bash
i2cdetect -y 1      # deve mostrar 0x27 (ou 0x3F) na grade
```

Ajuste `LCD_ADDR` no código conforme o endereço detectado. O teste deve
imprimir `Hello World` / `STATUS: OK` nas duas linhas.

**Sensor (RF3)** — a leitura deve alternar entre `ABERTA` e `FECHADA` conforme
o obstáculo. **Atenção elétrica:** o pino ECHO do HC-SR04 é de 5V; use um
divisor resistivo para 3,3V antes do GPIO.

**Buzzer** — deve emitir bipe curto (sucesso) e bipe longo (falha) audíveis.

## Mapeamento de pinos (numeração BCM — ajuste à sua fiação)

| Sinal            | GPIO (BCM)          |
|------------------|---------------------|
| Teclado linhas   | 5, 6, 13, 19        |
| Teclado colunas  | 12, 16, 20, 21      |
| LCD I2C          | SDA=2, SCL=3        |
| Sensor TRIG/ECHO | 23 / 24 (ECHO ~3,3V)|
| Buzzer           | 18 (PWM)            |

## Nota sobre não-bloqueio

Nos testes isolados o uso de `gpioDelay` é aceitável. Na **integração**, a
temporização do buzzer e as esperas devem ser gerenciadas por uma **máquina de
estados não-bloqueante**, evitando `sleep()`/delays longos que congelariam a
varredura do teclado ou a leitura do sensor.
