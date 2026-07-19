#include "AppCatalogModel.hpp"
#include "ConfigStore.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QSet>
#include <QProcess>
#include <QRegularExpression>
#include <QIcon>
#include <QDebug>

namespace {
QString cleanExec(const QString &exec) {
    QString result = exec;
    // Remove standard desktop entry field codes like %f, %F, %u, %U, %i, %c, %k etc.
    static QRegularExpression fieldCodeRegex(R"(\s*['"]?%[a-zA-Z]['"]?)");
    result.remove(fieldCodeRegex);
    return result.trimmed();
}

int matchScore(const QString &text, const QString &query) {
    if (query.isEmpty()) return 0;
    QString t = text.toLower();
    QString q = query.toLower();
    
    if (t == q) return 100;
    if (t.startsWith(q)) return 90 - t.indexOf(q);
    if (t.contains(q)) return 70 - t.indexOf(q);
    
    // Fuzzy subsequence match
    int textIdx = 0;
    int queryIdx = 0;
    while (textIdx < t.length() && queryIdx < q.length()) {
        if (t[textIdx] == q[queryIdx]) {
            queryIdx++;
        }
        textIdx++;
    }
    if (queryIdx == q.length()) {
        return 10;
    }
    return -1;
}
} // namespace

AppCatalogModel::AppCatalogModel(ConfigStore *configStore, QObject *parent)
    : QAbstractListModel(parent), m_configStore(configStore) {
    if (m_configStore) {
        connect(m_configStore, &ConfigStore::pinnedAppsChanged, this, &AppCatalogModel::loadApps);
    }
    loadApps();
}

int AppCatalogModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_filteredApps.size();
}

QVariant AppCatalogModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredApps.size()) {
        return QVariant();
    }
    const AppInfo &app = m_filteredApps.at(index.row());
    switch (role) {
        case DesktopIdRole:
            return app.desktopId;
        case NameRole:
            return app.name;
        case ExecRole:
            return app.exec;
        case IconNameRole:
            return app.iconName;
        case FirstLetterRole:
            return app.firstLetter;
        case HasIconRole:
            return app.hasIcon;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> AppCatalogModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DesktopIdRole] = "desktopId";
    roles[NameRole] = "name";
    roles[ExecRole] = "exec";
    roles[IconNameRole] = "iconName";
    roles[FirstLetterRole] = "firstLetter";
    roles[HasIconRole] = "hasIcon";
    return roles;
}

QString AppCatalogModel::desktopIdAt(int row) const {
    if (row < 0 || row >= m_filteredApps.size()) return QString();
    return m_filteredApps.at(row).desktopId;
}

QString AppCatalogModel::nameAt(int row) const {
    if (row < 0 || row >= m_filteredApps.size()) return QString();
    return m_filteredApps.at(row).name;
}

void AppCatalogModel::launch(const QString &desktopId) {
    const AppInfo *app = appByDesktopId(desktopId);
    if (app) {
        launchExec(app->exec);
    } else {
        // Fallback for hardcoded apps that might not have been fully parsed
        for (const auto &val : m_wheelApps) {
            QVariantMap map = val.toMap();
            if (map.value("desktopId").toString() == desktopId) {
                launchExec(map.value("exec").toString());
                return;
            }
        }
        qWarning() << "Could not launch app with desktopId:" << desktopId;
    }
}

void AppCatalogModel::launchExec(const QString &exec) {
    QString cleaned = cleanExec(exec);
    if (cleaned.isEmpty()) {
        qWarning() << "Cleaned exec string is empty, cannot launch.";
        return;
    }
    qDebug() << "Launching via hyprctl exec:" << cleaned;
    QProcess::startDetached("hyprctl", QStringList() << "dispatch" << "exec" << "--" << cleaned);
}

QString AppCatalogModel::searchQuery() const {
    return m_searchQuery;
}

void AppCatalogModel::setSearchQuery(const QString &query) {
    if (m_searchQuery == query) return;
    m_searchQuery = query;
    emit searchQueryChanged();
    updateFilteredApps();
}

QVariantList AppCatalogModel::wheelApps() const {
    return m_wheelApps;
}

