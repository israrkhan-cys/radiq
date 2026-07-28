#include "ConfigStore.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

ConfigStore::ConfigStore(QObject *parent) : QObject(parent) {
    // Determine path
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    m_configFilePath = QDir(configDir).filePath("radiq/config.json");

    // Load or create defaults
    loadConfig();
}

QString ConfigStore::configFilePath() const {
    return m_configFilePath;
}

void ConfigStore::loadConfig() {
    // Set default values in memory first
    m_pinnedApps = {
        "firefox.desktop",
        "Alacritty.desktop",
        "org.kde.dolphin.desktop",
        "code.desktop",
        "discord.desktop",
        "gimp.desktop",
        "btop.desktop",
        "com.obsproject.Studio.desktop"
    };
    m_hotkeyDisplay = "SUPER + A";
    m_ringOuterRadius = 230;
    m_ringInnerRadius = 100;
    m_iconSize = 56;
    m_wedgeColor = "#CC1F2335";
    m_wedgeHighlightColor = "#2D3F76";
    m_borderColor = "#414868";
    m_borderHighlightColor = "#7AA2F7";
    m_scrimColor = "#CC0F0F15";
    m_blurAmount = 10;
    m_animationDurationMs = 130;
    m_fontFamily = "Inter, Roboto, sans-serif";

    QFile file(m_configFilePath);
    if (!file.exists()) {
        qInfo() << "Config file does not exist. Creating default config at:" << m_configFilePath;
        saveDefaultConfig();
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open config file for reading:" << m_configFilePath << ". Using defaults.";
        return;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Malformed JSON in config file at:" << m_configFilePath 
                   << "Error:" << parseError.errorString() << ". Using defaults.";
        return;
    }

    qInfo() << "Loading configuration from:" << m_configFilePath;
    QJsonObject obj = doc.object();

    if (obj.contains("pinnedApps")) {
        QJsonArray arr = obj["pinnedApps"].toArray();
        QStringList list;
        for (const auto &val : arr) {
            list.append(val.toString());
        }
        m_pinnedApps = list;
    } else {
        qWarning() << "Config missing key: 'pinnedApps'. Using default.";
    }

    if (obj.contains("hotkeyDisplay")) {
        m_hotkeyDisplay = obj["hotkeyDisplay"].toString();
    } else {
        qWarning() << "Config missing key: 'hotkeyDisplay'. Using default.";
    }

    if (obj.contains("ringOuterRadius")) {
        m_ringOuterRadius = obj["ringOuterRadius"].toInt();
    } else {
        qWarning() << "Config missing key: 'ringOuterRadius'. Using default.";
    }

    if (obj.contains("ringInnerRadius")) {
        m_ringInnerRadius = obj["ringInnerRadius"].toInt();
    } else {
        qWarning() << "Config missing key: 'ringInnerRadius'. Using default.";
    }

    if (obj.contains("iconSize")) {
        m_iconSize = obj["iconSize"].toInt();
    } else {
        qWarning() << "Config missing key: 'iconSize'. Using default.";
    }

    if (obj.contains("wedgeColor")) {
        m_wedgeColor = obj["wedgeColor"].toString();
    } else {
        qWarning() << "Config missing key: 'wedgeColor'. Using default.";
    }

    if (obj.contains("wedgeHighlightColor")) {
        m_wedgeHighlightColor = obj["wedgeHighlightColor"].toString();
    } else {
        qWarning() << "Config missing key: 'wedgeHighlightColor'. Using default.";
    }

    if (obj.contains("borderColor")) {
        m_borderColor = obj["borderColor"].toString();
    } else {
        qWarning() << "Config missing key: 'borderColor'. Using default.";
    }

    if (obj.contains("borderHighlightColor")) {
        m_borderHighlightColor = obj["borderHighlightColor"].toString();
    } else {
        qWarning() << "Config missing key: 'borderHighlightColor'. Using default.";
    }

    if (obj.contains("scrimColor")) {
        m_scrimColor = obj["scrimColor"].toString();
    } else {
        qWarning() << "Config missing key: 'scrimColor'. Using default.";
    }

    if (obj.contains("blurAmount")) {
        m_blurAmount = obj["blurAmount"].toInt();
    } else {
        qWarning() << "Config missing key: 'blurAmount'. Using default.";
    }

    if (obj.contains("animationDurationMs")) {
        m_animationDurationMs = obj["animationDurationMs"].toInt();
    } else {
        qWarning() << "Config missing key: 'animationDurationMs'. Using default.";
    }

    if (obj.contains("fontFamily")) {
        m_fontFamily = obj["fontFamily"].toString();
    } else {
        qWarning() << "Config missing key: 'fontFamily'. Using default.";
    }
}

