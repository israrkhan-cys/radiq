#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QVariantList>
#include <QQuickImageProvider>

struct AppInfo {
    QString desktopId;
    QString name;
    QString exec;
    QString iconName;
    QString firstLetter;
    bool hasIcon = false;
};

class ConfigStore;

class AppCatalogModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QVariantList wheelApps READ wheelApps NOTIFY wheelAppsChanged)

public:
    enum Roles {
        DesktopIdRole = Qt::UserRole + 1,
        NameRole,
        ExecRole,
        IconNameRole,
        FirstLetterRole,
        HasIconRole
    };

    explicit AppCatalogModel(ConfigStore *configStore = nullptr, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString desktopIdAt(int row) const;
    Q_INVOKABLE QString nameAt(int row) const;
    Q_INVOKABLE void launch(const QString &desktopId);
    Q_INVOKABLE void launchExec(const QString &exec);

    QString searchQuery() const;
    void setSearchQuery(const QString &query);

    QVariantList wheelApps() const;

    const AppInfo *appByDesktopId(const QString &desktopId) const;
    const QVector<AppInfo> &apps() const;

signals:
    void searchQueryChanged();
    void wheelAppsChanged();

private:
    void loadApps();
    void updateFilteredApps();

    QVector<AppInfo> m_allApps;
    QVector<AppInfo> m_filteredApps;
    QVariantList m_wheelApps;
    QString m_searchQuery;
    ConfigStore *m_configStore = nullptr;
};

class AppIconProvider final : public QQuickImageProvider {
public:
    AppIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};