const AppInfo *AppCatalogModel::appByDesktopId(const QString &desktopId) const {
    for (const auto &app : m_allApps) {
        if (app.desktopId == desktopId) {
            return &app;
        }
    }
    return nullptr;
}

const QVector<AppInfo> &AppCatalogModel::apps() const {
    return m_allApps;
}

void AppCatalogModel::loadApps() {
    m_allApps.clear();
    QSet<QString> processedDesktopIds;

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString &dirPath : appDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        // Scan recursively to catch subdirectories (e.g. kde or categories)
        QDirIterator it(dirPath, QStringList() << "*.desktop", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QString desktopId = QFileInfo(filePath).fileName();

            if (processedDesktopIds.contains(desktopId)) continue;
            processedDesktopIds.insert(desktopId);

            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            QTextStream in(&file);
            QString name;
            QString exec;
            QString iconName;
            bool noDisplay = false;
            bool hidden = false;
            bool inDesktopEntry = false;

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("[Desktop Entry]")) {
                    inDesktopEntry = true;
                    continue;
                }
                if (line.startsWith("[") && inDesktopEntry) {
                    // Entered another section
                    inDesktopEntry = false;
                }
                if (!inDesktopEntry) continue;

                // Simple parser
                if (line.startsWith("Name=")) {
                    name = line.mid(5).trimmed();
                } else if (line.startsWith("Exec=")) {
                    exec = line.mid(5).trimmed();
                } else if (line.startsWith("Icon=")) {
                    iconName = line.mid(5).trimmed();
                } else if (line.startsWith("NoDisplay=")) {
                    noDisplay = (line.mid(10).trimmed().toLower() == "true");
                } else if (line.startsWith("Hidden=")) {
                    hidden = (line.mid(7).trimmed().toLower() == "true");
                }
            }

            if (noDisplay || hidden || name.isEmpty() || exec.isEmpty()) {
                continue;
            }

            AppInfo app;
            app.desktopId = desktopId;
            app.name = name;
            app.exec = exec;
            app.iconName = iconName;
            app.firstLetter = name.left(1).toUpper();
            if (app.firstLetter.isEmpty()) {
                app.firstLetter = "?";
            }

            // Check if icon exists
            app.hasIcon = false;
            if (!iconName.isEmpty()) {
                if (QIcon::hasThemeIcon(iconName) || QFile::exists(iconName)) {
                    app.hasIcon = true;
                } else {
                    int lastDot = iconName.lastIndexOf('.');
                    if (lastDot > 0) {
                        QString nameWithoutExt = iconName.left(lastDot);
                        if (QIcon::hasThemeIcon(nameWithoutExt)) {
                            app.hasIcon = true;
                            app.iconName = nameWithoutExt;
                        }
                    }
                }
            }

            if (!app.hasIcon) {
                qWarning() << "Fallback to text icon for app:" << app.name << " (Icon entry:" << app.iconName << ")";
            }

            m_allApps.append(app);
        }
    }

    // Get target lists from ConfigStore or fallback
    QStringList targetDesktopIds;
    if (m_configStore) {
        targetDesktopIds = m_configStore->pinnedApps();
    }

    const QStringList defaultDesktopIds = {
        "firefox.desktop",
        "Alacritty.desktop",
        "org.kde.dolphin.desktop",
        "code.desktop",
        "discord.desktop",
        "gimp.desktop",
        "btop.desktop",
        "com.obsproject.Studio.desktop"
    };

    const QStringList defaultNames = {
        "Firefox",
        "Alacritty",
        "Dolphin",
        "VS Code",
        "Discord",
        "GIMP",
        "Btop",
        "OBS Studio"
    };

    const QStringList defaultExecs = {
        "firefox",
        "alacritty",
        "dolphin",
        "code",
        "discord",
        "gimp",
        "alacritty -e btop",
        "obs"
    };

    const QStringList defaultIcons = {
        "firefox",
        "Alacritty",
        "system-file-manager",
        "code",
        "discord",
        "gimp",
        "system-monitor",
        "obs"
    };

    if (targetDesktopIds.isEmpty()) {
        targetDesktopIds = defaultDesktopIds;
    } else {
        // Fill remaining slots up to at least 8 elements using non-duplicate default apps
        QSet<QString> addedIds(targetDesktopIds.begin(), targetDesktopIds.end());
        for (int i = 0; i < defaultDesktopIds.size(); ++i) {
            QString defId = defaultDesktopIds.at(i);
            if (!addedIds.contains(defId)) {
                targetDesktopIds.append(defId);
                addedIds.insert(defId);
            }
            if (targetDesktopIds.size() >= 8) {
                break;
            }
        }
    }

    m_wheelApps.clear();
    for (int i = 0; i < targetDesktopIds.size(); ++i) {
        QString targetId = targetDesktopIds.at(i);
        const AppInfo *foundApp = appByDesktopId(targetId);
        
        QVariantMap map;
        if (foundApp) {
            map["desktopId"] = foundApp->desktopId;
            map["name"] = foundApp->name;
            map["exec"] = foundApp->exec;
            map["iconName"] = foundApp->iconName;
            map["firstLetter"] = foundApp->firstLetter;
            map["hasIcon"] = foundApp->hasIcon;
        } else {
            // Setup fallback item
            map["desktopId"] = targetId;
            
            // Check if it matches one of our defaults
            int defaultIdx = defaultDesktopIds.indexOf(targetId);
            QString fallbackName;
            QString fallbackExec;
            QString fallbackIcon;
            
            if (defaultIdx != -1) {
                fallbackName = defaultNames.at(defaultIdx);
                fallbackExec = defaultExecs.at(defaultIdx);
                fallbackIcon = defaultIcons.at(defaultIdx);
            } else {
                // Custom app fallback guessing
                QString base = targetId;
                if (base.endsWith(".desktop")) {
                    base.chop(8);
                }
                fallbackExec = base;
                fallbackIcon = base;
                
                if (!base.isEmpty()) {
                    fallbackName = base.at(0).toUpper() + base.mid(1);
                } else {
                    fallbackName = "Unknown";
                }
            }
            
            map["name"] = fallbackName;
            map["exec"] = fallbackExec;
            map["iconName"] = fallbackIcon;
            map["firstLetter"] = fallbackName.isEmpty() ? "?" : fallbackName.left(1).toUpper();
            
            bool hasIcon = false;
            if (QIcon::hasThemeIcon(fallbackIcon)) {
                hasIcon = true;
            }
            map["hasIcon"] = hasIcon;
            
            if (!hasIcon) {
                qWarning() << "Fallback to text icon for app:" << fallbackName << " (Icon entry:" << fallbackIcon << ")";
            }
        }
        m_wheelApps.append(map);
    }
    emit wheelAppsChanged();
}

