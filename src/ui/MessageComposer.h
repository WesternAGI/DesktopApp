#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include "data/Models.h"

namespace DesktopApp {

/**
 * @brief Simple message composer widget
 */
class MessageComposer : public QWidget
{
    Q_OBJECT

public:
    explicit MessageComposer(QWidget *parent = nullptr);

    void setFocus();
    void clear();
    void setCurrentProvider(const QString &providerId);

signals:
    void messageSent(const QString &text, const AttachmentList &attachments);
    void providerChanged(const QString &providerId);

private slots:
    void onSendClicked();

private:
    void setupUI();
    void connectSignals();
    void updateStyling();

    // Simple UI Components
    QVBoxLayout *m_mainLayout;
    QTextEdit *m_textEdit;
    QPushButton *m_sendButton;
};

} // namespace DesktopApp
