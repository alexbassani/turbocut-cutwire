#include "engine/AudioFileWriter.h"
#include "engine/EmojiCatalog.h"
#include "engine/FontCatalog.h"
#include "engine/ReverseProxyCache.h"
#include "mcp/McpStdio.h"
#include "models/AddonManager.h"
#include "models/AppController.h"
#include "models/AssetLibrary.h"
#include "models/EditorState.h"
#include "models/FileDialogs.h"
#include "models/LayoutStore.h"
#include "models/UpdateChecker.h"
#include "ClipPreviewImageProvider.h"
#include "DriftImageProvider.h"
#include "MulticamImageProvider.h"
#include "SegmentImageProvider.h"
#include "TextStylePreviewImageProvider.h"
#include "preview/PreviewItem.h"

// QApplication (not QGuiApplication) is required so QFileDialog can use the
// native platform file picker, which routes through xdg-desktop-portal.
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QFileOpenEvent>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QPainter>
#include <QSplashScreen>
#include <QThread>
#include <QTimer>
#include <QtQml/qqml.h>
#include <QFile>

#include <cmath>

extern "C" {
#include <libavutil/log.h>
}

namespace {

bool verboseLoggingRequested(int argc, char *argv[])
{
    if (qEnvironmentVariableIntValue("DRIFT_VERBOSE") != 0)
        return true;
    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "--verbose") == 0)
            return true;
    }
    return false;
}

// FFmpeg logs at INFO and Qt prints every qDebug/qInfo, which buries the failures worth acting on
// under per-frame filtergraph chatter. qWarning is this codebase's failure channel, so it stays on
// either way. QT_LOGGING_RULES is applied after these (EnvironmentRules outrank ApiRules), so it
// still overrides them.
void applyLogLevel(bool verbose)
{
    QLoggingCategory::setFilterRules(verbose
                                         ? QStringLiteral("*.debug=true\n"
                                                          "*.info=true\n"
                                                          "qt.*.debug=false")
                                         : QStringLiteral("*.debug=false\n"
                                                          "*.info=false"));
    av_log_set_level(verbose ? AV_LOG_VERBOSE : AV_LOG_ERROR);
}

// TurboCut: splash de abertura — fundo preto com o wordmark (resources/splash.png,
// pixmap 2x para telas com escala) e uma barra prata espelhada deslizando em modo
// indeterminado. Fica na tela enquanto fontes, caches e o QML carregam; fecha no
// instante em que a janela principal aparece.
class TurboSplash : public QSplashScreen
{
public:
    TurboSplash()
        : QSplashScreen([] {
              QPixmap pm(QStringLiteral(":/app/splash.png"));
              pm.setDevicePixelRatio(2.0);
              return pm;
          }())
    {
        m_pulse.setInterval(33);
        QObject::connect(&m_pulse, &QTimer::timeout, this, [this] {
            m_phase = std::fmod(m_phase + 0.022, 1.0);
            update();
        });
        m_pulse.start();
    }

protected:
    void drawContents(QPainter *painter) override
    {
        QSplashScreen::drawContents(painter);
        const double w = width();
        const double h = height();
        const double barW = w * 0.46;
        const double barH = 5.0;
        const QRectF track((w - barW) / 2.0, h * 0.72, barW, barH);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 26));
        painter->drawRoundedRect(track, barH / 2.0, barH / 2.0);
        // Bloco prata espelhado varrendo o trilho (indeterminado: o boot não tem
        // estágios instrumentados, então a barra mostra vida, não porcentagem).
        const double blockW = barW * 0.28;
        const double x = track.left() - blockW + m_phase * (barW + 2.0 * blockW);
        QRectF block = QRectF(x, track.top(), blockW, barH).intersected(track);
        if (block.width() <= 0.5)
            return;
        QLinearGradient silver(block.topLeft(), block.bottomLeft());
        silver.setColorAt(0.0, QColor(250, 250, 252));
        silver.setColorAt(0.45, QColor(148, 150, 156));
        silver.setColorAt(0.5, QColor(232, 234, 238));
        silver.setColorAt(1.0, QColor(108, 110, 116));
        painter->setBrush(silver);
        painter->drawRoundedRect(block, barH / 2.0, barH / 2.0);
    }

private:
    QTimer m_pulse;
    double m_phase = 0.0;
};

class FileOpenFilter : public QObject
{
public:
    explicit FileOpenFilter(AppController *controller, QObject *parent = nullptr)
        : QObject(parent)
        , m_controller(controller)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            const auto *open = static_cast<QFileOpenEvent *>(event);
            m_controller->queueExternalProject(open->url());
            return true;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    AppController *m_controller = nullptr;
};

} // namespace

int main(int argc, char *argv[])
{
    applyLogLevel(verboseLoggingRequested(argc, argv));

    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "--mcp-stdio") == 0) {
            QCoreApplication app(argc, argv);
            QCoreApplication::setApplicationName("TurboCut");
            QCoreApplication::setOrganizationName("TurboCut");
            return drift::mcp::runStdioAttach();
        }
    }

    // On NVIDIA/Wayland, leaving the API unspecified makes EGL interpret 3.3
    // as an invalid GLES version and fail with EGL_BAD_MATCH. Start from the
    // default format to retain the platform-selected window-buffer attributes.
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Keep Qt Quick on OpenGL and create its global share context before the
    // application, enabling zero-copy texture handoff from the compositor.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

