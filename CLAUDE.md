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
8. Atualizar `## Estado atual` abaixo NO MESMO fluxo (regra da casa: doc que
   envelhece vira armadilha).

## Ecossistema (contexto, não código deste repo)

- **TurboStudio** (repo privado do Alex) tem o card que instala/atualiza/abre o
  TurboCut lendo as releases deste repo. ⛔ Bancada compartilhada com outras
  frentes: NUNCA mexer lá sem avisar o Alex antes e sem o protocolo da casa.
- **Landing**: cpmdark.com.br/turbocut (fonte em `_planejamento/landing/`).
- **Upstream**: relação de colaborador — bugs reportados, tradução oferecida.
  Rascunhos de issues em `_planejamento/rascunhos-upstream/` (só o Alex posta).

## Quem é o usuário

Alex Bassani — criador de conteúdo e "vibecoder": descreve o que quer, valida na
tela, decide o produto; a IA executa e explica sem jargão. Sempre pt-BR. Decisões
dele são registradas com data e palavras literais nos docs de planejamento.

## Estado atual (atualizado 30/08/2026 — distribuição fechada; aguardando tag do Drift)

- **Repo transferido para a organização `cpmdark`** (30/08): agora é
  `github.com/cpmdark/turbocut-cutwire` (nome bate com o domínio oficial).
  Links antigos redirecionam sozinhos; feed de update, instalador, README,
  PKGBUILD, metainfo e landing já apontam pro endereço novo. Falta: card do
  TurboStudio (bilhete com o Alex) e build de teste antes do próximo release.
- **v0.4.1 no ar**: instalador Windows leva o **VC++ Redistributable** (bug real
  de Windows limpo, `MSVCP140_1.dll`; o Drift oficial ainda tem o furo — reporte
  upstream pendente de OK do Alex, item 16-B do backlog).
- **v0.4.0** foi o lançamento: repo público, release multi-plataforma, app 100%
  pt-BR, splash de marca, fix FreeType, ícone oficial, Tests 10/10.
- **Card do TurboStudio SAIU no trem 5.51.1 e FUNCIONA** com aluno real
  (confirmado pelo Alex em 30/08) — baixa/instala/abre; o "atualizar" estreia no
  primeiro sync.
- **Landing no domínio oficial**: cpmdark.com.br/turbocut (mirror
  alexbassani.com.br/turbocut segue no ar).
- **Upstream:** tradução pt-BR mergeada (PR #122); fix FreeType adotado (#115) —
  sai do delta no próximo sync; incidente da aspa ENCERRADO (main deles verde,
  nosso PR #128 fechado como superseded). Registro em
  `_planejamento/RELACAO_UPSTREAM.md`.
- **Aguardando o Drift cortar tag nova** — será a 1ª atualização real dos
  usuários; 327 strings novas já traduzidas e playbook pronto.
- **Rodrigo (autocut)** fez o fork (`guigomaster01/turbocut-cutwire`); sem
  código ainda; onde o autocut mora segue decisão pendente do Alex.
- Pendências e backlog: `_planejamento/BACKLOG_PROXIMAS_TAREFAS.md` (privado).

## Checklist antes de qualquer alteração

- [ ] Isso é feature de motor? → upstream, não aqui (Regra #0).
- [ ] Toquei em catálogo? → regras de ouro 4–5.
- [ ] Toquei em asset? → mesmo nome de arquivo (regra 1).
- [ ] Vou mexer no TurboStudio? → PARE: avisar o Alex antes, protocolo da bancada.
- [ ] Terminei algo? → backlog privado e `## Estado atual` atualizados junto.
