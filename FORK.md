# TurboCut × Drift — o que muda neste fork

O TurboCut é um *fork fino* do [CutWire Drift](https://github.com/CutWire-Studios/Drift),
mantido por [Alex Bassani](https://github.com/alexbassani). O motor, a arquitetura e o mérito
são do CutWire Studios — este fork existe para entregar o editor com identidade TurboCut e em
português do Brasil, como parte do ecossistema Turbo.

Licença: **GPL-3.0**, a mesma do upstream. O código-fonte completo deste fork vive neste
repositório.

## Política do fork

- **Delta mínimo.** Nada de refatorar o motor nem criar recursos próprios aqui: mudança que
  valha a pena vira contribuição para o upstream. Este fork carrega apenas marca, tradução,
  conteúdo e empacotamento.
- **Compatibilidade total de projetos.** O formato `.drift` não é tocado — um projeto abre no
  Drift e no TurboCut igualmente.
- **Sync frequente.** `main` = `upstream/main` + os commits listados abaixo, rebaseados a cada
  release do Drift.

## Mudanças em relação ao upstream

| Área | Arquivos | O quê |
|---|---|---|
| Nome do app | `src/main.cpp`, `src/qml/Main.qml` | `setApplicationName/OrganizationName` e título da janela → "TurboCut" (AppData fica em `%APPDATA%\TurboCut`, separado do Drift) |
| Ícones | `Drift_icon.png`, `resources/drift.png`, `resources/windows/drift.ico` | Arte TurboCut (mesmos nomes de arquivo para manter o delta pequeno) |
| Instalador Windows | `installer/windows/Drift.iss` | Nome/publisher TurboCut, **AppId próprio** (nunca conflita com uma instalação do Drift), binário instalado como `turbocut.exe`, instalador em pt-BR, ProgID `TurboCut.Project` |
| Visual do instalador | `resources/windows/wizard-large.bmp`, `wizard-small.bmp` (gerados; fonte em `branding/TurboCut-instalador-banner.png`) | Banner e logo do assistente do Inno Setup com a marca TurboCut, no lugar do visual padrão |
| Feed de atualização | `CMakeLists.txt` (`DRIFT_UPDATE_FEED_URL`) | Aponta para as releases deste repositório, não para as do Drift |
| Link de bugs no app | `src/qml/components/DebugInfoDialog.qml` | "Report a bug" abre as issues deste repositório (documentação e Discord continuam sendo os do CutWire) |
| CI | `.github/workflows/package.yml`, `build-windows.yml` (novo) | Artefatos `TurboCut-Setup-*.exe` / `TurboCut-Portable-*.zip`; build manual só-Windows |
| Tradução | `i18n/drift_pt_BR.ts` | Interface completa em português do Brasil (planejamos contribuir a tradução ao upstream) |
| Identidade | `README.md`, `FORK.md`, `branding/` | Este aviso, a arte da marca |

Fora isso, o código é o do Drift. Serviço de addons, documentação (docs.cutwire.org) e
comunidade continuam sendo os do projeto original.

## Avisos que permanecem no app

Alguns textos internos continuam dizendo "Drift" (relatório de debug, guia MCP, filtros de
arquivo) — de propósito: são superfícies técnicas ligadas ao formato/protocolo do upstream, e
mantê-las intactas conserva o delta pequeno e a compatibilidade.
