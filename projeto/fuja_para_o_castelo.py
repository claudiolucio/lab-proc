import pygame
import sys
import math

pygame.init()

# Janela
LARGURA = 900
ALTURA = 450
tela = pygame.display.set_mode((LARGURA, ALTURA))
pygame.display.set_caption("Fuja para o Castelo")

clock = pygame.time.Clock()

# Cores
AZUL_CEU = (120, 190, 255)
VERDE = (80, 200, 100)
PRETO = (0, 0, 0)
VERMELHO = (220, 50, 50)
MARROM = (120, 70, 30)
CINZA = (130, 130, 130)
CINZA_CLARO = (170, 170, 170)
CINZA_ESCURO = (90, 90, 90)
AMARELO = (255, 220, 60)
AZUL = (60, 90, 220)
ROXO = (120, 60, 180)
PELE = (240, 190, 140)

# Fonte
fonte = pygame.font.SysFont(None, 34)

# Chão
CHAO_Y = 360

# Personagem
principe = pygame.Rect(80, CHAO_Y - 70, 40, 70)
altura_normal = 70
altura_abaixado = 40
velocidade_y = 0
gravidade = 1
pulando = False
abaixado = False

# Obstáculos mais distantes entre si
obstaculos = [
    {"rect": pygame.Rect(400, CHAO_Y - 40, 45, 40), "tipo": "baixo"},
    {"rect": pygame.Rect(700, CHAO_Y - 110, 45, 70), "tipo": "alto"},
    {"rect": pygame.Rect(1000, CHAO_Y - 45, 50, 45), "tipo": "baixo"},
    {"rect": pygame.Rect(1300, CHAO_Y - 120, 50, 80), "tipo": "alto"},
    {"rect": pygame.Rect(1600, CHAO_Y - 50, 55, 50), "tipo": "baixo"},
]

# Castelo final
castelo_x = 2000
castelo_y = CHAO_Y - 210
porta_castelo = pygame.Rect(castelo_x + 95, CHAO_Y - 100, 60, 100)

# Tiros
tiros = []
velocidade_tiro = 9

# Controle
velocidade_cenario = 4
jogo_ativo = True
venceu = False


def desenhar_texto(texto, x, y):
    imagem = fonte.render(texto, True, PRETO)
    tela.blit(imagem, (x, y))


