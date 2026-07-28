# Fuga para o Castelo

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Release](https://img.shields.io/badge/release-v0.1.0-blue)
![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-green)

**Fuga para o Castelo** é um jogo de plataforma 2D no qual um príncipe precisa atravessar
um cenário repleto de obstáculos para alcançar o castelo. Ao longo do caminho, o jogador
deve pular pedras e árvores e atravessar **portais** que o transportam para um mundo de
perguntas de conhecimentos gerais, onde acertar a resposta é essencial para continuar a
jornada com todas as vidas.

---

## Descrição do Jogo

O personagem principal é um **príncipe** que corre por um cenário lateral rumo ao castelo.
Durante o percurso ele enfrenta dois tipos de desafios:

1. **Obstáculos físicos** — pedras e árvores que precisam ser saltadas. A colisão com um
   obstáculo faz o jogador perder uma vida.
2. **Portais de conhecimento** — existem **3 portais** ao longo de cada nível. Ao entrar em
   um portal, o jogador é transportado para um mundo de perguntas e recebe uma questão de
   **múltipla escolha (alternativas de "a" a "d")** sobre conhecimentos gerais. Acertar
   permite retornar ao cenário e prosseguir; **errar faz o jogador perder uma vida**.

O objetivo é chegar ao castelo com pelo menos uma vida restante.

---

## Documentação

O relatório da Semana 1 está organizado em Markdown, seguindo o template de planejamento
da disciplina:

| Seção | Arquivo |
|-------|---------|
| 1. Identificação do Grupo | [`docs/01-identificacao.md`](docs/01-identificacao.md) |
| 2. Leitura Pré-Aula (e Motivação) | [`docs/02-leitura-pre-aula.md`](docs/02-leitura-pre-aula.md) |
| 3. Especificação de Requisitos e Testes | [`docs/03-requisitos-e-testes.md`](docs/03-requisitos-e-testes.md) |
| 4. Arquitetura do Sistema | [`docs/04-arquitetura.md`](docs/04-arquitetura.md) |
| 5. Método Experimental | [`docs/05-metodo-experimental.md`](docs/05-metodo-experimental.md) |
| 6. Lições Aprendidas | [`docs/06-licoes-aprendidas.md`](docs/06-licoes-aprendidas.md) |

O PDF compilado desses arquivos, entregue no Moodle, está em
[`docs/relatorio-semana-01.pdf`](docs/relatorio-semana-01.pdf).

---

## Organização do Repositório

```
fuga-para-o-castelo/
├── README.md                       # Visão geral do projeto
├── LICENSE                         # Licença MIT
├── CHANGELOG.md                    # Histórico de versões
├── .gitignore
├── docs/
│   ├── 01-identificacao.md
│   ├── 02-leitura-pre-aula.md
│   ├── 03-requisitos-e-testes.md
│   ├── 04-arquitetura.md
│   ├── 05-metodo-experimental.md
│   ├── 06-licoes-aprendidas.md
│   ├── relatorio-semana-01.pdf     # Compilado para o Moodle
│   └── img/                        # Figuras do relatório
├── src/                            # Código-fonte (em desenvolvimento)
└── assets/                         # Sprites e recursos (em desenvolvimento)
```

---

## Como Executar

> O jogo ainda está em fase de planejamento. As instruções de execução serão adicionadas
> junto ao primeiro protótipo jogável.

```bash
git clone https://github.com/SEU-USUARIO/fuga-para-o-castelo.git
cd fuga-para-o-castelo
```

---

## Equipe

| Nome | NUSP | Frente de trabalho (proposta) |
|------|------|-------------------------------|
| Solano Omar Oliveira do Nascimento | 14608017 | Módulo de Cenário · Módulo de Física e Colisão |
| Lays Vieira Zandomingos | 14587999 | Módulo de Portais · Módulo de Perguntas · Banco de Perguntas |
| Claudio Lucio Cunha da Silva | 14565003 | Gerenciador de Estados · Gerenciador de Vidas · Interface/HUD |

> A divisão de frentes segue os blocos da arquitetura (Seção 4) e será revista a cada
> reunião semanal de sincronização.

---

## Licença

Distribuído sob a licença MIT. Veja [`LICENSE`](LICENSE) para mais informações.

_Projeto acadêmico — entrega da Semana 1 (Release v0.1.0)._