void AppCatalogModel::updateFilteredApps() {
    beginResetModel();
    m_filteredApps.clear();
    
    if (!m_searchQuery.trimmed().isEmpty()) {
        struct ScoredApp {
            AppInfo app;
            int score;
        };
        QVector<ScoredApp> scoredApps;
        for (const auto &app : m_allApps) {
            int score = matchScore(app.name, m_searchQuery);
            if (score >= 0) {
                scoredApps.append({app, score});
            }
        }
        
        // Sort by score descending
        std::sort(scoredApps.begin(), scoredApps.end(), [](const ScoredApp &a, const ScoredApp &b) {
            return a.score > b.score;
        });
        
        for (const auto &sa : scoredApps) {
            m_filteredApps.append(sa.app);
        }
    }
    endResetModel();
}

AppIconProvider::AppIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap AppIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    int width = requestedSize.width() > 0 ? requestedSize.width() : 64;
    int height = requestedSize.height() > 0 ? requestedSize.height() : 64;
    
    QIcon icon;
    if (id.startsWith("/")) {
        icon = QIcon(id);
    } else {
        icon = QIcon::fromTheme(id);
    }
    
    if (icon.isNull()) {
        int lastDot = id.lastIndexOf('.');
        if (lastDot > 0) {
            icon = QIcon::fromTheme(id.left(lastDot));
        }
    }
    
    if (size) {
        *size = QSize(width, height);
    }
    
    if (icon.isNull()) {
        QPixmap empty(width, height);
        empty.fill(Qt::transparent);
        return empty;
    }
    
    return icon.pixmap(width, height);
}

