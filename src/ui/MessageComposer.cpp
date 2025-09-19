#include "MessageComposer.h"
#include "core/Application.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>

namespace DesktopApp {

MessageComposer::MessageComposer(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_textEdit(nullptr)
    , m_sendButton(nullptr)
{
    setupUI();
    connectSignals();
}

void MessageComposer::setupUI()
{
    setMinimumHeight(80);
    setMaximumHeight(140);
    
    // Main layout with better margins
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 12, 16, 12);
    m_mainLayout->setSpacing(8);

    // Input area with improved spacing
    QWidget *inputWidget = new QWidget();
    inputWidget->setObjectName("inputWidget");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(12);

    // Better text input
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("Type your message...");
    m_textEdit->setMinimumHeight(44);
    m_textEdit->setMaximumHeight(100);
    m_textEdit->setAcceptRichText(false);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    inputLayout->addWidget(m_textEdit, 1);

    // Better send button
    m_sendButton = new QPushButton("Send");
    m_sendButton->setFixedSize(80, 44);
    m_sendButton->setEnabled(false);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    inputLayout->addWidget(m_sendButton);

    m_mainLayout->addWidget(inputWidget);

    // Apply improved styling
    updateStyling();
}

void MessageComposer::connectSignals()
{
    // Simple text input handling
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        bool hasText = !m_textEdit->toPlainText().trimmed().isEmpty();
        m_sendButton->setEnabled(hasText);
    });

    // Simple send button
    connect(m_sendButton, &QPushButton::clicked, this, &MessageComposer::onSendClicked);
}

void MessageComposer::updateStyling()
{
    // Better styling for modern chat interface
    setStyleSheet(R"(
        MessageComposer {
            background-color: #ffffff;
            border-top: 1px solid #e1e5ea;
        }
        
        QTextEdit {
            border: 1px solid #d1d5db;
            border-radius: 12px;
            padding: 12px 16px;
            font-size: 14px;
            background-color: #f9fafb;
            min-height: 20px;
        }
        
        QTextEdit:focus {
            border: 2px solid #3b82f6;
            background-color: #ffffff;
        }
        
        QPushButton {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 20px;
            font-weight: bold;
            font-size: 14px;
            padding: 10px 20px;
        }
        
        QPushButton:hover:enabled {
            background-color: #2563eb;
        }
        
        QPushButton:pressed:enabled {
            background-color: #1d4ed8;
        }
        
        QPushButton:disabled {
            background-color: #9ca3af;
            color: #ffffff;
        }
        
        QComboBox {
            border: 1px solid #d1d5db;
            border-radius: 6px;
            padding: 6px 12px;
            background-color: #f9fafb;
            font-size: 12px;
            min-width: 120px;
        }
        
        QComboBox:hover {
            border: 1px solid #3b82f6;
        }
        
        QComboBox:focus {
            border: 1px solid #3b82f6;
            background-color: #ffffff;
        }
        
        QComboBox QAbstractItemView {
            border: 1px solid #d1d5db;
            background-color: #ffffff;
            selection-background-color: #f3f4f6;
            selection-color: #111827;
        }
        
        QComboBox QAbstractItemView::item {
            padding: 8px 12px;
            border: none;
            background-color: transparent;
        }
        
        QComboBox QAbstractItemView::item:selected {
            background-color: #f3f4f6;
            color: #111827;
        }
        
        QComboBox QAbstractItemView::item:hover {
            background-color: #e5e7eb;
            color: #111827;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        
        QComboBox::down-arrow {
            width: 12px;
            height: 12px;
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iMTIiIHZpZXdCb3g9IjAgMCAxMiAxMiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTMgNC41TDYgNy41TDkgNC41IiBzdHJva2U9IiM2Qjc2ODAiIHN0cm9rZS13aWR0aD0iMS41IiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+);
        }
    )");
}

void MessageComposer::setFocus()
{
    m_textEdit->setFocus();
}

void MessageComposer::clear()
{
    m_textEdit->clear();
}

void MessageComposer::setCurrentProvider(const QString &providerId)
{
    // Provider selection removed - this method is now a no-op
    Q_UNUSED(providerId);
}

void MessageComposer::onSendClicked()
{
    QString text = m_textEdit->toPlainText().trimmed();
    
    qDebug() << "MessageComposer::onSendClicked() called with text:" << text;
    
    if (text.isEmpty()) {
        qDebug() << "Text is empty, returning";
        return;
    }

    qDebug() << "Emitting messageSent signal";
    // Simple emit without complex checks
    emit messageSent(text, QList<Attachment>());
    
    qDebug() << "Clearing text edit";
    m_textEdit->clear();
    qDebug() << "MessageComposer::onSendClicked() completed";
}

} // namespace DesktopApp