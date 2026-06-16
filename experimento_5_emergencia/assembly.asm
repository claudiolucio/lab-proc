# ============================================================
# Aula 06 - PCS3732 | CPUlator RISC-V RV32
# Demonstração: Interrupção de Hardware (Terminal IRQ 24)
# Fluxo: Loop Principal → Trigger → Context Switch → ISR → Retorno
# ============================================================

.global _start

# --- Endereços de Memory-Mapped I/O ---
.equ TERMINAL_RX,  0xffff0000   # Leitura: dado recebido (limpa IRQ)
.equ TERMINAL_TX,  0xffff000c   # Escrita: enviar dado ao terminal

_start:
    # [PASSO 1] Inicializa Stack Pointer
    li   sp, 0x7FFFFFC

    # [PASSO 2] Aponta mtvec para a ISR (vetor de interrupção)
    la   t0, _isr
    csrw mtvec, t0

    # [PASSO 3] Habilita interrupção externa de máquina
    # mie: bit 11 = MEIE (Machine External Interrupt Enable)
    li   t0, 0x800
    csrw mie, t0

    # [PASSO 4] Habilita interrupções globais
    # mstatus: bit 3 = MIE (Machine Interrupt Enable)
    csrsi mstatus, 0x8

    # [PASSO 5] Escreve mensagem inicial no terminal
    li   t1, TERMINAL_TX
    li   t0, 'S'
    sw   t0, 0(t1)
    li   t0, 'Y'
    sw   t0, 0(t1)
    li   t0, 'S'
    sw   t0, 0(t1)
    li   t0, ':'
    sw   t0, 0(t1)
    li   t0, 'O'
    sw   t0, 0(t1)
    li   t0, 'K'
    sw   t0, 0(t1)

# ============================================================
# LOOP PRINCIPAL (slide 6: "Leitura do LDR, Webserver a 1Hz")
# CPU fica aqui até o botão SOS (Terminal) disparar a IRQ
# ============================================================
main_loop:
    nop
    nop
    nop
    j main_loop

# ============================================================
# ISR - Rotina de Serviço de Interrupção (slide 6, passo 4)
# Prioridade máxima: interrompe o loop principal imediatamente
# ============================================================
.align 4
_isr:
    # --- Context Switch: salva registradores na Stack (passo 3) ---
    addi sp, sp, -20
    sw   ra,  16(sp)
    sw   t0,  12(sp)
    sw   t1,   8(sp)
    sw   a0,   4(sp)
    sw   a1,   0(sp)

    # --- Verifica mcause (bit 31 = 1 → interrupção) ---
    csrr t0, mcause
    srli t1, t0, 31
    beqz t1, _fim_isr

    # --- Lê o caractere do terminal (limpa a interrupção) ---
    li   t0, TERMINAL_RX
    lw   a0, 0(t0)

    # --- Ecoa o caractere + envia "!SOS" ao terminal ---
    li   t1, TERMINAL_TX
    sw   a0, 0(t1)          # ecoa o caractere recebido
    li   a1, '!'
    sw   a1, 0(t1)
    li   a1, 'S'
    sw   a1, 0(t1)
    li   a1, 'O'
    sw   a1, 0(t1)
    li   a1, 'S'
    sw   a1, 0(t1)

_fim_isr:
    # --- Restaura contexto: pop da Stack (passo 5) ---
    lw   ra,  16(sp)
    lw   t0,  12(sp)
    lw   t1,   8(sp)
    lw   a0,   4(sp)
    lw   a1,   0(sp)
    addi sp, sp, 20

    # --- MRET: retorna ao main_loop (passo 6) ---
    mret
	
	