#pragma once

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMenu>
#include "data/Models.h"

namespace DesktopApp {

/**
 * @brief Enhanced message widget with modern chat features
 * 
 * Features:
 * - Message actions (copy, regenerate, edit)
 * - Timestamps with relative formatting
 * - Better typography and spacing
 * - Syntax highlighting for code blocks
 * - Smooth animations
 * - Accessibility support
 */
class EnhancedMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnhancedMessageWidget(const Message& message, QWidget *parent = nullptr);
    
    void updateContent(const QString& content);
    void setStreaming(bool streaming);
    void setGenerating(bool generating);
    void setTimestamp(const QDateTime& timestamp);
    void showActions(bool show = true);
    
    const Message& message() const { return m_message; }
    bool isStreaming() const { return m_isStreaming; }

signals:
    void copyRequested(const QString& content);
    void regenerateRequested(const QString& messageId);
    void editRequested(const QString& messageId);
    void deleteRequested(const QString& messageId);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onCopyClicked();
    void onRegenerateClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onThemeChanged();

private:
    void setupUI();
    void setupActions();
    void updateStyling();
    void updateTimestamp();
    void animateActions(bool show);
    QString formatTimestamp(const QDateTime& timestamp) const;
    QString processMarkdown(const QString& content) const;
    
    // UI components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QHBoxLayout *m_actionsLayout;
    
    QLabel *m_authorLabel;
    QLabel *m_timestampLabel;
    QTextEdit *m_contentText;
    
    // Action buttons
    QWidget *m_actionsWidget;
    QPushButton *m_copyButton;
    QPushButton *m_regenerateButton;
    QPushButton *m_editButton;
    QPushButton *m_moreButton;
    QMenu *m_moreMenu;
    
    // Animation effects
    QGraphicsOpacityEffect *m_actionsOpacity;
    QPropertyAnimation *m_actionsAnimation;
    
    // Data
    Message m_message;
    bool m_isStreaming;
    bool m_isGenerating;
    bool m_actionsVisible;
    
    // Timers
    QTimer *m_timestampTimer;
    QTimer *m_streamingTimer;
};

} // namespace DesktopApp