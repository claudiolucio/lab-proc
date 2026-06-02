# Experimento 3 - Multiplicacao e fatorial no ESP32

Este experimento estende a calculadora binaria de 4 bits para executar as operacoes de multiplicacao e fatorial diretamente no ESP32. A pagina web embarcada serve apenas como interface: ela valida se os campos possuem 4 bits, envia os parametros para a rota `/calc` e exibe o JSON retornado pelo microcontrolador.

## Implementacao no microcontrolador

As operacoes aritmeticas foram implementadas no arquivo `code/calculator_2_multiplication.ino`, em C/C++ para Arduino/ESP32. O Javascript da pagina nao calcula multiplicacao nem fatorial; ele apenas chama:

```javascript
const response = await fetch("/calc?" + params.toString());
const data = await response.json();
```

A multiplicacao converte os operandos de 4 bits em valores com sinal usando complemento de dois e executa o produto no ESP32:

```cpp
int16_t multiply4(uint8_t a, uint8_t b) {
  return (int16_t)signed4(a) * (int16_t)signed4(b);
}
```

No tratamento da requisicao HTTP, a opcao `mul` chama essa funcao C, grava nos LEDs os 4 bits menos significativos do resultado e sinaliza overflow quando o valor exato fica fora da faixa de 4 bits com sinal (-8 a +7):

```cpp
} else if (op == "mul") {
  exactResult = multiply4(a, b);
  result = exactResult & 0x0F;
  overflow = !fitsSigned4(exactResult);
  operationText = "A x B";
}
```

O fatorial tambem foi implementado no ESP32. Como a entrada esta em complemento de dois, a operacao aceita apenas valores nao negativos de A:

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

Na rota `/calc`, a opcao `fact` ignora o operando B, calcula `A!` no microcontrolador, escreve o resultado truncado nos LEDs e informa overflow quando o resultado exato nao cabe em 4 bits com sinal:

```cpp
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
```

## Interface web

A interface HTML/JavaScript possui as novas opcoes:

```html
<option value="mul">MULT: A x B</option>
<option value="fact">FAT: A!</option>
```

Essas opcoes sao apenas comandos enviados ao ESP32. O resultado exibido na pagina vem da resposta JSON montada no microcontrolador, incluindo o resultado exato, o valor de 4 bits enviado aos LEDs e a indicacao de overflow.
