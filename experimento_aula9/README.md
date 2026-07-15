# Metrônomo em C++ — Raspberry Pi 3 e Freenove FNK0054

Projeto em C++17. Os botões usam `wiringPi`, seguindo os exemplos do kit Freenove; os atuadores permanecem na implementação original.

## Compilação rápida

```bash
sudo apt update
sudo apt install g++ make wiringpi
make
```

Execute os programas que acessam GPIO com:

```bash
sudo ./bin/01_led_pwm
sudo ./bin/metronome
```

O teste de temporização não acessa GPIO:

```bash
./bin/07_timing
```

Leia `PASSO_A_PASSO_LABORATORIO.md` antes da montagem.


## Alteração dos botões

Os botões agora são lidos por polling com `digitalRead()`, `PUD_UP` e numeração BCM, seguindo o exemplo `ButtonLED.c` da Freenove.

- solto: `HIGH`;
- pressionado: `LOW`;
- ligação: GPIO → botão → GND;
- debounce: espera pela soltura mais 30 ms.
