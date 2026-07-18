#pragma once

#include <QObject>
#include <QString>

class QLocalServer;

class SocketServer final : public QObject {
    Q_OBJECT
public:
    explicit SocketServer(const QString &socketName, QObject *parent = nullptr);
    bool start(QString *errorMessage = nullptr);

signals:
    void showRequested();
    void hideRequested();
    void toggleRequested();

private slots:
    void handleNewConnection();

private:
    void handleCommand(const QString &command);

    QString m_socketName;
    QLocalServer *m_server = nullptr;
};
