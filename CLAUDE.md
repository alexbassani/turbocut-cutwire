# TurboCut — Contexto para Claude Code

> Leia este arquivo antes de qualquer coisa. É o ponto de partida de toda sessão.
> Ele é **índice**, não manual: detalhes vivem em [FORK.md](FORK.md), no upstream
> ([docs/BUILDING.md](docs/BUILDING.md)) e — para sessões do mantenedor — na pasta
> local `_planejamento/` (privada, fora deste repositório público; colaboradores
> externos: contribuam via issues, e sejam bem-vindos).

## O que é o TurboCut

Fork brasileiro e **fino** do [CutWire Drift](https://github.com/CutWire-Studios/Drift)
(editor de vídeo desktop, C++/Qt 6/FFmpeg, GPL-3.0), mantido por Alex Bassani.
O delta é deliberadamente mínimo: marca, tradução pt-BR, correções de plataforma e
empacotamento. **Motor é do upstream; identidade e distribuição são nossas.**

## ⛔ Regra #0 (acima de todas): o delta é sagrado

**NUNCA refatorar o motor, NUNCA criar feature de edição no fork.** Melhoria de
editor vai para o upstream como issue/PR; o fork só carrega marca, tradução,
conteúdo e distribuição. Cada linha a mais aqui é uma linha re-aplicada em TODO
sync, para sempre.

### ⛔ Regra #0.1 — a base é a TAG de release, NUNCA o main do upstream

Bug real (28/08/2026): o 1º build nasceu do `main` do Drift e renderizava a UI com
glifos corrompidos (feature não-lançada de UI scale). O `main` deles é canteiro de
obras; **todo sync parte da última tag `vX.Y.Z`**.

## Regras de ouro (NUNCA violar)

1. **Assets de marca trocam CONTEÚDO, nunca NOME** — `Drift_icon.png`,
   `resources/drift.png`, `resources/windows/drift.ico` seguem com esses nomes
   (delta zero em código/CMake).
2. **AppId do Inno Setup NUNCA muda**: `{DC0EC0C4-090C-4519-A3EF-33E9B201A062}` —
   é o que faz o Setup atualizar in-place (e o card do TurboStudio depende dele
   no registro).
3. **Formato de projeto `.drift` é intocável** — compatibilidade total com o
   upstream, projetos abrem nos dois editores.
4. **Catálogo pt-BR no formato canônico do lupdate**, senão o teste `Translations`
   reprova (2 incidentes reais em 28/08/2026): `numerusform` um por linha
   (multi-linha) e aspas duplas como `&quot;` dentro de `<translation>`.
5. **`sourcelanguage="en_US"`** no cabeçalho do catálogo (o lupdate normaliza).
6. **Versão: os três SEMPRE iguais** — `project(Drift VERSION X.Y.Z)` == `pkgver`
   do PKGBUILD == tag (o release.yml recusa se divergir). A base vem do Drift;
   correções só nossas entre releases deles ganham **patch próprio** (foi o caso
   da `0.4.1`, com o runtime do Visual C++). Quando o Drift lançar `0.5.0`,
   voltamos a acompanhar a base deles.
7. **O fix FreeType do Windows fica** (`src/main.cpp`, `QT_QPA_PLATFORM`
   `windows:fontengine=freetype`): o DirectWrite corrompe glifos de fontes
   embutidas em certas GPUs/escalas. Respeita env já definido.
8. Textos MCP ficam em inglês (contrato de máquina — regra do upstream).

## O delta completo (onde a marca mora)

Ver [FORK.md](FORK.md) — tabela arquivo a arquivo. Resumo: `src/main.cpp` (nomes,
FreeType, splash `TurboSplash`), `src/qml/Main.qml` (título),
`installer/windows/Drift.iss`, `CMakeLists.txt` (feed de update → este repo),
`.github/workflows/` (artefatos TurboCut-* + `build-windows.yml` manual),
`i18n/drift_pt_BR.ts`, `packaging/arch/PKGBUILD` (fonte = este repo),
`branding/`, `resources/splash.png`.

## Workflow de sync (quando o Drift lançar versão nova)

1. `git fetch upstream --tags`
2. Rebase dos nossos commits sobre a NOVA TAG (`git rebase --onto vX.Y.Z <tag-antiga> main`).
   Conflitos esperados: quase nenhum (delta fino); `main.cpp` se mexerem no boot.
3. Regenerar o catálogo pt-BR contra o novo `i18n/drift.ts`: script
   `remap-ptbr.ps1` (reaproveita por chave contexto+fonte com normalização de
   apóstrofos retos↔curvos; só traduzir as strings novas à mão).
4. Conferir regras de ouro 4–6. Push (o Alex roda force-push se pedir; push normal
   passa).
5. O workflow **Tests** roda sozinho no push (10 testes; `Translations` é o
   guardião do catálogo).
6. Build de teste: `gh workflow run build-windows.yml -R cpmdark/turbocut-cutwire -f version=X.Y.Z-turboN`.
7. Validou? Tag `vX.Y.Z` → release.yml gera TODAS as plataformas e publica a
   release (o aviso de atualização in-app e o card do TurboStudio leem daqui).
8. Atualizar `## Estado atual (atualizado 30/08/2026 — v0.5.0 LANCADA, 1a atualizacao real dos usuarios)

- **✅ v0.5.1 NO AR** (05/09): correcao do piscar preto na pre-visualizacao em
  placas NVIDIA (defeito que a 0.5.0 trouxe junto com a aceleracao do preview).
  Sync limpo: 1 conflito so (selo de plataformas do README), catalogo com 0
  pendencias, release verde de primeira em todas as plataformas.
  https://github.com/cpmdark/turbocut-cutwire/releases/tag/v0.5.1
- **⏳ PROXIMO SYNC ja tem material:** o `main` deles tem ~20 commits apos a
  v0.5.1, com **camada de ajuste** (ideia do Alex, issue #154, entregue em 1 dia),
  **multi-selecao + acoes em lote na bin** (nossa #123) e **arrastar a selecao
  junta na timeline**. Aguardando eles cortarem a tag (regra #0.1). O pt_BR do
  `main` deles ja tem **44 strings sem traducao**.
- **🎉 v0.5.0 NO AR** (30/08): primeiro sync completo do fork, e a PRIMEIRA
  atualizacao que os usuarios recebem pelos canais que construimos (aviso in-app
  + botao "Atualizar" do card do TurboStudio, ambos estreando).
  https://github.com/cpmdark/turbocut-cutwire/releases/tag/v0.5.0
  - Entregou: **export ~3x mais rapido** (medido pelo Alex; era a issue #127,
    que eles corrigiram), pastas na biblioteca, preview/aparar/cortar antes da
    timeline, estabilizacao, Face Swap, camada de efeitos, escala da interface.
  - Delta caiu de 19 para **14 commits** — o fix FreeType saiu porque o upstream
    adotou (regra de ouro 7 virou historica).
  - Catalogo: 27 strings novas traduzidas; o lupdate deles gravou
    `language="en"` num arquivo pt_BR e o cabecalho foi restaurado (regra 5).
  - Notas da release reescritas em portugues no metainfo (a entrada que veio do
    upstream falava de Drift/Android em ingles) e titulo corrigido a mao para
    "TurboCut v0.5.0" (item 8-B: o release.yml usa o nome do metainfo).
- **⚠️ 3 armadilhas do release, para o proximo sync:**
  1. **A tag do upstream tem o MESMO nome da nossa** (`v0.5.0`). Depois do
     `git fetch upstream --tags`, a tag local aponta pro commit DELES —
     `git tag -d vX.Y.Z` e recriar em `main` ANTES de empurrar, senao o CI
     publica o codigo do upstream sem a nossa marca.
  2. **Force-push x branch protection:** o sync reescreve a historia. O Alex
     liga "Allow force pushes" nas settings, a IA empurra e **recoloca a trava
     via API** (`gh api -X PUT .../branches/main/protection --input -`).
  3. **Uma falha em QUALQUER pacote pula a publicacao** ("Publish GitHub
     release: skipped"). Na 0.5.0 o Flatpak falhou baixando o `soundtouch` de
     `www.surina.net` (site externo fora do ar) — `gh run rerun <id> --failed`
     resolveu sem recompilar o resto.
- **Repo na organizacao `cpmdark`**; landing nos 2 dominios com o endereco novo.
- **Upstream:** traducao mergeada (#122) e o Alex creditado nas notas da 0.5.0;
  #115/#116/#127 fechadas. Comentarios postados em 30/08 nas #123 (multi-selecao
  na bin + importar pasta), #100 (acoes em grupo) e #76 (clipe composto) —
  **prazo de reavaliacao: ~13/09/2026** (item 19 do backlog: se nao andar,
  decidir entre PR nosso e patch no fork).
- **Rodrigo (autocut)** tem fork (`guigomaster01/turbocut-cutwire`), sem codigo.
- Pendencias e backlog: `_planejamento/BACKLOG_PROXIMAS_TAREFAS.md` (privado).

## Checklist antes de qualquer alteração

- [ ] Isso é feature de motor? → upstream, não aqui (Regra #0).
- [ ] Toquei em catálogo? → regras de ouro 4–5.
- [ ] Toquei em asset? → mesmo nome de arquivo (regra 1).
- [ ] Vou mexer no TurboStudio? → PARE: avisar o Alex antes, protocolo da bancada.
- [ ] Terminei algo? → backlog privado e `## Estado atual` atualizados junto.