def desenhar_principe(rect, esta_abaixado):
    pele = (240, 190, 140)
    cabelo = (80, 45, 20)
    roupa = (45, 90, 220)
    calca = (30, 40, 120)
    bota = (40, 25, 15)

    if esta_abaixado:
        corpo = pygame.Rect(rect.x + 6, rect.y + 12, 32, 28)
        cabeca = pygame.Rect(rect.x + 10, rect.y - 10, 24, 24)

        pygame.draw.polygon(tela, VERMELHO, [
            (rect.x + 8, rect.y + 18),
            (rect.x - 18, rect.y + 36),
            (rect.x + 8, rect.y + 40)
        ])

        pygame.draw.ellipse(tela, pele, cabeca)
        pygame.draw.arc(tela, cabelo, (cabeca.x, cabeca.y - 4, 24, 18), math.pi, 2 * math.pi, 4)
        pygame.draw.circle(tela, PRETO, (rect.x + 28, rect.y + 1), 2)

        pygame.draw.polygon(tela, AMARELO, [
            (rect.x + 8, rect.y - 8),
            (rect.x + 14, rect.y - 22),
            (rect.x + 20, rect.y - 8),
            (rect.x + 26, rect.y - 22),
            (rect.x + 34, rect.y - 8)
        ])

        pygame.draw.ellipse(tela, roupa, corpo)
        pygame.draw.line(tela, pele, (rect.x + 35, rect.y + 25), (rect.x + 48, rect.y + 35), 5)

        pygame.draw.line(tela, calca, (rect.x + 14, rect.y + 38), (rect.x + 2, rect.y + 45), 6)
        pygame.draw.line(tela, calca, (rect.x + 28, rect.y + 38), (rect.x + 42, rect.y + 45), 6)

        pygame.draw.ellipse(tela, bota, (rect.x - 3, rect.y + 42, 15, 7))
        pygame.draw.ellipse(tela, bota, (rect.x + 36, rect.y + 42, 15, 7))

    else:
        corpo = pygame.Rect(rect.x + 8, rect.y + 22, 28, 38)
        cabeca = pygame.Rect(rect.x + 8, rect.y - 10, 26, 26)

        pygame.draw.polygon(tela, VERMELHO, [
            (rect.x + 8, rect.y + 25),
            (rect.x - 25, rect.y + 70),
            (rect.x + 8, rect.y + 68)
        ])

        pygame.draw.ellipse(tela, pele, cabeca)
        pygame.draw.arc(tela, cabelo, (cabeca.x, cabeca.y - 4, 26, 18), math.pi, 2 * math.pi, 4)
        pygame.draw.circle(tela, PRETO, (rect.x + 27, rect.y + 2), 2)

        pygame.draw.polygon(tela, AMARELO, [
            (rect.x + 6, rect.y - 10),
            (rect.x + 12, rect.y - 28),
            (rect.x + 20, rect.y - 10),
            (rect.x + 28, rect.y - 28),
            (rect.x + 36, rect.y - 10)
        ])

        pygame.draw.ellipse(tela, roupa, corpo)

        pygame.draw.line(tela, pele, (rect.x + 10, rect.y + 32), (rect.x - 5, rect.y + 45), 5)
        pygame.draw.line(tela, pele, (rect.x + 34, rect.y + 32), (rect.x + 50, rect.y + 40), 5)

        pygame.draw.line(tela, calca, (rect.x + 17, rect.y + 58), (rect.x + 10, rect.y + 72), 7)
        pygame.draw.line(tela, calca, (rect.x + 29, rect.y + 58), (rect.x + 38, rect.y + 72), 7)

        pygame.draw.ellipse(tela, bota, (rect.x + 4, rect.y + 68, 18, 8))
        pygame.draw.ellipse(tela, bota, (rect.x + 32, rect.y + 68, 18, 8))


def desenhar_obstaculo(obstaculo):
    rect = obstaculo["rect"]

    if obstaculo["tipo"] == "baixo":
        pygame.draw.rect(tela, MARROM, rect)
        pygame.draw.circle(tela, MARROM, (rect.x + 15, rect.y), 18)
        pygame.draw.circle(tela, MARROM, (rect.x + 35, rect.y + 5), 15)
        pygame.draw.circle(tela, CINZA_ESCURO, (rect.x + 20, rect.y + 20), 4)
    else:
        pygame.draw.rect(tela, ROXO, rect)
        pygame.draw.rect(tela, PRETO, (rect.x, rect.y, rect.width, 8))
        pygame.draw.line(tela, PRETO, (rect.x, rect.y), (rect.x + rect.width, rect.y + rect.height), 2)


def desenhar_pedra(x, y, largura, altura):
    pygame.draw.rect(tela, CINZA_ESCURO, (x, y, largura, altura), 1)