void ConfigStore::saveDefaultConfig() {
    QJsonObject obj;
    QJsonArray arr;
    for (const auto &app : m_pinnedApps) {
        arr.append(app);
    }
    obj["pinnedApps"] = arr;
    obj["hotkeyDisplay"] = m_hotkeyDisplay;
    obj["ringOuterRadius"] = m_ringOuterRadius;
    obj["ringInnerRadius"] = m_ringInnerRadius;
    obj["iconSize"] = m_iconSize;
    obj["wedgeColor"] = m_wedgeColor;
    obj["wedgeHighlightColor"] = m_wedgeHighlightColor;
    obj["borderColor"] = m_borderColor;
    obj["borderHighlightColor"] = m_borderHighlightColor;
    obj["scrimColor"] = m_scrimColor;
    obj["blurAmount"] = m_blurAmount;
    obj["animationDurationMs"] = m_animationDurationMs;
    obj["fontFamily"] = m_fontFamily;

    QFileInfo info(m_configFilePath);
    QDir().mkpath(info.absolutePath());

    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open config file for writing:" << m_configFilePath;
        return;
    }

    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void ConfigStore::setPinnedApps(const QStringList &val) {
    if (m_pinnedApps == val) return;
    m_pinnedApps = val;
    emit pinnedAppsChanged();
}
void ConfigStore::setHotkeyDisplay(const QString &val) {
    if (m_hotkeyDisplay == val) return;
    m_hotkeyDisplay = val;
    emit hotkeyDisplayChanged();
}
void ConfigStore::setRingOuterRadius(int val) {
    if (m_ringOuterRadius == val) return;
    m_ringOuterRadius = val;
    emit ringOuterRadiusChanged();
}
void ConfigStore::setRingInnerRadius(int val) {
    if (m_ringInnerRadius == val) return;
    m_ringInnerRadius = val;
    emit ringInnerRadiusChanged();
}
void ConfigStore::setIconSize(int val) {
    if (m_iconSize == val) return;
    m_iconSize = val;
    emit iconSizeChanged();
}
void ConfigStore::setWedgeColor(const QString &val) {
    if (m_wedgeColor == val) return;
    m_wedgeColor = val;
    emit wedgeColorChanged();
}
void ConfigStore::setWedgeHighlightColor(const QString &val) {
    if (m_wedgeHighlightColor == val) return;
    m_wedgeHighlightColor = val;
    emit wedgeHighlightColorChanged();
}
void ConfigStore::setBorderColor(const QString &val) {
    if (m_borderColor == val) return;
    m_borderColor = val;
    emit borderColorChanged();
}
void ConfigStore::setBorderHighlightColor(const QString &val) {
    if (m_borderHighlightColor == val) return;
    m_borderHighlightColor = val;
    emit borderHighlightColorChanged();
}
void ConfigStore::setScrimColor(const QString &val) {
    if (m_scrimColor == val) return;
    m_scrimColor = val;
    emit scrimColorChanged();
}
void ConfigStore::setBlurAmount(int val) {
    if (m_blurAmount == val) return;
    m_blurAmount = val;
    emit blurAmountChanged();
}
void ConfigStore::setAnimationDurationMs(int val) {
    if (m_animationDurationMs == val) return;
    m_animationDurationMs = val;
    emit animationDurationMsChanged();
}
void ConfigStore::setFontFamily(const QString &val) {
    if (m_fontFamily == val) return;
    m_fontFamily = val;
    emit fontFamilyChanged();
}
