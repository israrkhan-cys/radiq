#pragma once

#include <QObject>

class QQuickWindow;
class SocketServer;

class OverlayController final : public QObject {
    Q_OBJECT
public:
    explicit OverlayController(QQuickWindow *window = nullptr, QObject *parent = nullptr);
    void setWindow(QQuickWindow *window);
    void bindSocketServer(SocketServer *server);

signals:
    void closeRequested();

public slots:
    void showOverlay();
    void hideOverlay();
    void toggleOverlay();
    Q_INVOKABLE void finishHide();

private:
    QQuickWindow *m_window = nullptr;
};

