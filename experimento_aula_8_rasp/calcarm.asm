@ =========================================================================
@ Calculadora 4 bits - ARMv7 (CPUlator: System -> ARM)
@ Operacoes: ADD, SUB, MUL, FAT (fatorial)
@ Operandos de teste definidos em A_VAL / B_VAL (editar para cada caso)
@ Resultados ficam em R4 (ADD), R5 (SUB), R6 (MUL), R7 (FAT)
@ Overflow de ADD/SUB sinalizado em R8 (1 = overflow, 0 = ok)
@ =========================================================================

    .global _start

_start:
    LDR     R0, =A_VAL
    LDR     R0, [R0]          @ R0 = operando A (4 bits, 0-15)
    LDR     R1, =B_VAL
    LDR     R1, [R1]          @ R1 = operando B (4 bits, 0-15)

@ --------------------------------------------------------------------
@ [+] ADD: R4 = R0 + R1, mascarado em 4 bits; overflow se R4_raw > 15
@ --------------------------------------------------------------------
    ADD     R2, R0, R1        @ soma sem mascara
    AND     R4, R2, #0xF      @ resultado mascarado em 4 bits
    CMP     R2, #15
    MOVGT   R8, #1            @ overflow se soma > 15
    MOVLE   R8, #0

@ --------------------------------------------------------------------
@ [-] SUB: R5 = R0 - R1; flag de sinal negativo checada via CMP
@ --------------------------------------------------------------------
    CMP     R0, R1
    SUBGE   R5, R0, R1        @ R0 >= R1: subtracao direta
    SUBLT   R5, R1, R0        @ R0 <  R1: inverte e marca negativo
    MOVLT   R9, #1            @ R9 = 1 indica resultado negativo
    MOVGE   R9, #0

@ --------------------------------------------------------------------
@ [*] MUL: R6 = R0 * R1 via deslocamento de bits (shift) e adicao
@ --------------------------------------------------------------------
    MOV     R6, #0            @ acumulador
    MOV     R3, R1            @ contador = B
mul_loop:
    CMP     R3, #0
    BEQ     mul_done
    ADD     R6, R6, R0        @ acumula A, R1 vezes (shift+add simplificado)
    SUB     R3, R3, #1
    B       mul_loop
mul_done:

@ --------------------------------------------------------------------
@ [!] FAT: R7 = A! (fatorial de A), loop iterativo com monitoramento
@           de overflow (R10 = 1 se R7 ultrapassar 16 bits)
@ --------------------------------------------------------------------
    MOV     R7, #1            @ acumulador do fatorial
    MOV     R3, R0            @ contador = A
    MOV     R10, #0           @ flag de overflow do fatorial
fat_loop:
    CMP     R3, #1
    BLE     fat_done
    MUL     R7, R7, R3
    LDR     R11, =0xFFFF
    CMP     R7, R11
    MOVGT   R10, #1           @ marca overflow se exceder 16 bits
    SUB     R3, R3, #1
    B       fat_loop
fat_done:

stop:
    B       stop              @ loop infinito (padrao de fim do CPUlator)

    .data
    .align 4
A_VAL:  .word 3                @ operando A (alterar para cada caso de teste)
B_VAL:  .word 2                @ operando B (alterar para cada caso de teste)