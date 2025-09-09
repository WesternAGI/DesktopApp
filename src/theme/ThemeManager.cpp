#include "ThemeManager.h"
#include <QApplication>
#include <QDebug>

namespace GadAI {

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(Light)
{
    // Load default light theme
    loadLightTheme();
    generateStylesheet();
    applyToApplication(); // Apply theme immediately upon construction
}

void ThemeManager::setTheme(const QString& themeName)
{
    if (themeName.toLower() == "dark") {
        setTheme(Dark);
    } else {
        setTheme(Light);
    }
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }

    m_currentTheme = theme;
    
    if (theme == Dark) {
        loadDarkTheme();
    } else {
        loadLightTheme();
    }
    
    generateStylesheet();
    applyToApplication();
    emit themeChanged();
}

QString ThemeManager::currentThemeString() const
{
    return m_currentTheme == Dark ? "dark" : "light";
}

QColor ThemeManager::color(const QString& name) const
{
    return m_colorMap.value(name, QColor());
}

QString ThemeManager::stylesheet() const
{
    return m_stylesheet;
}

void ThemeManager::applyToApplication()
{
    if (qApp) {
        qApp->setStyleSheet(m_stylesheet);
        qDebug() << "Applied" << currentThemeString() << "theme to application";
    }
}

void ThemeManager::loadLightTheme()
{
    // ChatGPT-like light theme colors
    m_tokens.primary = QColor("#19C37D");        // ChatGPT green
    m_tokens.primaryHover = QColor("#16A568");   // Darker green
    m_tokens.secondary = QColor("#8E8EA0");      // Subtle gray
    m_tokens.background = QColor("#FFFFFF");     // Pure white
    m_tokens.surface = QColor("#F7F7F8");        // Very light gray (ChatGPT background)
    m_tokens.surfaceHover = QColor("#ECECF1");   // Slightly darker for hover
    m_tokens.border = QColor("#D1D5DB");         // Light border
    m_tokens.text = QColor("#0D0D0D");           // Near black text
    m_tokens.textSecondary = QColor("#676767");  // Medium gray text
    m_tokens.textMuted = QColor("#8E8EA0");      // Muted text
    m_tokens.success = QColor("#19C37D");        // Success green
    m_tokens.warning = QColor("#FF8C00");        // Warning orange
    m_tokens.error = QColor("#FF4444");

    // Typography
    m_tokens.fontFamily = QFont("Segoe UI", 10);
    m_tokens.fontSizeSmall = 11;
    m_tokens.fontSizeNormal = 13;
    m_tokens.fontSizeLarge = 15;
    m_tokens.fontSizeHeading = 18;

    // Spacing (in pixels)
    m_tokens.spacingXs = 4;
    m_tokens.spacingS = 8;
    m_tokens.spacingM = 16;
    m_tokens.spacingL = 24;
    m_tokens.spacingXl = 32;

    // Radii
    m_tokens.radiusSmall = 4;
    m_tokens.radiusMedium = 8;
    m_tokens.radiusLarge = 12;

    // Shadows
    m_tokens.shadowLight = "0 1px 3px rgba(0, 0, 0, 0.1)";
    m_tokens.shadowMedium = "0 4px 6px rgba(0, 0, 0, 0.1)";
    m_tokens.shadowHeavy = "0 10px 15px rgba(0, 0, 0, 0.1)";

    // Update color map for easy access
    m_colorMap.clear();
    m_colorMap["primary"] = m_tokens.primary;
    m_colorMap["background"] = m_tokens.background;
    m_colorMap["surface"] = m_tokens.surface;
    m_colorMap["text"] = m_tokens.text;
    m_colorMap["border"] = m_tokens.border;
}

