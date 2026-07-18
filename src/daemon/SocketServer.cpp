#include "SocketServer.hpp"

#include <QByteArray>
#include <QLocalServer>
#include <QLocalSocket>

SocketServer::SocketServer(const QString &socketName, QObject *parent)
    : QObject(parent), m_socketName(socketName), m_server(new QLocalServer(this)) {
    connect(m_server, &QLocalServer::newConnection, this, &SocketServer::handleNewConnection);
}

bool SocketServer::start(QString *errorMessage) {
    QLocalServer::removeServer(m_socketName);
    if (!m_server->listen(m_socketName)) {
        if (errorMessage) {
            *errorMessage = m_server->errorString();
        }
        return false;
    }
    return true;
}

void SocketServer::handleNewConnection() {
    while (m_server->hasPendingConnections()) {
        QLocalSocket *client = m_server->nextPendingConnection();
        if (!client) {
            continue;
        }

        if (client->waitForReadyRead(500)) {
            const QByteArray raw = client->readAll();
            handleCommand(QString::fromUtf8(raw).trimmed().toLower());
        }

        client->disconnectFromServer();
        client->deleteLater();
    }
}

void SocketServer::handleCommand(const QString &command) {
    if (command == "show") {
        emit showRequested();
        return;
    }
    if (command == "hide") {
        emit hideRequested();
        return;
    }
    if (command == "toggle") {
        emit toggleRequested();
        return;
    }
}
