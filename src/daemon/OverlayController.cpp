#include "OverlayController.hpp"

#include "SocketServer.hpp"

#include <QQuickWindow>

OverlayController::OverlayController(QQuickWindow *window, QObject *parent)
    : QObject(parent), m_window(window) {}

void OverlayController::setWindow(QQuickWindow *window) {
    m_window = window;
}

void OverlayController::bindSocketServer(SocketServer *server) {
    connect(server, &SocketServer::showRequested, this, &OverlayController::showOverlay);
    connect(server, &SocketServer::hideRequested, this, &OverlayController::hideOverlay);
    connect(server, &SocketServer::toggleRequested, this, &OverlayController::toggleOverlay);
}

void OverlayController::showOverlay() {
    if (!m_window) {
        return;
    }
    m_window->show();
    m_window->requestActivate();
    m_window->raise();
}

void OverlayController::hideOverlay() {
    emit closeRequested();
}

void OverlayController::finishHide() {
    if (!m_window) {
        return;
    }
    m_window->hide();
}

void OverlayController::toggleOverlay() {
    if (!m_window) {
        return;
    }
    if (m_window->isVisible()) {
        hideOverlay();
    } else {
        showOverlay();
    }
}
