#pragma once

#include <QObject>
#include <QColor>
#include <QFont>
#include <QHash>
#include <QVariant>

namespace DesktopApp {

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
    
    // Message-specific colors
    QColor userMessage;
    QColor assistantMessage;
    QColor systemMessage;
    QColor codeBackground;
    QColor codeBorder;
    QColor linkColor;
    QColor linkHover;
    
    // Typography
    QFont fontFamily;
    QFont fontFamilyMono;
    int fontSizeXs;       // 11px
    int fontSizeSmall;    // 12px
    int fontSizeNormal;   // 14px
    int fontSizeLarge;    // 16px
    int fontSizeHeading;  // 20px
    int fontSizeTitle;    // 24px
    
    // Line heights
    double lineHeightTight;   // 1.2
    double lineHeightNormal;  // 1.5
    double lineHeightLoose;   // 1.7
    
    // Spacing
    int spacingXs;   // 4px
    int spacingS;    // 8px
    int spacingM;    // 16px
    int spacingL;    // 24px
    int spacingXl;   // 32px
    int spacingXxl;  // 48px
    
    // Radii
    int radiusSmall;   // 6px
    int radiusMedium;  // 8px
    int radiusLarge;   // 12px
    int radiusXl;      // 16px
    
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

} // namespace DesktopApp
