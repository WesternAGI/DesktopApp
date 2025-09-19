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
    , m_providerCombo(nullptr)
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

    // Status area with better spacing
    QWidget *statusWidget = new QWidget();
    statusWidget->setObjectName("statusWidget");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(8);

    // Provider selector with better styling
    m_providerCombo = new QComboBox();
    m_providerCombo->addItem("Echo Provider", "echo");
    m_providerCombo->addItem("Backend AI", "backend_ai");
    m_providerCombo->setCursor(Qt::PointingHandCursor);
    statusLayout->addWidget(m_providerCombo);

    statusLayout->addStretch();
    m_mainLayout->addWidget(statusWidget);

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

    // Simple provider selection
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        QString providerId = m_providerCombo->itemData(index).toString();
        emit providerChanged(providerId);
    });
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
    for (int i = 0; i < m_providerCombo->count(); ++i) {
        if (m_providerCombo->itemData(i).toString() == providerId) {
            m_providerCombo->setCurrentIndex(i);
            break;
        }
    }
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