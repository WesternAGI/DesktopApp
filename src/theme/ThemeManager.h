#pragma once

#include <QObject>
#include <QColor>
#include <QFont>
#include <QHash>
#include <QVariant>

namespace GadAI {

/**
 * @brief Design token structure for theming system
 */
struct DesignTokens {
    // Colors
    QColor primary;
    QColor primaryHover;
    QColor secondary;
    QColor background;
    QColor surface;
    QColor surfaceHover;
    QColor border;
    QColor text;
    QColor textSecondary;
    QColor textMuted;
    QColor success;
    QColor warning;
    QColor error;
    
    // Typography
    QFont fontFamily;
    int fontSizeSmall;
    int fontSizeNormal;
    int fontSizeLarge;
    int fontSizeHeading;
    
    // Spacing
    int spacingXs;
    int spacingS;
    int spacingM;
    int spacingL;
    int spacingXl;
    
    // Radii
    int radiusSmall;
    int radiusMedium;
    int radiusLarge;
    
    // Shadows
    QString shadowLight;
    QString shadowMedium;
    QString shadowHeavy;
};

/**
 * @brief Theme manager handling light/dark themes and design tokens
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        Light,
        Dark
    };

    explicit ThemeManager(QObject *parent = nullptr);

    /**
     * @brief Set the current theme
     */
    void setTheme(const QString& themeName);
    void setTheme(Theme theme);

    /**
     * @brief Get current theme information
     */
    Theme currentTheme() const { return m_currentTheme; }
    QString currentThemeString() const;

    /**
     * @brief Get design tokens for current theme
     */
    const DesignTokens& tokens() const { return m_tokens; }

    /**
     * @brief Get a color by name
     */
    QColor color(const QString& name) const;

    /**
     * @brief Get stylesheet for the current theme
     */
    QString stylesheet() const;

    /**
     * @brief Apply theme to the entire application
     */
    void applyToApplication();

signals:
    void themeChanged();

private:
    void loadLightTheme();
    void loadDarkTheme();
    void generateStylesheet();

    Theme m_currentTheme;
    DesignTokens m_tokens;
    QString m_stylesheet;
    QHash<QString, QColor> m_colorMap;
};

} // namespace GadAI
