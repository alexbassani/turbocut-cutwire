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
8. Atualizar `## Estado atual (atualizado 30/08/2026 — SYNC DA v0.5.0 FEITO, aguardando validacao do build)

- **⏳ SYNC 0.5.0 EM ANDAMENTO (o mais importante agora):** o Drift cortou a
  `v0.5.0` em 30/08 e o rebase JA FOI FEITO e empurrado (main = base v0.5.0).
  Falta: validar o `TurboCut-Setup` de teste (`0.5.0-turbo1`) na tela do Alex e,
  com o OK dele, cortar a tag `v0.5.0` — que dispara a release e a PRIMEIRA
  atualizacao real dos usuarios (aviso in-app + card do TurboStudio).
  - Delta emagreceu: **14 commits** (eram 19) — o fix FreeType saiu porque eles
    adotaram (regra de ouro 7 agora e historica; o codigo vem do upstream).
  - Conflitos resolvidos: `main.cpp` (nomes migraram para antes do QApplication,
    por causa do ui/scale novo; include duplicado do splash), `Main.qml`
    (`visibility` em vez de `visible`), metainfo (entradas 0.5.0 + 0.4.1).
  - Catalogo: 27 strings novas traduzidas (pastas na bin, Android, preview na
    GPU) e **cabecalho consertado** — o lupdate deles gravou `language="en"`
    num arquivo pt_BR (regra de ouro 5). Tests VERDE, `Translations` incluso.
  - ⚠️ **Push do sync exige force-push** e a branch protection bloqueia: o Alex
    liga "Allow force pushes" nas settings, a IA empurra e **recoloca a trava
    via API** (`gh api -X PUT .../branches/main/protection --input -`).
- **O que a 0.5.0 traz aos usuarios:** export e preview na GPU (resolve a dor
  do Alex, nossa issue #127 fechada como concluida), pastas na biblioteca,
  preview/trim/crop antes da timeline, estabilizacao de video, Face Swap,
  escala da interface, e a traducao pt-BR do Alex **creditada nas notas deles**.
- **v0.4.1** e a release publica atual (VC++ Redistributable no instalador).
- **Card do TurboStudio** funciona com aluno real (trem 5.51.1); o "atualizar"
  estreia justamente nesta 0.5.0.
- **Landing** nos dois dominios com o endereco novo (cpmdark.com.br/turbocut).
- **Repo na organizacao `cpmdark`** desde 30/08 (transferencia completa: feed de
  update, instalador, README, PKGBUILD, metainfo, docs e landing).
- **Upstream:** traducao mergeada (PR #122); #115/#116/#127 fechadas; #124 virou
  duplicata da #100 deles (aplicar transicao/atributos em massa — pedido real de
  usuario no YouTube, candidata a PR nosso); #123 e #125 abertas.
- **Rodrigo (autocut)** tem fork (`guigomaster01/turbocut-cutwire`), sem codigo.
- Pendencias e backlog: `_planejamento/BACKLOG_PROXIMAS_TAREFAS.md` (privado).

## Checklist antes de qualquer alteração

- [ ] Isso é feature de motor? → upstream, não aqui (Regra #0).
- [ ] Toquei em catálogo? → regras de ouro 4–5.
- [ ] Toquei em asset? → mesmo nome de arquivo (regra 1).
- [ ] Vou mexer no TurboStudio? → PARE: avisar o Alex antes, protocolo da bancada.
- [ ] Terminei algo? → backlog privado e `## Estado atual` atualizados junto.
