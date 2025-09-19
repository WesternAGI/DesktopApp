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
    setMinimumHeight(60);
    setMaximumHeight(120);
    
    // Simple main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(4);

    // Simple input area
    QWidget *inputWidget = new QWidget();
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(4, 4, 4, 4);
    inputLayout->setSpacing(4);

    // Simple text input
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("Type your message...");
    m_textEdit->setMinimumHeight(40);
    m_textEdit->setMaximumHeight(80);
    m_textEdit->setAcceptRichText(false);
    inputLayout->addWidget(m_textEdit, 1);

    // Simple send button
    m_sendButton = new QPushButton("Send");
    m_sendButton->setFixedSize(60, 40);
    m_sendButton->setEnabled(false);
    inputLayout->addWidget(m_sendButton);

    m_mainLayout->addWidget(inputWidget);

    // Simple status area
    QWidget *statusWidget = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);

    // Simple provider selector 
    m_providerCombo = new QComboBox();
    m_providerCombo->addItem("Echo Provider", "echo");
    m_providerCombo->addItem("Backend AI", "backend_ai");
    statusLayout->addWidget(m_providerCombo);

    statusLayout->addStretch();
    m_mainLayout->addWidget(statusWidget);

    // Set simple styling
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
    // Simple styling only
    setStyleSheet("QTextEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }");
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
    
    if (text.isEmpty()) {
        return;
    }

    // Simple emit without complex checks
    emit messageSent(text, QList<Attachment>());
    m_textEdit->clear();
}

} // namespace DesktopApp