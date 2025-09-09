#pragma once

#include <QObject>
#include <QHash>
#include <QSvgRenderer>
#include <QPixmap>
#include <QIcon>
#include <memory>

namespace GadAI {

/**
 * @brief Icon registry managing SVG icons with theme support
 */
class IconRegistry : public QObject
{
    Q_OBJECT

public:
    explicit IconRegistry(QObject *parent = nullptr);

    /**
     * @brief Register an SVG icon
     */
    void registerIcon(const QString& name, const QString& svgData);

    /**
     * @brief Get an icon by name
     */
    QIcon icon(const QString& name) const;

    /**
     * @brief Get a pixmap for an icon at a specific size
     */
    QPixmap pixmap(const QString& name, const QSize& size) const;

    /**
     * @brief Check if an icon exists
     */
    bool hasIcon(const QString& name) const;

    /**
     * @brief Load default icons
     */
    void loadDefaultIcons();

signals:
    void iconsLoaded();

private:
    struct IconData {
        QString svgData;
        mutable std::shared_ptr<QSvgRenderer> renderer; // copyable
    };

    mutable QHash<QString, IconData> m_icons;
    
    QString getDefaultIcon(const QString& name) const;
};

} // namespace GadAI
