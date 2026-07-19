#include "OverlayController.hpp"
#include "SocketServer.hpp"
#include "AppCatalogModel.hpp"
#include "ConfigStore.hpp"

#include <LayerShellQt/Window>

#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

namespace {
constexpr auto kSocketName = "radial-launcher.sock";
constexpr auto kLayerNamespace = "radial-launcher";
} // namespace

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("radiald");

    ConfigStore configStore;
    AppCatalogModel appCatalog(&configStore);
    OverlayController overlayController;

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("appicon"), new AppIconProvider());
    engine.rootContext()->setContextProperty(QStringLiteral("config"), &configStore);
    engine.rootContext()->setContextProperty(QStringLiteral("appCatalog"), &appCatalog);
    engine.rootContext()->setContextProperty(QStringLiteral("overlayController"), &overlayController);

    const QUrl mainQml(QStringLiteral("qrc:/qt/qml/Radial/ui/qml/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [mainQml](QObject *obj, const QUrl &objUrl) {
            if (!obj && objUrl == mainQml) {
                QCoreApplication::exit(1);
            }
        },
        Qt::QueuedConnection);
    engine.load(mainQml);

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window) {
        return 1;
    }

    auto *layerWindow = LayerShellQt::Window::get(window);
    layerWindow->setScope(QString::fromUtf8(kLayerNamespace));
    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
    LayerShellQt::Window::Anchors anchors;
    anchors |= LayerShellQt::Window::AnchorTop;
    anchors |= LayerShellQt::Window::AnchorBottom;
    anchors |= LayerShellQt::Window::AnchorLeft;
    anchors |= LayerShellQt::Window::AnchorRight;
    layerWindow->setAnchors(anchors);

    overlayController.setWindow(window);
    SocketServer socketServer(QString::fromUtf8(kSocketName));
    overlayController.bindSocketServer(&socketServer);

    QString startError;
    if (!socketServer.start(&startError)) {
        qCritical("Failed to start socket server: %s", qPrintable(startError));
        return 1;
    }

    window->hide();
    return app.exec();
}
