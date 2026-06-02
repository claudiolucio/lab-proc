# Experimento 3 - Multiplicacao e fatorial no ESP32

Este experimento reproduz uma calculadora binaria de 4 bits hospedada no ESP32. A interface web serve apenas para enviar operandos e mostrar a resposta; as operacoes de multiplicacao, fatorial e a medida de tempo sao executadas no microcontrolador, no arquivo `code/calculator_2_multiplication.ino`.

## Materiais necessarios

- ESP32 com suporte ao ambiente Arduino.
- Cabo USB para gravar e alimentar a placa.
- Arduino IDE ou `arduino-cli` com o pacote de placas ESP32 instalado.
- 4 LEDs.
- 4 resistores para limitar corrente dos LEDs.
- Jumpers e protoboard.
- Computador ou celular com Wi-Fi para acessar a pagina servida pelo ESP32.

## Ligacao dos LEDs

O codigo usa quatro saidas digitais para mostrar os 4 bits menos significativos do resultado:

```cpp
const int LED_PINS[4] = {6, 7, 8, 9};
```

Ligue cada GPIO a um LED em serie com resistor. O bit menos significativo fica em `GPIO 6`, e o bit mais significativo fica em `GPIO 9`.

| Bit | GPIO |
| --- | --- |
| 0 | 6 |
| 1 | 7 |
| 2 | 8 |
| 3 | 9 |

Se a sua placa ESP32 nao expuser esses GPIOs ou se eles forem reservados no modelo usado, altere o vetor `LED_PINS` no codigo antes de gravar.

## Como gravar o codigo

1. Abra o arquivo `experimento3_multiplicacao_fatorial/code/calculator_2_multiplication.ino` na Arduino IDE.
2. Selecione a placa ESP32 correta no menu de placas.
3. Selecione a porta serial da placa.
4. Compile e envie o codigo para o ESP32.
5. Abra o Monitor Serial em `115200` baud.

Ao iniciar, o ESP32 deve imprimir mensagens como:

```text
Calculadora 4 bits iniciada.
SSID: ESP32_CALCULADORA
Senha: 12345678
IP: 192.168.4.1
Servidor HTTP iniciado.
```

## Como acessar a calculadora

1. No computador ou celular, conecte-se a rede Wi-Fi criada pelo ESP32:
   - SSID: `ESP32_CALCULADORA`
   - Senha: `12345678`
2. Abra o navegador em `http://192.168.4.1/`.
3. Digite os operandos `A` e `B` em binario com exatamente 4 bits.
4. Escolha a operacao desejada.
5. Clique em `Calcular`.

## Representacao dos numeros

Os operandos usam complemento de dois com 4 bits. A faixa representavel diretamente nos LEDs e de `-8` a `+7`.

Exemplos:

| Binario | Decimal |
| --- | --- |
| `0000` | 0 |
| `0011` | 3 |
| `0111` | 7 |
| `1000` | -8 |
| `1111` | -1 |

Quando o resultado exato nao cabe nessa faixa, a pagina indica overflow. Mesmo nesses casos, os LEDs mostram os 4 bits menos significativos do resultado.

## Operacoes disponiveis

- `SOMA: A + B`
- `SUB: A - B`
- `MULT: A x B`
- `FAT: A!`

Na operacao de fatorial, apenas o operando `A` e usado. O operando `B` e ignorado. Como `A` esta em complemento de dois, o fatorial aceita apenas valores nao negativos.

## Casos de teste sugeridos

| Operacao | A | B | Resultado esperado |
| --- | --- | --- | --- |
| Multiplicacao | `0011` | `0010` | exato `6`, LEDs `0110`, sem overflow |
| Multiplicacao | `0111` | `0010` | exato `14`, LEDs `1110`, com overflow |
| Multiplicacao | `1111` | `0011` | exato `-3`, LEDs `1101`, sem overflow |
| Fatorial | `0011` | qualquer | exato `6`, LEDs `0110`, sem overflow |
| Fatorial | `0100` | qualquer | exato `24`, LEDs `1000`, com overflow |
| Fatorial | `1111` | qualquer | erro, pois `A = -1` |

## Como registrar o tempo de execucao

A pagina mostra o campo:

```text
Tempo de calculo = ... us
```

O Monitor Serial tambem imprime:

```text
Tempo de calculo: ... us
```

Esse tempo e medido dentro do ESP32 com `micros()`, antes e depois do bloco de aritmetica. Ele nao inclui o tempo de requisicao HTTP, a renderizacao da pagina ou a escrita nos LEDs.

Trecho usado no codigo:

```cpp
// Aritmetica implementada no ESP32, nao no Javascript.
uint32_t startMicros = micros();

if (op == "mul") {
  exactResult = multiply4(a, b);
  result = exactResult & 0x0F;
  overflow = !fitsSigned4(exactResult);
  operationText = "A x B";
} else if (op == "fact") {
  uint16_t factorialResult;

  if (!factorial4(a, factorialResult)) {
    server.send(400, "application/json", "{\"error\":\"Fatorial aceita apenas A entre 0 e 7\"}");
    return;
  }

  usesB = false;
  exactResult = factorialResult;
  result = exactResult & 0x0F;
  overflow = !fitsSigned4(exactResult);
  operationText = "A!";
}

uint32_t elapsedMicros = micros() - startMicros;
```

## Evidencia de que o calculo roda no ESP32

O Javascript da pagina nao implementa multiplicacao nem fatorial. Ele apenas envia os parametros para a rota `/calc`:

```javascript
const response = await fetch("/calc?" + params.toString());
const data = await response.json();
```

A multiplicacao esta implementada em C/C++ no `.ino`:

```cpp
int16_t multiply4(uint8_t a, uint8_t b) {
  return (int16_t)signed4(a) * (int16_t)signed4(b);
}
```

O fatorial tambem esta implementado em C/C++ no `.ino`:

```cpp
bool factorial4(uint8_t a, uint16_t &exactResult) {
  int8_t value = signed4(a);

  if (value < 0) {
    return false;
  }

  exactResult = 1;

  for (int8_t i = 2; i <= value; i++) {
    exactResult *= i;
  }

  return true;
}
```

O JSON retornado para a pagina e montado pelo ESP32, incluindo o resultado exato, o valor exibido nos LEDs, o overflow e o tempo medido:

```cpp
json += "\"exactResult\":" + String(exactResult) + ",";
json += "\"elapsedMicros\":" + String(elapsedMicros) + ",";
json += "\"overflow\":" + String(overflow ? "true" : "false");
```