void ThemeManager::loadDarkTheme()
{
    // ChatGPT-like dark theme colors
    m_tokens.primary = QColor("#19C37D");        // ChatGPT green
    m_tokens.primaryHover = QColor("#16A568");   // Darker green
    m_tokens.secondary = QColor("#9CA3AF");      // Light gray
    m_tokens.background = QColor("#212121");     // ChatGPT dark background
    m_tokens.surface = QColor("#2F2F2F");        // Darker surface
    m_tokens.surfaceHover = QColor("#3C3C3C");   // Hover surface
    m_tokens.border = QColor("#4A4A4A");         // Medium border
    m_tokens.text = QColor("#ECECEC");           // Light text
    m_tokens.textSecondary = QColor("#C5C5D2");  // Secondary text
    m_tokens.textMuted = QColor("#9CA3AF");      // Muted text
    m_tokens.success = QColor("#19C37D");        // Success green
    m_tokens.warning = QColor("#FFA726");        // Warning orange
    m_tokens.error = QColor("#FF5252");

    // Typography (same as light)
    m_tokens.fontFamily = QFont("Segoe UI", 10);
    m_tokens.fontSizeSmall = 11;
    m_tokens.fontSizeNormal = 13;
    m_tokens.fontSizeLarge = 15;
    m_tokens.fontSizeHeading = 18;

    // Spacing (same as light)
    m_tokens.spacingXs = 4;
    m_tokens.spacingS = 8;
    m_tokens.spacingM = 16;
    m_tokens.spacingL = 24;
    m_tokens.spacingXl = 32;

    // Radii (same as light)
    m_tokens.radiusSmall = 4;
    m_tokens.radiusMedium = 8;
    m_tokens.radiusLarge = 12;

    // Shadows (adjusted for dark theme)
    m_tokens.shadowLight = "0 1px 3px rgba(0, 0, 0, 0.5)";
    m_tokens.shadowMedium = "0 4px 6px rgba(0, 0, 0, 0.5)";
    m_tokens.shadowHeavy = "0 10px 15px rgba(0, 0, 0, 0.5)";

    // Update color map
    m_colorMap.clear();
    m_colorMap["primary"] = m_tokens.primary;
    m_colorMap["background"] = m_tokens.background;
    m_colorMap["surface"] = m_tokens.surface;
    m_colorMap["text"] = m_tokens.text;
    m_colorMap["border"] = m_tokens.border;
}

void ThemeManager::generateStylesheet()
{
    // Create the stylesheet with proper argument substitution
    m_stylesheet = QString(R"(
/* Global ChatGPT-like Application Styles */
QWidget {
    background-color: %1;
    color: %2;
    font-family: "Segoe UI", Roboto, Ubuntu, Cantarell, "Noto Sans", sans-serif;
    font-size: 14px;
}

/* Main Window */
QMainWindow {
    background-color: %1;
    border: none;
}

/* Buttons - ChatGPT style */
QPushButton {
    background-color: %3;
    color: %2;
    border: 1px solid %4;
    border-radius: 8px;
    padding: 8px 16px;
    font-weight: 500;
    font-size: 14px;
}

QPushButton:hover {
    background-color: %5;
}

QPushButton:pressed {
    background-color: %4;
}

QPushButton:disabled {
    background-color: %4;
    color: %6;
}

/* Primary Button - ChatGPT green */
QPushButton[class="primary"] {
    background-color: %7;
    color: white;
    border: none;
    font-weight: 600;
}

QPushButton[class="primary"]:hover {
    background-color: %8;
}

/* Text Input - ChatGPT style */
QLineEdit, QTextEdit, QPlainTextEdit {
    background-color: %1;
    color: %2;
    border: 1px solid %4;
    border-radius: 8px;
    padding: 12px;
    font-size: 14px;
    selection-background-color: %7;
    selection-color: white;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
    border-color: %7;
    outline: none;
}

/* List Widgets - ChatGPT style */
QListWidget {
    background-color: %3;
    border: 1px solid %4;
    border-radius: 8px;
    outline: none;
}

QListWidget::item {
    background-color: transparent;
    color: %2;
    padding: 12px;
    border-bottom: 1px solid %4;
    border-radius: 6px;
    margin: 2px;
}

QListWidget::item:selected {
    background-color: %7;
    color: white;
}

QListWidget::item:hover {
    background-color: %5;
}

/* Scroll Bars - ChatGPT style */
QScrollBar:vertical {
    background-color: transparent;
    width: 8px;
    border: none;
    border-radius: 4px;
}

QScrollBar::handle:vertical {
    background-color: %4;
    border-radius: 4px;
    min-height: 20px;
    margin: 0px;
}

QScrollBar::handle:vertical:hover {
    background-color: %6;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

/* Menu Bar */
QMenuBar {
    background-color: %1;
    color: %2;
    border-bottom: 1px solid %4;
}

QMenuBar::item:selected {
    background-color: %5;
}

/* Status Bar */
QStatusBar {
    background-color: %3;
    color: %6;
    border-top: 1px solid %4;
}

/* Splitter */
QSplitter::handle {
    background-color: %4;
}

QSplitter::handle:horizontal {
    width: 1px;
}

QSplitter::handle:vertical {
    height: 1px;
}
)")
        .arg(m_tokens.background.name())      // %1 - background
        .arg(m_tokens.text.name())            // %2 - text
        .arg(m_tokens.surface.name())         // %3 - surface
        .arg(m_tokens.border.name())          // %4 - border
        .arg(m_tokens.surfaceHover.name())    // %5 - surface hover
        .arg(m_tokens.textMuted.name())       // %6 - muted text
        .arg(m_tokens.primary.name())         // %7 - primary
        .arg(m_tokens.primaryHover.name());   // %8 - primary hover
}

} // namespace GadAI
