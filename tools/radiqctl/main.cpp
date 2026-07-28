#include <QCoreApplication>
#include <QLocalSocket>
#include <QStringList>

namespace {
constexpr auto kSocketName = "radiq-launcher.sock";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    const QString command = args.size() > 1 ? args.at(1).trimmed().toLower() : QStringLiteral("toggle");

    if (command != "show" && command != "hide" && command != "toggle") {
        qWarning("Usage: radiqctl [show|hide|toggle]");
        return 2;
    }

    QLocalSocket socket;
    socket.connectToServer(QString::fromUtf8(kSocketName));
    if (!socket.waitForConnected(1000)) {
        qWarning("Could not connect to radiqd socket '%s'", kSocketName);
        return 1;
    }

    socket.write(command.toUtf8());
    if (!socket.waitForBytesWritten(1000)) {
        qWarning("Failed to send command.");
        return 1;
    }

    socket.disconnectFromServer();
    return 0;
}
