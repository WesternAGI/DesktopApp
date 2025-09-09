#include "IconRegistry.h"
#include <QSvgRenderer>
#include <QPainter>
#include <QDebug>

namespace DesktopApp {

IconRegistry::IconRegistry(QObject *parent)
    : QObject(parent)
{
    loadDefaultIcons();
}

void IconRegistry::registerIcon(const QString& name, const QString& svgData)
{
    IconData iconData;
    iconData.svgData = svgData;
    iconData.renderer = std::make_unique<QSvgRenderer>(svgData.toUtf8());
    
    if (!iconData.renderer->isValid()) {
        qWarning() << "Invalid SVG data for icon:" << name;
        return;
    }
    
    m_icons[name] = std::move(iconData);
    qDebug() << "Registered icon:" << name;
}

QIcon IconRegistry::icon(const QString& name) const
{
    if (!hasIcon(name)) {
        qWarning() << "Icon not found:" << name;
        return QIcon();
    }

    // Create pixmaps for different sizes
    QIcon icon;
    QList<int> sizes = {16, 24, 32, 48, 64};
    
    for (int size : sizes) {
        QPixmap pixmap = this->pixmap(name, QSize(size, size));
        if (!pixmap.isNull()) {
            icon.addPixmap(pixmap);
        }
    }
    
    return icon;
}

QPixmap IconRegistry::pixmap(const QString& name, const QSize& size) const
{
    if (!hasIcon(name)) {
        return QPixmap();
    }

    const IconData& iconData = m_icons[name];
    
    // Create renderer if not exists
    if (!iconData.renderer) {
        iconData.renderer = std::make_unique<QSvgRenderer>(iconData.svgData.toUtf8());
    }

    if (!iconData.renderer->isValid()) {
        return QPixmap();
    }

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    iconData.renderer->render(&painter);
    
    return pixmap;
}

bool IconRegistry::hasIcon(const QString& name) const
{
    return m_icons.contains(name);
}

void IconRegistry::loadDefaultIcons()
{
    // Chat and conversation icons
    registerIcon("chat", getDefaultIcon("chat"));
    // Circular chat bubble used by some providers
    registerIcon("message-circle", getDefaultIcon("message-circle"));
    registerIcon("new-chat", getDefaultIcon("new-chat"));
    registerIcon("delete", getDefaultIcon("delete"));
    registerIcon("edit", getDefaultIcon("edit"));
    registerIcon("pin", getDefaultIcon("pin"));
    registerIcon("archive", getDefaultIcon("archive"));
    
    // Message and composer icons
    registerIcon("send", getDefaultIcon("send"));
    registerIcon("attach", getDefaultIcon("attach"));
    registerIcon("microphone", getDefaultIcon("microphone"));
    registerIcon("stop", getDefaultIcon("stop"));
    registerIcon("copy", getDefaultIcon("copy"));
    
    // UI control icons
    registerIcon("search", getDefaultIcon("search"));
    registerIcon("settings", getDefaultIcon("settings"));
    registerIcon("menu", getDefaultIcon("menu"));
    registerIcon("close", getDefaultIcon("close"));
    registerIcon("minimize", getDefaultIcon("minimize"));
    registerIcon("maximize", getDefaultIcon("maximize"));
    
    // Theme icons
    registerIcon("light-mode", getDefaultIcon("light-mode"));
    registerIcon("dark-mode", getDefaultIcon("dark-mode"));
    
    // Provider icons
    registerIcon("provider", getDefaultIcon("provider"));
    registerIcon("model", getDefaultIcon("model"));
    
    emit iconsLoaded();
    qDebug() << "Loaded" << m_icons.size() << "default icons";
}

QString IconRegistry::getDefaultIcon(const QString& name) const
{
    // Simple SVG icons - these are original designs, not copied from any source
    if (name == "chat") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M20 2H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h14l4 4V4c0-1.1-.9-2-2-2zm-2 12H6v-2h12v2zm0-3H6V9h12v2zm0-3H6V6h12v2z"/>
        </svg>)";
    }

    if (name == "message-circle") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2C6.477 2 2 5.977 2 10.9c0 2.41 1.192 4.566 3.127 6.077-.165 1.097-.63 2.408-1.703 3.48-.2.2-.067.543.22.545 1.82.012 3.26-.544 4.292-1.126C9.23 20.433 10.58 20.8 12 20.8c5.523 0 10-3.978 10-8.9S17.523 2 12 2z"/>
        </svg>)";
    }
    
    if (name == "new-chat") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
        </svg>)";
    }
    
    if (name == "send") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M2.01 21L23 12 2.01 3 2 10l15 2-15 2z"/>
        </svg>)";
    }
    
    if (name == "search") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
        </svg>)";
    }
    
    if (name == "settings") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M19.14,12.94c0.04-0.3,0.06-0.61,0.06-0.94c0-0.32-0.02-0.64-0.07-0.94l2.03-1.58c0.18-0.14,0.23-0.41,0.12-0.61 l-1.92-3.32c-0.12-0.22-0.37-0.29-0.59-0.22l-2.39,0.96c-0.5-0.38-1.03-0.7-1.62-0.94L14.4,2.81c-0.04-0.24-0.24-0.41-0.48-0.41 h-3.84c-0.24,0-0.43,0.17-0.47,0.41L9.25,5.35C8.66,5.59,8.12,5.92,7.63,6.29L5.24,5.33c-0.22-0.08-0.47,0-0.59,0.22L2.74,8.87 C2.62,9.08,2.66,9.34,2.86,9.48l2.03,1.58C4.84,11.36,4.8,11.69,4.8,12s0.02,0.64,0.07,0.94l-2.03,1.58 c-0.18,0.14-0.23,0.41-0.12,0.61l1.92,3.32c0.12,0.22,0.37,0.29,0.59,0.22l2.39-0.96c0.5,0.38,1.03,0.7,1.62,0.94l0.36,2.54 c0.05,0.24,0.24,0.41,0.48,0.41h3.84c0.24,0,0.44-0.17,0.47-0.41l0.36-2.54c0.59-0.24,1.13-0.56,1.62-0.94l2.39,0.96 c0.22,0.08,0.47,0,0.59-0.22l1.92-3.32c0.12-0.22,0.07-0.47-0.12-0.61L19.14,12.94z M12,15.6c-1.98,0-3.6-1.62-3.6-3.6 s1.62-3.6,3.6-3.6s3.6,1.62,3.6,3.6S13.98,15.6,12,15.6z"/>
        </svg>)";
    }
    
    if (name == "attach") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M2 12.5C2 9.46 4.46 7 7.5 7H18c2.21 0 4 1.79 4 4s-1.79 4-4 4H9.5C8.12 15 7 13.88 7 12.5S8.12 10 9.5 10H17v2H9.41c-.55 0-.55 1 0 1H18c1.1 0 2-.9 2-2s-.9-2-2-2H7.5C5.57 9 4 10.57 4 12.5S5.57 16 7.5 16H17v2H7.5C4.46 18 2 15.54 2 12.5z"/>
        </svg>)";
    }
    
    if (name == "microphone") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M12 14c1.66 0 2.99-1.34 2.99-3L15 5c0-1.66-1.34-3-3-3S9 3.34 9 5v6c0 1.66 1.34 3 3 3zm5.3-3c0 3-2.54 5.1-5.3 5.1S6.7 14 6.7 11H5c0 3.41 2.72 6.23 6 6.72V21h2v-3.28c3.28-.48 6-3.3 6-6.72h-1.7z"/>
        </svg>)";
    }
    
    if (name == "stop") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M6 6h12v12H6z"/>
        </svg>)";
    }
    
    if (name == "delete") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
        </svg>)";
    }
    
    if (name == "edit") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M3 17.25V21h3.75L17.81 9.94l-3.75-3.75L3 17.25zM20.71 7.04c.39-.39.39-1.02 0-1.41l-2.34-2.34c-.39-.39-1.02-.39-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"/>
        </svg>)";
    }
    
    if (name == "light-mode") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M12,18c-3.3,0-6-2.7-6-6s2.7-6,6-6s6,2.7,6,6S15.3,18,12,18zM12,8c-2.2,0-4,1.8-4,4c0,2.2,1.8,4,4,4c2.2,0,4-1.8,4-4C16,9.8,14.2,8,12,8z"/>
            <path d="M12,4c-0.6,0-1-0.4-1-1V1c0-0.6,0.4-1,1-1s1,0.4,1,1v2C13,3.6,12.6,4,12,4z"/>
            <path d="M12,24c-0.6,0-1-0.4-1-1v-2c0-0.6,0.4-1,1-1s1,0.4,1,1v2C13,23.6,12.6,24,12,24z"/>
            <path d="M5.6,6.6c-0.3,0-0.5-0.1-0.7-0.3L3.5,4.9c-0.4-0.4-0.4-1,0-1.4s1-0.4,1.4,0l1.4,1.4c0.4,0.4,0.4,1,0,1.4C6.1,6.5,5.9,6.6,5.6,6.6z"/>
        </svg>)";
    }
    
    if (name == "dark-mode") {
        return R"(<svg viewBox="0 0 24 24" fill="currentColor">
            <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/>
        </svg>)";
    }
    
    // Default fallback
    return R"(<svg viewBox="0 0 24 24" fill="currentColor">
        <circle cx="12" cy="12" r="10"/>
    </svg>)";
}

} // namespace DesktopApp