def desenhar_janela_arco(x, y, largura, altura):
    pygame.draw.rect(tela, PRETO, (x, y + altura // 3, largura, altura * 2 // 3))
    pygame.draw.ellipse(tela, PRETO, (x, y, largura, altura // 2))


def desenhar_porta_arco(x, y, largura, altura):
    pygame.draw.rect(tela, MARROM, (x, y + altura // 4, largura, altura * 3 // 4))
    pygame.draw.ellipse(tela, MARROM, (x, y, largura, altura // 2))
    pygame.draw.rect(tela, PRETO, (x, y + altura // 4, largura, altura * 3 // 4), 3)
    pygame.draw.arc(tela, PRETO, (x, y, largura, altura // 2), math.pi, 2 * math.pi, 3)
    pygame.draw.line(tela, PRETO, (x + largura // 2, y + altura // 4), (x + largura // 2, y + altura), 2)
    pygame.draw.circle(tela, AMARELO, (x + largura // 2 + 12, y + altura // 2 + 10), 3)


def desenhar_muralha_com_dentes(x, y, largura, altura):
    pygame.draw.rect(tela, CINZA, (x, y, largura, altura))

    dente_largura = 18
    dente_altura = 18

    for i in range(0, largura, dente_largura * 2):
        pygame.draw.rect(tela, CINZA_CLARO, (x + i, y - dente_altura, dente_largura, dente_altura))

    # Linhas de pedra
    for linha in range(y + 15, y + altura, 22):
        pygame.draw.line(tela, CINZA_ESCURO, (x, linha), (x + largura, linha), 1)

    for px in range(x + 10, x + largura, 35):
        for py in range(y + 10, y + altura, 44):
            desenhar_pedra(px, py, 20, 12)


def desenhar_torre(x, y, largura, altura):
    pygame.draw.rect(tela, CINZA, (x, y, largura, altura))

    # Dentes no topo da torre
    dente_largura = largura // 4
    for i in range(4):
        if i % 2 == 0:
            pygame.draw.rect(tela, CINZA_CLARO, (x + i * dente_largura, y - 18, dente_largura, 18))

    # Telhado pontudo
    pygame.draw.polygon(tela, VERMELHO, [
        (x - 8, y - 18),
        (x + largura // 2, y - 65),
        (x + largura + 8, y - 18)
    ])

    # Bandeira
    pygame.draw.line(tela, PRETO, (x + largura // 2, y - 65), (x + largura // 2, y - 95), 3)
    pygame.draw.polygon(tela, AMARELO, [
        (x + largura // 2, y - 95),
        (x + largura // 2 + 35, y - 82),
        (x + largura // 2, y - 70)
    ])

    # Janela em arco
    desenhar_janela_arco(x + largura // 2 - 10, y + 35, 20, 35)

    # Pedras
    for linha in range(y + 15, y + altura, 25):
        pygame.draw.line(tela, CINZA_ESCURO, (x, linha), (x + largura, linha), 1)

    for px in range(x + 8, x + largura - 10, 28):
        for py in range(y + 10, y + altura, 50):
            desenhar_pedra(px, py, 16, 10)


def desenhar_castelo(x, y):
    # Sombra/base
    pygame.draw.ellipse(tela, (70, 70, 70), (x - 45, CHAO_Y - 10, 330, 25))

    # Torres laterais
    desenhar_torre(x - 35, y + 40, 60, 170)
    desenhar_torre(x + 225, y + 40, 60, 170)

    # Torre central
    desenhar_torre(x + 88, y, 75, 210)

    # Corpo do castelo
    desenhar_muralha_com_dentes(x + 15, y + 95, 235, 115)

    # Porta principal em arco
    desenhar_porta_arco(x + 95, y + 110, 60, 100)

    # Janelas do corpo principal
    desenhar_janela_arco(x + 45, y + 125, 25, 40)
    desenhar_janela_arco(x + 180, y + 125, 25, 40)

    # Contorno externo
    pygame.draw.rect(tela, CINZA_ESCURO, (x + 15, y + 95, 235, 115), 2)
    pygame.draw.rect(tela, CINZA_ESCURO, (x - 35, y + 40, 60, 170), 2)
    pygame.draw.rect(tela, CINZA_ESCURO, (x + 225, y + 40, 60, 170), 2)
    pygame.draw.rect(tela, CINZA_ESCURO, (x + 88, y, 75, 210), 2)


def reiniciar_jogo():
    global principe, velocidade_y, pulando, abaixado
    global obstaculos, tiros, castelo_x, porta_castelo
    global jogo_ativo, venceu

    principe = pygame.Rect(80, CHAO_Y - 70, 40, 70)
    velocidade_y = 0
    pulando = False
    abaixado = False

    obstaculos = [
        {"rect": pygame.Rect(400, CHAO_Y - 40, 45, 40), "tipo": "baixo"},
        {"rect": pygame.Rect(700, CHAO_Y - 110, 45, 70), "tipo": "alto"},
        {"rect": pygame.Rect(1000, CHAO_Y - 45, 50, 45), "tipo": "baixo"},
        {"rect": pygame.Rect(1300, CHAO_Y - 120, 50, 80), "tipo": "alto"},
        {"rect": pygame.Rect(1600, CHAO_Y - 50, 55, 50), "tipo": "baixo"},
    ]

    tiros = []
    castelo_x = 2000
    porta_castelo = pygame.Rect(castelo_x + 95, CHAO_Y - 100, 60, 100)

    jogo_ativo = True
    venceu = False


while True:
    for evento in pygame.event.get():
        if evento.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

        if evento.type == pygame.KEYDOWN:
            if evento.key == pygame.K_UP and not pulando and jogo_ativo and not abaixado:
                velocidade_y = -18
                pulando = True

            if evento.key == pygame.K_DOWN and not pulando and jogo_ativo:
                abaixado = True
                principe.height = altura_abaixado
                principe.y = CHAO_Y - altura_abaixado

            if evento.key == pygame.K_SPACE and jogo_ativo:
                tiro = pygame.Rect(principe.x + principe.width, principe.y + 20, 14, 8)
                tiros.append(tiro)

            if evento.key == pygame.K_r:
                reiniciar_jogo()

        if evento.type == pygame.KEYUP:
            if evento.key == pygame.K_DOWN and jogo_ativo:
                abaixado = False
                principe.height = altura_normal
                principe.y = CHAO_Y - altura_normal

    if jogo_ativo:
        # Física do pulo
        principe.y += velocidade_y
        velocidade_y += gravidade

        if principe.y >= CHAO_Y - principe.height:
            principe.y = CHAO_Y - principe.height
            velocidade_y = 0
            pulando = False

        # Movimento do cenário
        for obstaculo in obstaculos:
            obstaculo["rect"].x -= velocidade_cenario

        castelo_x -= velocidade_cenario
        porta_castelo.x = castelo_x + 95

        # Movimento dos tiros
        for tiro in tiros:
            tiro.x += velocidade_tiro

        # Remove tiros fora da tela
        tiros = [tiro for tiro in tiros if tiro.x < LARGURA]

        # Colisão tiro com obstáculo
        novos_obstaculos = []
        for obstaculo in obstaculos:
            destruido = False

            for tiro in tiros:
                if tiro.colliderect(obstaculo["rect"]):
                    destruido = True
                    if tiro in tiros:
                        tiros.remove(tiro)
                    break

            if not destruido:
                novos_obstaculos.append(obstaculo)

        obstaculos = novos_obstaculos

        # Colisão príncipe com obstáculo
        for obstaculo in obstaculos:
            if principe.colliderect(obstaculo["rect"]):
                jogo_ativo = False
                venceu = False

        # Chegada ao castelo
        if principe.colliderect(porta_castelo):
            jogo_ativo = False
            venceu = True

    # Desenho da tela
    tela.fill(AZUL_CEU)

    # Grama
    pygame.draw.rect(tela, VERDE, (0, CHAO_Y, LARGURA, ALTURA - CHAO_Y))

    # Linha do chão
    pygame.draw.line(tela, PRETO, (0, CHAO_Y), (LARGURA, CHAO_Y), 2)

    # Castelo
    desenhar_castelo(castelo_x, castelo_y)

    # Príncipe
    desenhar_principe(principe, abaixado)

    # Obstáculos
    for obstaculo in obstaculos:
        desenhar_obstaculo(obstaculo)

    # Tiros
    for tiro in tiros:
        pygame.draw.ellipse(tela, AMARELO, tiro)

    # Textos
    desenhar_texto("Fuja para o Castelo", 20, 20)
    desenhar_texto("Seta cima: pular | Seta baixo: abaixar | Espaco: atirar | R: reiniciar", 20, 55)

    if not jogo_ativo:
        if venceu:
            desenhar_texto("Voce chegou ao castelo! Vitoria!", 260, 180)
        else:
            desenhar_texto("O principe bateu no obstaculo! Fim de jogo.", 220, 180)

        desenhar_texto("Pressione R para reiniciar", 300, 220)

    pygame.display.update()
    clock.tick(60)