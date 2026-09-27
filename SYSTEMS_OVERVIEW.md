# 🎯 Systems Overview — Brazilian Mafia and Major Heists

## Visão Geral dos Sistemas

Este documento fornece uma visão de alto nível de todos os sistemas principais do jogo e como eles interagem entre si.

---

## 1. SISTEMA DE MUNDO ABERTO (World System)

### Mapa Contínuo (Seamless Streaming)
- Uma única instância de mundo
- Três regiões conectadas por rodovias
- Sem telas de carregamento
- Streaming de assets por zona
- LOD (Level of Detail) dinâmico

### Regiões
```
São Paulo (Financeiro)
    ↓ Rodovia Anchieta
Rio de Janeiro (Territorial)
    ↓ Rodovia BR-116
Campo Grande (Fronteira)
```

### Clima e Tempo
- Ciclo dia/noite de 24 minutos (real)
- Sistemas climáticos (chuva, neblina, poeira)
- Impacto na visibilidade e traction

---

## 2. SISTEMA DE PERSONAGEM (Character System)

### Protagonistas Jogáveis

| Nome | Especialidade | Habilidade | Uso |
|------|---------------|-----------|-----|
| Thiago "Alemão" | Hacker/Logística | Visão Tática | Infiltração, planejamento |
| Marcos "Marcola" | Combate | Adrenalina | Ação direta, violência |
| Roberto "Beto Pampa" | Pilotagem | Fuga Extrema | Perseguição, escape |

### Atributos
- Força
- Precisão
- Resistência
- Inteligência
- Pilotagem
- Furtividade
- Liderança

### Progression
- XP por ação
- Desbloqueio de habilidades
- Aumento de atributos

---

## 3. SISTEMA DE COMBATE (Combat System)

### Tipos de Armas
- Pistolas (precisão, baixo dano)
- Fuzis (semiautomático, médio dano)
- Metralhadoras (automático, alto dano, baixa precisão)
- Shotguns (curta distância, alto dano)
- Explosivos (granadas, satchel charges)
- Melee (facas, bastões)

### Mecânicas
- Cobertura (atrás de objetos)
- Mira livre ou focada (ADS)
- Recuo e precisão progressiva
- Dano em membro específico
- Ragdoll e física

### IA Inimiga
- Comportamento adaptativo
- Comunicação tática
- Flanqueamento
- Retirada estratégica

---

## 4. SISTEMA DE DIREÇÃO (Driving System)

### Tipos de Veículos
- Carros de rua (sedan, SUV)
- Motos (cruiser, sport)
- Vans e furgões
- Caminhões
- Helicopteros (roubáveis)
- Lanchas (opcional)

### Física
- Tração dinâmica
- Suspenção realista
- Dano visual progressivo
- Pegadas de derrapagem
- Colisões com dano

### Customização
- Suspensão (baixa/fixa/ar)
- Cores e wraps
- Pneus
- Escapamentos
- Rodas
- Interiores
- Sistemas de som

---

## 5. SISTEMA DE WANTED / RESPOSTA POLICIAL (Wanted System)

### Níveis de Wanted

```
⭐ Nível 1: Polícia Militar
├─ Abordagem padrão
├─ Opção de suborno
└─ Escape simples

⭐⭐ Nível 2: ROCAM (Motos)
├─ Perseguição agressiva
├─ Bloqueios pontuais
└─ Velocidade alta

⭐⭐⭐ Nível 3: Forças de Choque
├─ ROTA + BAEP
├─ Road spikes
├─ Helicóptero com térmica
└─ Veículos blindados

⭐⭐⭐⭐ Nível 4: Operações Especiais
├─ BOPE / GATE
├─ Caveirões blindados
├─ Snipers em posição
└─ Tática coordenada

⭐⭐⭐⭐⭐ Nível 5: Intervenção Federal
├─ Bloqueio de rodovias
├─ Interceptação aérea (jatos)
├─ Desativação de GPS
└─ Operação em larga escala
```

### Detecção
- Crimes testemunhados
- Câmeras de segurança
- Chamadas de civis
- Proximidade de polícia
- Dano causado

### Evasão
- Tempo reduz wanted
- Sair da zona de alerta
- Matar testemunhas
- Destruir câmeras
- Cobertura (mata, subterrâneo)

---

## 6. SISTEMA DE ECONOMIA (Economy System)

### Tipos de Dinheiro
- **Dinheiro Limpo:** legal, sem suspeita
- **Dinheiro Sujo:** roubado, rastreável
- **Crédito de Facção:** moeda interna

### Fontes de Renda
- Assaltos (alto risco, alto ganho)
- Roubos de carga (médio)
- Contratos de facção (variável)
- Empregos legais (baixo, regular)
- Negócios de fachada

### Gastos
- Customização de veículos
- Compra de armas
- Munição e consumíveis
- Propina/suborno
- Terapia (trauma)
- Tratamento médico

### Lavagem de Dinheiro
- PixClandestino (sistema integrado)
- Lava-jatos
- Adegas
- Restaurantes
- Negócios legais
- Conversão lenta para dinheiro limpo

---

## 7. SISTEMA DE FACÇÃO (Faction System)

### Facções Principais

