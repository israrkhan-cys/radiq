#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class ConfigStore final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList pinnedApps READ pinnedApps WRITE setPinnedApps NOTIFY pinnedAppsChanged)
    Q_PROPERTY(QString hotkeyDisplay READ hotkeyDisplay WRITE setHotkeyDisplay NOTIFY hotkeyDisplayChanged)
    Q_PROPERTY(int ringOuterRadius READ ringOuterRadius WRITE setRingOuterRadius NOTIFY ringOuterRadiusChanged)
    Q_PROPERTY(int ringInnerRadius READ ringInnerRadius WRITE setRingInnerRadius NOTIFY ringInnerRadiusChanged)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(QString wedgeColor READ wedgeColor WRITE setWedgeColor NOTIFY wedgeColorChanged)
    Q_PROPERTY(QString wedgeHighlightColor READ wedgeHighlightColor WRITE setWedgeHighlightColor NOTIFY wedgeHighlightColorChanged)
    Q_PROPERTY(QString borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QString borderHighlightColor READ borderHighlightColor WRITE setBorderHighlightColor NOTIFY borderHighlightColorChanged)
    Q_PROPERTY(int blurAmount READ blurAmount WRITE setBlurAmount NOTIFY blurAmountChanged)
    Q_PROPERTY(int animationDurationMs READ animationDurationMs WRITE setAnimationDurationMs NOTIFY animationDurationMsChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)

public:
    explicit ConfigStore(QObject *parent = nullptr);

    QStringList pinnedApps() const { return m_pinnedApps; }
    void setPinnedApps(const QStringList &val);

    QString hotkeyDisplay() const { return m_hotkeyDisplay; }
    void setHotkeyDisplay(const QString &val);

    int ringOuterRadius() const { return m_ringOuterRadius; }
    void setRingOuterRadius(int val);

    int ringInnerRadius() const { return m_ringInnerRadius; }
    void setRingInnerRadius(int val);

    int iconSize() const { return m_iconSize; }
    void setIconSize(int val);

    QString wedgeColor() const { return m_wedgeColor; }
    void setWedgeColor(const QString &val);

    QString wedgeHighlightColor() const { return m_wedgeHighlightColor; }
    void setWedgeHighlightColor(const QString &val);

    QString borderColor() const { return m_borderColor; }
    void setBorderColor(const QString &val);

    QString borderHighlightColor() const { return m_borderHighlightColor; }
    void setBorderHighlightColor(const QString &val);

    int blurAmount() const { return m_blurAmount; }
    void setBlurAmount(int val);

    int animationDurationMs() const { return m_animationDurationMs; }
    void setAnimationDurationMs(int val);

    QString fontFamily() const { return m_fontFamily; }
    void setFontFamily(const QString &val);

    QString configFilePath() const;

signals:
    void pinnedAppsChanged();
    void hotkeyDisplayChanged();
    void ringOuterRadiusChanged();
    void ringInnerRadiusChanged();
    void iconSizeChanged();
    void wedgeColorChanged();
    void wedgeHighlightColorChanged();
    void borderColorChanged();
    void borderHighlightColorChanged();
    void blurAmountChanged();
    void animationDurationMsChanged();
    void fontFamilyChanged();

private:
    void loadConfig();
    void saveDefaultConfig();

    QStringList m_pinnedApps;
    QString m_hotkeyDisplay;
    int m_ringOuterRadius = 230;
    int m_ringInnerRadius = 100;
    int m_iconSize = 56;
    QString m_wedgeColor;
    QString m_wedgeHighlightColor;
    QString m_borderColor;
    QString m_borderHighlightColor;
    int m_blurAmount = 10;
    int m_animationDurationMs = 130;
    QString m_fontFamily;

    QString m_configFilePath;
};