#ifdef Q_OS_WIN
    // TurboCut: the DirectWrite font engine renders the qrc-embedded UI fonts with
    // corrupted glyphs on some GPU/driver/display-scaling combinations (glyph indices
    // resolve against the wrong table). FreeType is unaffected. Overridable via env.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "windows:fontengine=freetype");
#endif

    QApplication app(argc, argv);
    QApplication::setApplicationName("TurboCut");
    QApplication::setOrganizationName("TurboCut");
    // Associates the window with the installed .desktop entry so shells (notably
    // Wayland) can find its icon and app metadata.
    QGuiApplication::setDesktopFileName(QStringLiteral("org.cutwire.Drift"));
    // Title bar / taskbar icon when no desktop entry is available (Windows, and
    // Linux runs from the build tree). The .exe still needs the Windows .rc icon
    // for Explorer and pinned-taskbar identity.
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/app/drift.png")));

    // TurboCut: splash antes das cargas pesadas (fontes, caches, QML). O fade é
    // manual porque essas cargas bloqueiam o event loop — uma QPropertyAnimation
    // só rodaria depois que tudo já tivesse carregado.
    auto *splash = new TurboSplash();
    splash->show();
    for (int step = 1; step <= 10; ++step) {
        splash->setWindowOpacity(step / 10.0);
        QCoreApplication::processEvents();
        QThread::msleep(24);
    }

    // qsTr/tr resolve when the QML engine loads, so translators must be installed first.
    // Protocol strings under src/mcp/ are excluded from the catalog; they stay English.
    AppController::installUiTranslators();

    // Registering the bundled fonts needs a QGuiApplication, and must happen before the compositor
    // thread starts touching QFontDatabase.
    reloadFontCatalog();
    reloadEmojiCatalog();

    // Noise-removal A/B snippets are scratch. Anything still here is from a previous session that
    // did not get to clean up after itself.
    drift::sweepDenoisePreviews();

    // Reversed proxies are a pure cache: dropping one only costs the clip its smooth playback, so
    // they are pruned to a budget rather than kept forever the way mattes are.
    drift::ReverseProxyCache::instance().load();
    drift::ReverseProxyCache::instance().sweep(drift::ReverseProxyCache::kDefaultMaxBytes);

    qmlRegisterType<PreviewItem>("Drift", 1, 0, "PreviewItem");

    static AssetLibrary assetLibrary;
    static EditorState editorState(&assetLibrary);
    static FileDialogs fileDialogs;
    static AddonManager addonManager;
    static UpdateChecker updateChecker;
    static LayoutStore layoutStore;
    qmlRegisterSingletonInstance("Drift", 1, 0, "AssetLibrary", &assetLibrary);
    qmlRegisterSingletonInstance("Drift", 1, 0, "EditorState", &editorState);
    qmlRegisterSingletonInstance("Drift", 1, 0, "AppController", &editorState);
    qmlRegisterSingletonInstance("Drift", 1, 0, "FileDialogs", &fileDialogs);
    qmlRegisterSingletonInstance("Drift", 1, 0, "Addons", &addonManager);
    qmlRegisterSingletonInstance("Drift", 1, 0, "Updates", &updateChecker);
    qmlRegisterSingletonInstance("Drift", 1, 0, "LayoutMemory", &layoutStore);

    app.installEventFilter(new FileOpenFilter(&editorState, &app));
    editorState.queueExternalProject(
        AppController::startupProjectUrlFromArguments(app.arguments()));

    QQmlApplicationEngine engine;
    QObject::connect(&editorState, &AppController::uiLanguageChanged,
                     &engine, &QQmlEngine::retranslate);
    engine.addImageProvider(QStringLiteral("drift"), new DriftImageProvider());
    engine.addImageProvider(QStringLiteral("segment"), new SegmentImageProvider());
    engine.addImageProvider(QStringLiteral("clippreview"), new ClipPreviewImageProvider());
    engine.addImageProvider(QStringLiteral("multicam"), new MulticamImageProvider());
    engine.addImageProvider(QStringLiteral("textstyle"), new TextStylePreviewImageProvider());
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QGuiApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("Drift", "Main");

    // Fecha o splash no instante em que a janela principal aparecer (Main.qml nasce
    // com visible=false e só mostra depois de restaurar a geometria da sessão).
    const auto rootObjects = engine.rootObjects();
    QQuickWindow *mainWindow =
        rootObjects.isEmpty() ? nullptr : qobject_cast<QQuickWindow *>(rootObjects.first());
    if (mainWindow) {
        QObject::connect(mainWindow, &QWindow::visibleChanged, splash, [splash](bool visible) {
            if (visible)
                splash->deleteLater();
        });
    }
    // Rede de segurança: o splash nunca fica pendurado se a janela não vier.
    QTimer::singleShot(15000, splash, &QObject::deleteLater);

    return app.exec();
}
