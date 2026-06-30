# =========================================================================
# Calculadora 4 bits - RISC-V RV32I (Venus / RARS)
# Operacoes: ADD, SUB, MUL, FAT (fatorial)
# Operandos de teste definidos em a_val / b_val (editar para cada caso)
# Resultados ficam em t0 (ADD), t1 (SUB), t2 (MUL), t3 (FAT)
# Overflow de ADD sinalizado em t4 (1 = overflow, 0 = ok)
# Sinal negativo de SUB sinalizado em t5 (1 = negativo, 0 = positivo)
# Overflow do fatorial sinalizado em t6 (1 = excedeu 16 bits)
# =========================================================================

.globl _start

.data
a_val:  .word 3        # operando A (4 bits, 0-15) - alterar por caso de teste
b_val:  .word 2         # operando B (4 bits, 0-15) - alterar por caso de teste

.text
_start:
    la      a0, a_val
    lw      a0, 0(a0)       # a0 = operando A
    la      a1, b_val
    lw      a1, 0(a1)       # a1 = operando B

    # --------------------------------------------------------------
    # [+] ADD: t0 = (a0 + a1) mascarado em 4 bits; t4 = overflow
    # --------------------------------------------------------------
    add     a2, a0, a1          # soma sem mascara
    andi    t0, a2, 0xF         # resultado mascarado em 4 bits
    li      t6, 15
    bgt     a2, t6, add_ovf
    li      t4, 0
    j       add_done
add_ovf:
    li      t4, 1
add_done:

    # --------------------------------------------------------------
    # [-] SUB: t1 = |a0 - a1|; t5 = 1 se resultado seria negativo
    # --------------------------------------------------------------
    blt     a0, a1, sub_neg
    sub     t1, a0, a1
    li      t5, 0
    j       sub_done
sub_neg:
    sub     t1, a1, a0
    li      t5, 1
sub_done:

    # --------------------------------------------------------------
    # [*] MUL: t2 = a0 * a1 via deslocamento de bits (shift) e adicao
    # --------------------------------------------------------------
    li      t2, 0               # acumulador
    mv      t3, a1              # contador = B
mul_loop:
    beqz    t3, mul_done
    add     t2, t2, a0          # acumula A, B vezes (shift+add simplificado)
    addi    t3, t3, -1
    j       mul_loop
mul_done:

    # --------------------------------------------------------------
    # [!] FAT: t3 = A! (fatorial de A), loop iterativo com
    #           monitoramento de overflow (t6 = 1 se exceder 16 bits)
    # --------------------------------------------------------------
    li      t3, 1               # acumulador do fatorial
    mv      s0, a0              # contador = A
    li      t6, 0                # flag de overflow do fatorial
    li      s1, 0xFFFF
fat_loop:
    li      s2, 1
    ble     s0, s2, fat_done
    mul     t3, t3, s0
    ble     t3, s1, fat_skip_ovf
    li      t6, 1
fat_skip_ovf:
    addi    s0, s0, -1
    j       fat_loop
fat_done:

    # --------------------------------------------------------------
    # Encerramento (ecall de saida, padrao Venus/RARS)
    # --------------------------------------------------------------
    li      a7, 10
    ecall