#### Sindicato Paulista (São Paulo)
- Hierarquia rígida
- Operações financeiras
- Foco: narcotráfico, roubo de cargo
- Líder: Pavilhão (presídio)
- Missões: logísticas, financeiras

#### Coalizão Miliciana (Rio)
- Ex-policiais
- Controle territorial
- Foco: proteção, extorsão
- Líder: Comandante regional
- Missões: violentas, territoriais

#### Barões da Fronteira (Campo Grande)
- Oligarquias rurais
- Operações internacionais
- Foco: agronegócio + tráfico
- Líder: Patrão local
- Missões: transporte, contrabando

### Reputação
- +100 a -100 por facção
- Missões exclusivas em determinados níveis
- Desconto em negócios
- Aliança/traição
- Impacto em disponibilidade de contratos

### Relacionamento entre Facções
- Sindicato e Milícia: rivais
- Milícia e Barões: cooperação
- Sindicato e Barões: negócios
- Conflito territorial dinâmico

---

## 8. SISTEMA DE MISSÃO (Mission System)

### Tipos

#### Missões Principais
- Campanha narrativa
- Operação Anchieta (primeiro grande heist)
- Objetivos obrigatórios
- Progressão de história

#### Missões de Facção
- Contratos exclusivos
- Ganho de reputação
- Acesso a itens/negócios
- Dinâmicas de facção

#### Missões Secundárias
- Roubos de cargo (por ZapCrime)
- Busca-e-resgate
- Assassinatos
- Coleta de items

#### Atividades Livres
- Corridas ilegais
- Parkour/acrobacias
- Desafios de tiro
- Destruição de propriedade

### Estrutura de Missão
```
Receber contrato (via NPC, telefone, ZapCrime)
    ↓
Planejamento (briefing, reconhecimento)
    ↓
Execução (abordagem escolhida)
    ↓
Resultado (sucesso/falha/parcial)
    ↓
Recompensa (dinheiro, reputação, items)
    ↓
Consequências (dinâmica de mundo, facções)
```

---

## 9. SISTEMA DE CUSTOMIZAÇÃO (Customization System)

### Categorias

#### Veículos
- Pintura e wraps
- Rodas e pneus
- Suspensão
- Escapamento e som
- Interiores
- Sistemas (freios, tração)

#### Armas
- Skins e acabamentos
- Miras e attachments
- Silenciadores
- Carregadores estendidos
- Grips customizados

#### Personagem
- Roupas (casual, tática, griffe)
- Cortes de cabelo (nevou, degradê)
- Tatuagens
- Acessórios (relógios, correntes)
- Máscaras e balaclavas

### Oficinas
- **Mecânicas de Quebrada:** veículos customizados
- **Loja de Armas:** weapons e attachments
- **Barbearias de Cria:** personagem visual
- **Loja de Roupas:** casual e tática

---

## 10. SISTEMA DE MULTIPLAYER (Multiplayer System)

### MyBrazil RP (Roleplay)

#### Servidores Dedicados
- Até 1.024 players por mapa
- Persistência de mundo
- Dinâmica de facção PvP
- Economia compartilhada

#### Aplicativos Internos

**ZapCrime**
- Contrato de missões
- Sistema de avisos
- Chat de facção
- Localização de drops

**PixClandestino**
- Lavagem de dinheiro
- Gerenciamento de negócios
- Investimentos
- Lucros automáticos

#### Empregos Legais
- Taxista
- Entregador
- Caminhoneiro
- Segurança

#### Corporação Policial
- Concurso de admissão
- Patrulhas
- Rastreamento de suspeitos
- Salário regular

---

## 11. SISTEMA DE PROGRESSÃO (Progression System)

### Experiência
- Combate: kills, headshots, precisão
- Direção: distância, truques, velocidade
- Missões: conclusão, bonus objectives
- Exploração: discovery, collectibles

### Níveis
- Level 1-100 (personal character)
- Reputação de facção: -100 a +100

### Desbloqueios
- Habilidades especiais (upgrades)
- Modificações de armas
- Acesso a missões
- Propriedades (garagens, casas)
- Negócios

---

## 12. SISTEMA DE IA (AI System)

### NPC Civis
- Rotinas diárias
- Reações a criminalidade
- Chamadas de polícia
- Fuga de perigo

### NPC Criminosos
- Patrulhas de facção
- Comportamento territorial
- Comunicação
- Combate tático

### Polícia
- Patrulhas dinâmicas
- Resposta a crimes
- Técnicas de perseguição
- Busca inteligente

---

## Interação entre Sistemas

```
WORLD
  ├─ CHARACTERS (protagonistas)
  ├─ VEHICLES (direção customizável)
  ├─ FACTIONS (dinâmica territorial)
  ├─ MISSIONS (progressão narrativa)
  │
  ├─ COMBAT SYSTEM
  │  ├─ Wanted escalation
  │  ├─ Police AI
  │  └─ Faction dynamics
  │
  ├─ DRIVING SYSTEM
  │  ├─ Pursuit mechanics
  │  └─ Customization
  │
  ├─ ECONOMY SYSTEM
  │  ├─ Faction progression
  │  ├─ Business ownership
  │  └─ Customization costs
  │
  └─ MULTIPLAYER SYSTEM
     ├─ Shared world
     ├─ Player interactions
     └─ Persistent economy
```

---

**Documento versão 1.0** — Setembro 2026
