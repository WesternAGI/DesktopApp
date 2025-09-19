#include "SimpleMessageWidget.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>

namespace DesktopApp {

SimpleMessageWidget::SimpleMessageWidget(const Message &message, QWidget *parent)
    : QWidget(parent)
    , m_message(message)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_senderLabel(nullptr)
    , m_timestampLabel(nullptr)
    , m_contentText(nullptr)
    , m_statusLabel(nullptr)
    , m_isStreaming(false)
    , m_isGenerating(false)
{
    qDebug() << "SimpleMessageWidget constructor called for message:" << message.id;
    setupUI();
    updateStyling();
    qDebug() << "SimpleMessageWidget constructor completed";
}

void SimpleMessageWidget::setupUI()
{
    qDebug() << "SimpleMessageWidget::setupUI() called";
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 8, 12, 8);
    m_mainLayout->setSpacing(4);

    // Header with sender and timestamp
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(8);

    m_senderLabel = new QLabel();
    m_senderLabel->setStyleSheet("font-weight: bold; color: #374151;");
    
    if (m_message.role == MessageRole::User) {
        m_senderLabel->setText("You");
    } else {
        m_senderLabel->setText("Assistant");
    }
    
    m_headerLayout->addWidget(m_senderLabel);

    m_timestampLabel = new QLabel();
    m_timestampLabel->setStyleSheet("color: #9CA3AF; font-size: 12px;");
    m_timestampLabel->setText(m_message.createdAt.toString("hh:mm"));
    m_headerLayout->addWidget(m_timestampLabel);
    
    m_headerLayout->addStretch();
    m_mainLayout->addLayout(m_headerLayout);

    // Content area
    m_contentText = new QTextEdit();
    m_contentText->setReadOnly(true);
    m_contentText->setPlainText(m_message.text);
    m_contentText->setMinimumHeight(40);
    m_contentText->setMaximumHeight(400);
    m_contentText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_contentText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainLayout->addWidget(m_contentText);

    // Status label
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #6B7280; font-size: 11px;");
    m_statusLabel->hide(); // Hidden by default
    m_mainLayout->addWidget(m_statusLabel);
    
    qDebug() << "SimpleMessageWidget::setupUI() completed";
}

void SimpleMessageWidget::updateStyling()
{
    // Simple styling based on message role
    if (m_message.role == MessageRole::User) {
        setStyleSheet(R"(
            SimpleMessageWidget {
                background-color: #EBF4FF;
                border-left: 3px solid #3B82F6;
                border-radius: 8px;
                margin: 4px 0px;
            }
            QTextEdit {
                background-color: transparent;
                border: none;
                font-size: 14px;
            }
        )");
    } else {
        setStyleSheet(R"(
            SimpleMessageWidget {
                background-color: #F9FAFB;
                border-left: 3px solid #10B981;
                border-radius: 8px;
                margin: 4px 0px;
            }
            QTextEdit {
                background-color: transparent;
                border: none;
                font-size: 14px;
            }
        )");
    }
}

void SimpleMessageWidget::updateContent(const QString &content)
{
    qDebug() << "SimpleMessageWidget::updateContent() called";
    m_message.text = content;
    m_contentText->setPlainText(content);
}

void SimpleMessageWidget::setStreaming(bool streaming)
{
    qDebug() << "SimpleMessageWidget::setStreaming(" << streaming << ")";
    m_isStreaming = streaming;
    
    if (streaming) {
        m_statusLabel->setText("Generating...");
        m_statusLabel->show();
    } else {
        m_statusLabel->hide();
    }
}

void SimpleMessageWidget::setGenerating(bool generating)
{
    qDebug() << "SimpleMessageWidget::setGenerating(" << generating << ")";
    m_isGenerating = generating;
    
    if (generating) {
        m_statusLabel->setText("Thinking...");
        m_statusLabel->show();
    } else {
        m_statusLabel->hide();
    }
}

void SimpleMessageWidget::appendContent(const QString &content)
{
    qDebug() << "SimpleMessageWidget::appendContent() called with:" << content.left(50);
    m_message.text += content;
    m_contentText->setPlainText(m_message.text);
    
    // Auto-scroll to bottom
    QTextCursor cursor = m_contentText->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_contentText->setTextCursor(cursor);
}

} // namespace DesktopApp