#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include "data/Models.h"

namespace DesktopApp {

/**
 * @brief Simple message widget for basic chat display
 * 
 * This is a simplified replacement for EnhancedMessageWidget
 * to avoid crashes while maintaining basic functionality.
 */
class SimpleMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleMessageWidget(const Message &message, QWidget *parent = nullptr);
    
    void updateContent(const QString &content);
    void setStreaming(bool streaming);
    void setGenerating(bool generating);
    void appendContent(const QString &content);
    
    const Message& message() const { return m_message; }

signals:
    void copyRequested(const QString &text);

private:
    void setupUI();
    void updateStyling();
    
    Message m_message;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QLabel *m_senderLabel;
    QLabel *m_timestampLabel;
    QTextEdit *m_contentText;
    QLabel *m_statusLabel;
    
    bool m_isStreaming;
    bool m_isGenerating;
};

} // namespace DesktopApp