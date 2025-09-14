#include "MessageThreadWidget.h"
#include "core/Application.h"
#include "data/JsonStore.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"
#include "providers/ProviderManager.h"
#include "providers/EchoProvider.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollBar>
#include <QClipboard>
#include <QApplication>
#include <QTimer>
#include <QTextCursor>
#include <QDebug>
#include <QEnterEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QRandomGenerator>
#include "SimpleMarkdown.h"

namespace DesktopApp {

MessageThreadWidget::MessageThreadWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_messagesContainer(nullptr)
    , m_messagesLayout(nullptr)
    , m_emptyLabel(nullptr)
    , m_streamingMessageWidget(nullptr)
    , m_loadingDotsWidget(nullptr)
    , m_streamingTimer(new QTimer(this))
    , m_streamingPosition(0)
    , m_providerManager(nullptr)
{
    setupUI();
    connectSignals();
    showEmptyState();
    updateOfflineNotice();
}

// LoadingDotsWidget implementation
LoadingDotsWidget::LoadingDotsWidget(QWidget *parent)
    : QWidget(parent)
    , m_dotsLabel(new QLabel(this))
    , m_animationTimer(new QTimer(this))
    , m_currentState(0)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->addStretch(); // Push dots to left
    layout->addWidget(m_dotsLabel);
    layout->addStretch(); // Add stretch after dots too for centering in bubble
    
    m_dotsLabel->setText("●●●");
    m_dotsLabel->setStyleSheet(R"(
        QLabel {
            color: #999999;
            font-size: 16px;
            background-color: #F0F0F0;
            border-radius: 20px;
            padding: 12px 16px;
            margin: 2px;
        }
    )");
    
    m_animationTimer->setInterval(600); // Change dots every 600ms
    connect(m_animationTimer, &QTimer::timeout, this, &LoadingDotsWidget::updateDots);
}

void LoadingDotsWidget::startAnimation()
{
    m_currentState = 0;
    updateDots();
    m_animationTimer->start();
}

void LoadingDotsWidget::stopAnimation()
{
    m_animationTimer->stop();
}

void LoadingDotsWidget::updateDots()
{
    switch (m_currentState) {
        case 0: m_dotsLabel->setText("●●●"); break;
        case 1: m_dotsLabel->setText("●●"); break;
        case 2: m_dotsLabel->setText("●"); break;
    }
    m_currentState = (m_currentState + 1) % 3;
}

void MessageThreadWidget::setupUI()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Create scroll area for messages
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Messages container
    m_messagesContainer = new QWidget();
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    
    // Modern spacing and margins
    m_messagesLayout->setContentsMargins(20, 20, 20, 20);
    m_messagesLayout->setSpacing(8); // Tighter spacing between messages
    m_messagesLayout->addStretch(); // Push messages to bottom initially

    m_scrollArea->setWidget(m_messagesContainer);
    
    // Apply initial theme-aware styling
    updateChatAreaStyling();
    m_mainLayout->addWidget(m_scrollArea);

    // Offline notice label (hidden by default) - using theme tokens
    m_offlineLabel = new QLabel();
    m_offlineLabel->setWordWrap(true);
    m_offlineLabel->setAlignment(Qt::AlignCenter);
    // Initial style will be set by theme change handler
    m_offlineLabel->hide();
    m_mainLayout->insertWidget(0, m_offlineLabel);

    // Empty state label - using theme tokens
    m_emptyLabel = new QLabel("How can I help you today?");
    m_emptyLabel->setAlignment(Qt::AlignHCenter);
    // Initial style will be set by theme change handler
    m_emptyPanel = createEmptyPanel();

    // Scroll timer for smooth scrolling
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(true);
    m_scrollTimer->setInterval(50);
    connect(m_scrollTimer, &QTimer::timeout, this, &MessageThreadWidget::onScrollToBottom);
}

void MessageThreadWidget::connectSignals()
{
    // Connect streaming timer
    connect(m_streamingTimer, &QTimer::timeout, this, &MessageThreadWidget::onStreamingTimerTick);
    
    // Acquire provider manager from application and connect signals
    auto *app = Application::instance();
    m_providerManager = app->providerManager();
    auto *themeManager = app->themeManager();
    if (m_providerManager) {
        connect(m_providerManager, &ProviderManager::messageChunk,
                this, [this](const QString &convId, const QString &msgId, const QString &chunk) {
                    Q_UNUSED(msgId)
                    if (convId == m_currentConversationId && m_streamingMessageWidget) {
                        QString current = m_streamingMessageWidget->message().text + chunk; // simplistic append
                        m_streamingMessageWidget->updateContent(current);
                    }
                });
    connect(m_providerManager, &ProviderManager::activeProviderChanged, this, [this](const QString &){ updateOfflineNotice(); });
    connect(m_providerManager, &ProviderManager::providerStatusChanged, this, [this](AIProvider::Status, const QString &){ updateOfflineNotice(); });
    connect(m_providerManager, &ProviderManager::messageCompleted,
        this, [this](const QString &convId, const QString &msgId, const Message &message) {
            Q_UNUSED(msgId)
                    if (convId == m_currentConversationId && m_streamingMessageWidget) {
                        m_streamingMessageWidget->updateContent(message.text);
                        m_streamingMessageWidget->setStreaming(false);
                        m_streamingMessageWidget->setGenerating(false);
                        m_streamingMessageWidget = nullptr;
                        m_currentAssistantMessageId.clear();
                        emit conversationUpdated(convId);
                    }
                });
        connect(m_providerManager, &ProviderManager::messageFailed,
                this, [this](const QString &convId, const QString &, const QString &error) {
                    if (convId == m_currentConversationId && m_streamingMessageWidget) {
                        m_streamingMessageWidget->updateContent("Error: " + error);
                        m_streamingMessageWidget->setStreaming(false);
                        m_streamingMessageWidget->setGenerating(false);
                        m_streamingMessageWidget = nullptr;
                    }
                });
    }

    // Re-style on theme change for better ChatGPT-like contrast
    connect(themeManager, &ThemeManager::themeChanged, this, [this]() {
        // Update chat area background colors
        updateChatAreaStyling();
        
        // Update suggestion panel buttons
        if (m_emptyPanel) {
            auto *themeManager = Application::instance()->themeManager();
            const auto &tokens = themeManager->tokens();
            
            // Create consistent button styling using theme tokens
            QString btnStyle = QString(R"(
                QPushButton {
                    text-align: left;
                    background: %1;
                    color: %2;
                    border: 1px solid %3;
                    border-radius: %4px;
                    padding: 16px;
                    font-size: 14px;
                    font-weight: 400;
                    line-height: 1.4;
                }
                QPushButton:hover {
                    background: %5;
                    border-color: %6;
                }
                QPushButton:pressed {
                    background: %7;
                }
            )")
                .arg(tokens.surface.name())
                .arg(tokens.text.name())
                .arg(tokens.border.name())
                .arg(tokens.radiusLarge)
                .arg(tokens.surfaceHover.name())
                .arg(tokens.primary.name())
                .arg(tokens.border.name());
            
            for (auto *btn : m_emptyPanel->findChildren<QPushButton*>()) {
                btn->setStyleSheet(btnStyle);
            }
            
            // Update empty label using theme tokens
            if (m_emptyLabel) {
                m_emptyLabel->setStyleSheet(QString(R"(
                    QLabel { 
                        color: %1; 
                        font-size: 32px; 
                        font-weight: 400; 
                        margin-top: 48px;
                        margin-bottom: 24px;
                    })")
                    .arg(tokens.text.name()));
            }
            
            // Update subtitle in empty panel using theme tokens
            for (auto *label : m_emptyPanel->findChildren<QLabel*>()) {
                label->setStyleSheet(QString(R"(
                    QLabel { 
                        color: %1; 
                        font-size: 16px; 
                        font-weight: 400;
                        margin-bottom: 16px;
                    })")
                    .arg(tokens.textMuted.name()));
            }
            
            // Update offline notice using theme tokens
            if (m_offlineLabel) {
                m_offlineLabel->setStyleSheet(QString(R"(
                    QLabel { 
                        background: %1; 
                        color: %2; 
                        border: 1px solid %3; 
                        border-radius: %4px; 
                        padding: 12px 16px; 
                        margin: 8px 16px; 
                        font-size: 13px;
                        font-weight: 500;
                    })")
                    .arg(tokens.warning.name())
                    .arg(tokens.background.name()) 
                    .arg(tokens.warning.name())
                    .arg(tokens.radiusMedium));
            }
        }
        
        // Update all message widgets with new ChatGPT styling
        for (auto *msgWidget : findChildren<MessageWidget*>()) {
            msgWidget->updateStyling();
        }
    });
    
    // Apply initial theme styles
    if (m_emptyPanel) {
        auto *themeManager = Application::instance()->themeManager();
        const auto &tokens = themeManager->tokens();
        
        // Apply initial button styling
        QString btnStyle = QString(R"(
            QPushButton {
                text-align: left;
                background: %1;
                color: %2;
                border: 1px solid %3;
                border-radius: %4px;
                padding: 16px;
                font-size: 14px;
                font-weight: 400;
                line-height: 1.4;
            }
            QPushButton:hover {
                background: %5;
                border-color: %6;
            }
            QPushButton:pressed {
                background: %7;
            }
        )")
            .arg(tokens.surface.name())
            .arg(tokens.text.name())
            .arg(tokens.border.name())
            .arg(tokens.radiusLarge)
            .arg(tokens.surfaceHover.name())
            .arg(tokens.primary.name())
            .arg(tokens.border.name());
        
        for (auto *btn : m_emptyPanel->findChildren<QPushButton*>()) {
            btn->setStyleSheet(btnStyle);
        }
        
        // Apply initial empty label styling
        if (m_emptyLabel) {
            m_emptyLabel->setStyleSheet(QString(R"(
                QLabel { 
                    color: %1; 
                    font-size: 32px; 
                    font-weight: 400; 
                    margin-top: 48px;
                    margin-bottom: 24px;
                })")
                .arg(tokens.text.name()));
        }
        
        // Apply initial subtitle styling
        for (auto *label : m_emptyPanel->findChildren<QLabel*>()) {
            label->setStyleSheet(QString(R"(
                QLabel { 
                    color: %1; 
                    font-size: 16px; 
                    font-weight: 400;
                    margin-bottom: 16px;
                })")
                .arg(tokens.textMuted.name()));
        }
        
        // Apply initial offline notice styling
        if (m_offlineLabel) {
            m_offlineLabel->setStyleSheet(QString(R"(
                QLabel { 
                    background: %1; 
                    color: %2; 
                    border: 1px solid %3; 
                    border-radius: %4px; 
                    padding: 12px 16px; 
                    margin: 8px 16px; 
                    font-size: 13px;
                    font-weight: 500;
                })")
                .arg(tokens.warning.name())
                .arg(tokens.background.name()) 
                .arg(tokens.warning.name())
                .arg(tokens.radiusMedium));
        }
    }
}

void MessageThreadWidget::updateOfflineNotice()
{
    bool haveProvider = m_providerManager && m_providerManager->activeProvider();
    if (!m_offlineLabel) return;
    if (!haveProvider) {
        m_offlineLabel->setText("Offline echo mode: No model/provider configured. Your messages will just be repeated locally.");
        m_offlineLabel->show();
    } else {
        m_offlineLabel->hide();
    }
}

QWidget* MessageThreadWidget::createEmptyPanel() {
    QWidget *panel = new QWidget();
    panel->setStyleSheet("QWidget { background-color: transparent; }");
    
    // Create a main layout that centers content both horizontally and vertically
    QVBoxLayout *mainLayout = new QVBoxLayout(panel);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // Add stretch at top to center content vertically
    mainLayout->addStretch(1);
    
    // Create centered content container
    QWidget *centerContent = new QWidget();
    centerContent->setMaximumWidth(720); // Responsive max width
    centerContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    QVBoxLayout *contentLayout = new QVBoxLayout(centerContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(32);
    contentLayout->setAlignment(Qt::AlignCenter);
    
    // Subtitle
    QLabel *subtitle = new QLabel("Ask me anything, or try an example");
    subtitle->setAlignment(Qt::AlignCenter);
    
    // Use theme tokens for subtitle styling
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    const auto &tokens = themeManager->tokens();
    
    subtitle->setStyleSheet(QString(R"(
        QLabel { 
            color: %1; 
            font-size: 16px; 
            font-weight: 400;
            margin-bottom: 16px;
        }
    )").arg(tokens.textMuted.name()));
    contentLayout->addWidget(subtitle);
    
    // Grid of suggestion cards - ChatGPT style with responsive layout
    m_suggestionsGrid = new QGridLayout();
    m_suggestionsGrid->setSpacing(16);
    m_suggestionsGrid->setAlignment(Qt::AlignCenter);
    
    QStringList prompts = {
        "💡 Explain quantum computing in simple terms",
        "🎨 Give me creative ideas for a weekend project", 
        "🔧 Help me debug a C++ segmentation fault",
        "📝 Summarize the benefits of unit testing"
    };
    
    int r = 0, c = 0;
    for (const QString &p : prompts) {
        QPushButton *btn = new QPushButton(p);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setMinimumHeight(80);
        btn->setMinimumWidth(300);
        btn->setMaximumWidth(350);
        
        // ChatGPT-style card design using theme tokens
        auto *app = Application::instance();
        auto *themeManager = app->themeManager();
        const auto &tokens = themeManager->tokens();
        
        btn->setStyleSheet(QString(R"(
            QPushButton {
                text-align: left;
                background: %1;
                color: %2;
                border: 1px solid %3;
                border-radius: %4px;
                padding: 16px;
                font-size: 14px;
                font-weight: 400;
                line-height: 1.4;
            }
            QPushButton:hover {
                background: %5;
                border-color: %6;
            }
            QPushButton:pressed {
                background: %7;
            }
        )")
            .arg(tokens.surface.name())
            .arg(tokens.text.name())
            .arg(tokens.border.name())
            .arg(tokens.radiusLarge)
            .arg(tokens.surfaceHover.name())
            .arg(tokens.primary.name())
            .arg(tokens.border.name()));
        
        connect(btn, &QPushButton::clicked, this, [this, p]() { 
            // Remove emoji and extra spacing for the actual message
            QString cleanPrompt = p;
            cleanPrompt.remove(QRegularExpression("^[🎨💡🔧📝]\\s*"));
            addUserMessage(cleanPrompt); 
        });
        
        m_suggestionsGrid->addWidget(btn, r, c);
        if (++c == 2) { 
            c = 0; 
            ++r; 
        }
    }
    
    contentLayout->addLayout(m_suggestionsGrid);
    
    // Add the centered content to main layout
    mainLayout->addWidget(centerContent, 0, Qt::AlignCenter);
    
    // Add stretch at bottom to center content vertically
    mainLayout->addStretch(1);
    
    panel->hide();
    return panel;
}

void MessageThreadWidget::loadConversation(const QString &conversationId)
{
    if (m_currentConversationId == conversationId) {
        return;
    }

    m_currentConversationId = conversationId;
    clearMessages();

    if (conversationId.isEmpty()) {
        showEmptyState();
        return;
    }

    hideEmptyState();

    auto *app = Application::instance();
    auto *store = app->conversationStore();
    
    MessageList messages = store->getMessagesForConversation(conversationId);
    populateMessages(messages);

    qDebug() << "Loaded conversation" << conversationId << "with" << messages.size() << "messages";
}

void MessageThreadWidget::addUserMessage(const QString &text, const AttachmentList &attachments)
{
    Q_UNUSED(attachments)
    if (text.trimmed().isEmpty()) {
        return; // ignore empty
    }

    auto *app = Application::instance();
    auto *store = app->conversationStore();

    // If no conversation yet, create one automatically so first message works
    if (m_currentConversationId.isEmpty()) {
        Conversation conv("New Conversation");
        if (store->createConversation(conv)) {
            m_currentConversationId = conv.id;
            hideEmptyState(); // switch from empty panel to thread view
            emit conversationUpdated(conv.id); // refresh list so it appears
        } else {
            qWarning() << "Failed to auto-create conversation for first message";
            return;
        }
    }

    // Create user message
    Message userMessage(m_currentConversationId, MessageRole::User, text.trimmed());
    
    if (store->createMessage(userMessage)) {
        addMessageWidget(userMessage);
        scrollToBottom();
        // Auto title on first user message
        if (store->getConversationMessageCount(m_currentConversationId) == 1) {
            ensureAutoTitle(text.trimmed());
        }
        
        // Generate AI response
        generateResponse(text.trimmed());
        
        emit messageAdded(userMessage.id);
        emit conversationUpdated(m_currentConversationId);
    }
}

void MessageThreadWidget::clearMessages()
{
    // Remove all message widgets
    while (m_messagesLayout->count() > 1) { // Keep the stretch
        QLayoutItem *item = m_messagesLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    m_streamingMessageWidget = nullptr;
}

void MessageThreadWidget::populateMessages(const MessageList &messages)
{
    for (const Message &message : messages) {
        addMessageWidget(message);
    }
    
    if (!messages.isEmpty()) {
        scrollToBottom();
    }
}

void MessageThreadWidget::addMessageWidget(const Message &message)
{
    auto *messageWidget = new MessageWidget(message, this);
    
    // Insert before the stretch
    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, messageWidget);
    
    // Connect message actions
    connect(messageWidget, &MessageWidget::copyRequested,
            [](const QString &text) {
                QApplication::clipboard()->setText(text);
            });
    
    connect(messageWidget, &MessageWidget::editCompleted,
            this, [this](const QString &messageId, const QString &newText) {
                auto *app = Application::instance();
                auto *store = app->conversationStore();
                Message msg = store->getMessage(messageId);
                if (msg.isValid()) {
                    msg.text = newText;
                    msg.metadata.insert("edited", true);
                    store->updateMessage(msg);
                    
                    // Hide subsequent messages after this edited message
                    hideMessagesAfter(messageId);
                    
                    // Regenerate response with new user message
                    if (msg.role == MessageRole::User) {
                        generateResponse(newText);
                    }
                    
                    emit conversationUpdated(m_currentConversationId);
                }
            });
    
    connect(messageWidget, &MessageWidget::editCancelled,
            this, [this](const QString &messageId) {
                Q_UNUSED(messageId)
                // Nothing to do on cancel
            });
    
    connect(messageWidget, &MessageWidget::regenerateRequested,
            this, [this, messageWidget](const QString &messageId) {
                // Hide this AI message and all subsequent messages
                hideMessagesAfter(messageId);
                
                // Find the user message that triggered this AI response
                auto *app = Application::instance();
                auto *store = app->conversationStore();
                MessageList messages = store->getMessagesForConversation(m_currentConversationId);
                
                QString userMessageText;
                for (int i = messages.size() - 1; i >= 0; --i) {
                    if (messages[i].id == messageId) {
                        // Found the AI message, look for previous user message
                        for (int j = i - 1; j >= 0; --j) {
                            if (messages[j].role == MessageRole::User) {
                                userMessageText = messages[j].text;
                                break;
                            }
                        }
                        break;
                    }
                }
                
                if (!userMessageText.isEmpty()) {
                    generateResponse(userMessageText);
                }
            });
    
    connect(messageWidget, &MessageWidget::stopGenerationRequested,
            this, [this]() {
                // Stop current generation
                if (m_streamingTimer->isActive()) {
                    m_streamingTimer->stop();
                }
                if (m_streamingMessageWidget) {
                    m_streamingMessageWidget->setStreaming(false);
                    m_streamingMessageWidget->setGenerating(false);
                    m_streamingMessageWidget = nullptr;
                }
                if (m_providerManager) {
                    // TODO: Add stop method to provider manager
                }
                m_currentAssistantMessageId.clear();
            });
}

void MessageThreadWidget::hideMessagesAfter(const QString &messageId)
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    MessageList messages = store->getMessagesForConversation(m_currentConversationId);
    
    bool foundTarget = false;
    for (const Message &msg : messages) {
        if (foundTarget) {
            // Hide this message from UI
            for (int i = 0; i < m_messagesLayout->count(); ++i) {
                if (auto *widget = qobject_cast<MessageWidget*>(m_messagesLayout->itemAt(i)->widget())) {
                    if (widget->message().id == msg.id) {
                        widget->hide();
                        break;
                    }
                }
            }
            
            // Mark as hidden in store
            Message hiddenMsg = msg;
            hiddenMsg.metadata.insert("hidden", true);
            store->updateMessage(hiddenMsg);
        }
        
        if (msg.id == messageId) {
            foundTarget = true;
        }
    }
}

void MessageThreadWidget::scrollToBottom()
{
    m_scrollTimer->start();
}

void MessageThreadWidget::onScrollToBottom()
{
    QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MessageThreadWidget::showEmptyState()
{
    m_scrollArea->hide();
    
    // Create a centered empty state container if not exists
    if (!m_emptyStateContainer) {
        m_emptyStateContainer = new QWidget();
        QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyStateContainer);
        emptyLayout->setContentsMargins(0, 0, 0, 0);
        emptyLayout->setSpacing(0);
        emptyLayout->setAlignment(Qt::AlignCenter);
        
        // Add vertical stretch at top
        emptyLayout->addStretch(1);
        
        // Add the empty label (title)
        emptyLayout->addWidget(m_emptyLabel, 0, Qt::AlignCenter);
        
        // Add the suggestion panel
        emptyLayout->addWidget(m_emptyPanel, 0, Qt::AlignCenter);
        
        // Add vertical stretch at bottom
        emptyLayout->addStretch(1);
        
        // Add to main layout
        m_mainLayout->addWidget(m_emptyStateContainer);
    }
    
    m_emptyStateContainer->show();
    m_emptyLabel->show();
    m_emptyPanel->show();
}

void MessageThreadWidget::hideEmptyState()
{
    if (m_emptyStateContainer) {
        m_emptyStateContainer->hide();
    }
    m_emptyLabel->hide();
    if (m_emptyPanel) m_emptyPanel->hide();
    m_scrollArea->show();
}

void MessageThreadWidget::generateResponse(const QString &userMessage)
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    
    // Show loading dots immediately
    m_loadingDotsWidget = new LoadingDotsWidget(this);
    int insertIndex = m_messagesLayout->count() - 1;
    m_messagesLayout->insertWidget(insertIndex, m_loadingDotsWidget);
    m_loadingDotsWidget->startAnimation();
    scrollToBottom();
    
    // Simulate response generation time
    QTimer::singleShot(2000, this, [this, userMessage, store]() {
        // Remove loading dots
        if (m_loadingDotsWidget) {
            m_messagesLayout->removeWidget(m_loadingDotsWidget);
            m_loadingDotsWidget->deleteLater();
            m_loadingDotsWidget = nullptr;
        }
        
        // Create and display complete assistant response
        Message assistantMessage(m_currentConversationId, MessageRole::Assistant, userMessage);
        m_currentAssistantMessageId = assistantMessage.id;
        
        if (store->createMessage(assistantMessage)) {
            addMessageWidget(assistantMessage);
            scrollToBottom();
            emit conversationUpdated(m_currentConversationId);
        }
        
        m_currentAssistantMessageId.clear();
    });
}

void MessageThreadWidget::onProviderResponse(const QString &, bool) {}
void MessageThreadWidget::onProviderError(const QString &) {}

void MessageThreadWidget::ensureAutoTitle(const QString &firstUserText)
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    if (m_currentConversationId.isEmpty()) return;
    
    Conversation conv = store->getConversation(m_currentConversationId);
    if (conv.title == "New Conversation" || conv.title.trimmed().isEmpty()) {
        // Generate ChatGPT-like title from first user message
        QString title = generateConversationTitle(firstUserText);
        conv.title = title;
        conv.updateTimestamp();
        store->updateConversation(conv);
        
        // Notify conversation list to refresh
        emit conversationUpdated(m_currentConversationId);
    }
}

QString MessageThreadWidget::generateConversationTitle(const QString &userText)
{
    // Clean the text
    QString cleaned = userText.trimmed();
    
    // Remove common prefixes and make more concise
    QStringList prefixesToRemove = {
        "can you", "could you", "please", "i want to", "i need to", "help me",
        "explain", "tell me", "show me", "give me", "what is", "how to", "how do"
    };
    
    QString lowerText = cleaned.toLower();
    for (const QString &prefix : prefixesToRemove) {
        if (lowerText.startsWith(prefix)) {
            cleaned = cleaned.mid(prefix.length()).trimmed();
            break;
        }
    }
    
    // Split into words and take first 4-6 words
    QStringList words = cleaned.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    if (words.isEmpty()) {
        return "New Conversation";
    }
    
    // Capitalize first word
    if (!words.isEmpty()) {
        words[0] = words[0][0].toUpper() + words[0].mid(1).toLower();
    }
    
    // Take first 4-6 words depending on length
    int wordCount = qMin(words.size(), words.size() > 3 ? 4 : words.size());
    QString title = words.mid(0, wordCount).join(" ");
    
    // Add ellipsis if there are more words
    if (words.size() > wordCount) {
        title += "…";
    }
    
    // Ensure title is not too long
    if (title.length() > 50) {
        title = title.left(47) + "…";
    }
    
    return title;
}

void MessageThreadWidget::refineAutoTitle()
{
    // Placeholder: future implementation could send a summarization request
    // For now, do nothing; hook after assistant first full reply
}

void MessageThreadWidget::startStreamingAnimation(const QString &fullText)
{
    m_fullResponseText = fullText;
    m_streamingPosition = 0;
    
    // Configure timer for more natural word-by-word typing effect
    m_streamingTimer->setInterval(80); // 80ms between chunks for realistic typing speed
    m_streamingTimer->start();
}

void MessageThreadWidget::onStreamingTimerTick()
{
    if (!m_streamingMessageWidget || m_streamingPosition >= m_fullResponseText.length()) {
        // Animation complete
        if (m_streamingMessageWidget) {
            m_streamingMessageWidget->setStreaming(false);
            m_streamingMessageWidget->setGenerating(false);
            
            // Update the message in the store with final content
            auto *app = Application::instance();
            auto *store = app->conversationStore();
            if (!m_currentAssistantMessageId.isEmpty()) {
                Message msg = store->getMessage(m_currentAssistantMessageId);
                msg.text = m_fullResponseText;
                msg.isStreaming = false;
                store->updateMessage(msg);
                emit conversationUpdated(m_currentConversationId);
            }
            
            m_streamingMessageWidget = nullptr;
            m_currentAssistantMessageId.clear();
        }
        m_streamingTimer->stop();
        return;
    }
    
    // Find next good stopping point (word boundary or punctuation)
    int nextStop = m_streamingPosition;
    int textLength = m_fullResponseText.length();
    
    // Advance by 1-3 characters for more natural flow
    int chunkSize = QRandomGenerator::global()->bounded(1, 4); // Random 1-3 characters
    nextStop = qMin(m_streamingPosition + chunkSize, textLength);
    
    // If we hit a space, advance to include the next word for smoother flow
    if (nextStop < textLength && m_fullResponseText[nextStop] == ' ') {
        while (nextStop < textLength && m_fullResponseText[nextStop] == ' ') {
            nextStop++;
        }
        // Include next word
        while (nextStop < textLength && m_fullResponseText[nextStop] != ' ' && 
               m_fullResponseText[nextStop] != '.' && m_fullResponseText[nextStop] != ',' &&
               m_fullResponseText[nextStop] != '!' && m_fullResponseText[nextStop] != '?') {
            nextStop++;
        }
    }
    
    QString partialText = m_fullResponseText.left(nextStop);
    
    // Update the streaming message widget
    m_streamingMessageWidget->updateContent(partialText);
    
    // Advance position
    m_streamingPosition = nextStop;
    
    // Auto-scroll to bottom as text appears
    scrollToBottom();
}

void MessageThreadWidget::updateChatAreaStyling()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    const auto &tokens = themeManager->tokens();
    
    // Get background colors based on theme
    QString chatBg = tokens["background-color"].toString();
    QString scrollHandleColor = tokens["surface-color"].toString();
    QString scrollHandleHoverColor = tokens["surface-hover-color"].toString();
    
    // Apply theme-aware styling to scroll area
    QString scrollAreaStyle = QString(R"(
        QScrollArea {
            background-color: %1;
            border: none;
        }
        QScrollBar:vertical {
            background-color: transparent;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background-color: %2;
            border-radius: 3px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %3;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }
    )").arg(chatBg, scrollHandleColor, scrollHandleHoverColor);
    
    m_scrollArea->setStyleSheet(scrollAreaStyle);
    
    // Apply background to messages container
    QString containerStyle = QString("QWidget { background-color: %1; }").arg(chatBg);
    m_messagesContainer->setStyleSheet(containerStyle);
}

// MessageWidget class implementation
MessageWidget::MessageWidget(const Message &message, QWidget *parent)
    : QFrame(parent)
    , m_message(message)
    , m_mainLayout(nullptr)
    , m_bubbleContainer(nullptr)
    , m_bubbleLayout(nullptr)
    , m_contentEdit(nullptr)
    , m_editLineEdit(nullptr)
    , m_editControls(nullptr)
    , m_actionsWidget(nullptr)
    , m_copyButton(nullptr)
    , m_editButton(nullptr)
    , m_regenerateButton(nullptr)
    , m_saveEditButton(nullptr)
    , m_cancelEditButton(nullptr)
    , m_stopButton(nullptr)
    , m_isStreaming(false)
    , m_isGenerating(false)
    , m_inEditMode(false)
    , m_actionsOpacity(nullptr)
    , m_actionsEffect(nullptr)
{
    setupUI();
    updateStyling();
}

void MessageWidget::setupUI()
{
    setFrameStyle(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    
    // Make the widget background transparent so bubbles show
    setStyleSheet("MessageWidget { background: transparent; }");
    
    // Main horizontal layout for alignment with tighter spacing
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 4, 8, 4);
    m_mainLayout->setSpacing(8);
    
    setupBubbleLayout();
    setupActions();
    updateBubbleAlignment();
}

void MessageWidget::setupBubbleLayout()
{
    // Create bubble container with explicit name
    m_bubbleContainer = new QWidget();
    m_bubbleContainer->setObjectName("bubbleContainer");
    m_bubbleContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_bubbleContainer->setMinimumWidth(200);  // Ensure minimum readable width
    m_bubbleContainer->setMaximumWidth(600);  // Reasonable maximum width
    
    // Bubble layout
    m_bubbleLayout = new QVBoxLayout(m_bubbleContainer);
    m_bubbleLayout->setContentsMargins(0, 0, 0, 0);  // Remove margins since padding is in stylesheet
    m_bubbleLayout->setSpacing(4);
    
    // Content area
    m_contentEdit = new QTextEdit();
    m_contentEdit->setObjectName("messageContent");
    m_contentEdit->setReadOnly(true);
    m_contentEdit->setFrameStyle(QFrame::NoFrame);
    m_contentEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_contentEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_contentEdit->setStyleSheet("QTextEdit { background: transparent; border: none; }");
    m_contentEdit->document()->setDocumentMargin(0);
    
    // Auto-resize content
    connect(m_contentEdit->document(), &QTextDocument::contentsChanged, [this]() {
        QSize size = m_contentEdit->document()->size().toSize();
        m_contentEdit->setFixedHeight(qMax(30, size.height() + 10)); // Increased minimum height
        
        // Update bubble container size
        m_bubbleContainer->adjustSize();
    });
    
    // Edit line input (hidden by default)
    m_editLineEdit = new QLineEdit();
    m_editLineEdit->hide();
    m_editLineEdit->setStyleSheet("QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 8px; }");
    
    // Edit controls (hidden by default)
    m_editControls = new QWidget();
    m_editControls->hide();
    QHBoxLayout *editLayout = new QHBoxLayout(m_editControls);
    editLayout->setContentsMargins(0, 4, 0, 0);
    editLayout->setSpacing(8);
    
    m_cancelEditButton = new QPushButton("Cancel");
    m_cancelEditButton->setStyleSheet(R"(
        QPushButton {
            background: #F3F4F6;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 12px;
        }
        QPushButton:hover { background: #E5E7EB; }
    )");
    
    m_saveEditButton = new QPushButton("Save");
    m_saveEditButton->setStyleSheet(R"(
        QPushButton {
            background: #10B981;
            border: 1px solid #10B981;
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 12px;
            color: white;
        }
        QPushButton:hover { background: #059669; }
    )");
    
    connect(m_cancelEditButton, &QPushButton::clicked, this, &MessageWidget::onEditCancel);
    connect(m_saveEditButton, &QPushButton::clicked, this, &MessageWidget::onEditSave);
    
    editLayout->addWidget(m_cancelEditButton);
    editLayout->addWidget(m_saveEditButton);
    editLayout->addStretch();
    
    // Add to bubble
    m_bubbleLayout->addWidget(m_contentEdit);
    m_bubbleLayout->addWidget(m_editLineEdit);
    m_bubbleLayout->addWidget(m_editControls);
    
    // Set initial content
    m_contentEdit->setPlainText(m_message.text);
    
    // Add bubble to main layout
    m_mainLayout->addWidget(m_bubbleContainer);
}

void MessageWidget::setupActions()
{
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();
    
    // Actions container
    m_actionsWidget = new QWidget();
    m_actionsWidget->hide();
    
    QHBoxLayout *actionsLayout = new QHBoxLayout(m_actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(4);
    
    // Copy button (always visible)
    m_copyButton = new QPushButton();
    m_copyButton->setIcon(iconRegistry->icon("copy"));
    m_copyButton->setToolTip("Copy");
    m_copyButton->setFixedSize(32, 32);
    m_copyButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            background: white;
            padding: 6px;
        }
        QPushButton:hover {
            background: #F9FAFB;
            border-color: #9CA3AF;
        }
    )");
    connect(m_copyButton, &QPushButton::clicked, this, &MessageWidget::onCopyClicked);
    actionsLayout->addWidget(m_copyButton);
    
    if (m_message.role == MessageRole::User) {
        // Edit button for user messages
        m_editButton = new QPushButton();
        m_editButton->setIcon(iconRegistry->icon("edit"));
        m_editButton->setToolTip("Edit");
        m_editButton->setFixedSize(32, 32);
        m_editButton->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #D1D5DB;
                border-radius: 6px;
                background: white;
                padding: 6px;
            }
            QPushButton:hover {
                background: #F9FAFB;
                border-color: #9CA3AF;
            }
        )");
        connect(m_editButton, &QPushButton::clicked, this, &MessageWidget::onEditClicked);
        actionsLayout->addWidget(m_editButton);
    } else if (m_message.role == MessageRole::Assistant) {
        // Regenerate/Try again button for AI messages
        m_regenerateButton = new QPushButton();
        m_regenerateButton->setIcon(iconRegistry->icon("send"));
        m_regenerateButton->setToolTip("Try again");
        m_regenerateButton->setFixedSize(32, 32);
        m_regenerateButton->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #D1D5DB;
                border-radius: 6px;
                background: white;
                padding: 6px;
            }
            QPushButton:hover {
                background: #F9FAFB;
                border-color: #9CA3AF;
            }
        )");
        connect(m_regenerateButton, &QPushButton::clicked, this, &MessageWidget::onRegenerateClicked);
        actionsLayout->addWidget(m_regenerateButton);
        
        // Stop button (shown when generating)
        m_stopButton = new QPushButton();
        m_stopButton->setIcon(iconRegistry->icon("stop"));
        m_stopButton->setToolTip("Stop generation");
        m_stopButton->setFixedSize(32, 32);
        m_stopButton->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #EF4444;
                border-radius: 6px;
                background: white;
                padding: 6px;
                color: #EF4444;
            }
            QPushButton:hover {
                background: #FEF2F2;
                border-color: #DC2626;
            }
        )");
        m_stopButton->hide();
        connect(m_stopButton, &QPushButton::clicked, this, &MessageWidget::onStopGeneration);
        actionsLayout->addWidget(m_stopButton);
    }
    
    // Add actions to main layout
    m_mainLayout->addWidget(m_actionsWidget);
    
    // Setup hover animations
    m_actionsEffect = new QGraphicsOpacityEffect(m_actionsWidget);
    m_actionsWidget->setGraphicsEffect(m_actionsEffect);
    m_actionsEffect->setOpacity(0.0);
    
    m_actionsOpacity = new QPropertyAnimation(m_actionsEffect, "opacity");
    m_actionsOpacity->setDuration(150);
}

void MessageWidget::updateBubbleAlignment()
{
    // Clear main layout
    m_mainLayout->removeWidget(m_bubbleContainer);
    m_mainLayout->removeWidget(m_actionsWidget);
    
    if (m_message.role == MessageRole::User) {
        // User messages: align right
        m_mainLayout->addStretch();
        m_mainLayout->addWidget(m_actionsWidget);
        m_mainLayout->addWidget(m_bubbleContainer);
    } else {
        // AI messages: align left  
        m_mainLayout->addWidget(m_bubbleContainer);
        m_mainLayout->addWidget(m_actionsWidget);
        m_mainLayout->addStretch();
    }
}

void MessageWidget::updateContent(const QString &content)
{
    m_contentEdit->setPlainText(content);
    m_message.text = content;
}

void MessageWidget::setStreaming(bool streaming)
{
    m_isStreaming = streaming;
    // Could add typing cursor animation here
}

void MessageWidget::setGenerating(bool generating)
{
    m_isGenerating = generating;
    
    if (m_regenerateButton && m_stopButton) {
        if (generating) {
            m_regenerateButton->hide();
            m_stopButton->show();
        } else {
            m_stopButton->hide();
            m_regenerateButton->show();
        }
    }
}

void MessageWidget::updateStyling()
{
    // Modern chat bubble styling inspired by popular chat apps
    QString bubbleStyle;
    QString textStyle;
    
    if (m_message.role == MessageRole::User) {
        // User messages: clean blue bubble, right-aligned
        bubbleStyle = R"(
            QWidget#bubbleContainer {
                background-color: #0084FF;
                border: none;
                border-radius: 20px;
                padding: 12px 16px;
                margin: 2px;
                min-width: 120px;
                max-width: 480px;
            }
        )";
        
        // White text for user messages
        textStyle = R"(
            QTextEdit#messageContent {
                background-color: transparent;
                color: white;
                font-size: 15px;
                font-weight: 400;
                border: none;
                padding: 0px;
                margin: 0px;
                line-height: 1.4;
            }
        )";
    } else {
        // AI messages: light gray bubble, left-aligned  
        bubbleStyle = R"(
            QWidget#bubbleContainer {
                background-color: #F0F0F0;
                border: none;
                border-radius: 20px;
                padding: 12px 16px;
                margin: 2px;
                min-width: 120px;
                max-width: 480px;
            }
        )";
           
        // Dark text for AI messages
        textStyle = R"(
            QTextEdit#messageContent {
                background-color: transparent;
                color: #000000;
                font-size: 15px;
                font-weight: 400;
                border: none;
                padding: 0px;
                margin: 0px;
                line-height: 1.4;
            }
        )";
    }
    
    m_bubbleContainer->setStyleSheet(bubbleStyle);
    m_contentEdit->setStyleSheet(textStyle);
    
    // Force layout update
    m_bubbleContainer->update();
    m_contentEdit->update();
    this->update();
}

void MessageWidget::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    showHoverActions();
}

void MessageWidget::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    if (!m_inEditMode) {
        hideHoverActions();
    }
}

void MessageWidget::showHoverActions()
{
    if (m_actionsOpacity) {
        m_actionsOpacity->stop();
        m_actionsOpacity->setStartValue(m_actionsEffect->opacity());
        m_actionsOpacity->setEndValue(1.0);
        m_actionsOpacity->start();
    }
    m_actionsWidget->show();
}

void MessageWidget::hideHoverActions()
{
    if (m_actionsOpacity) {
        m_actionsOpacity->stop();
        m_actionsOpacity->setStartValue(m_actionsEffect->opacity());
        m_actionsOpacity->setEndValue(0.0);
        m_actionsOpacity->start();
        
        // Hide widget after animation
        QTimer::singleShot(150, this, [this]() {
            if (m_actionsEffect->opacity() < 0.1) {
                m_actionsWidget->hide();
            }
        });
    }
}

void MessageWidget::enterEditMode()
{
    if (m_inEditMode || m_message.role != MessageRole::User) return;
    
    m_inEditMode = true;
    m_originalContent = m_message.text;
    
    // Hide content, show edit input
    m_contentEdit->hide();
    m_editLineEdit->setText(m_message.text);
    m_editLineEdit->show();
    m_editControls->show();
    m_editLineEdit->setFocus();
    m_editLineEdit->selectAll();
    
    // Keep actions visible during editing
    showHoverActions();
}

void MessageWidget::exitEditMode(bool save)
{
    if (!m_inEditMode) return;
    
    m_inEditMode = false;
    
    if (save) {
        QString newText = m_editLineEdit->text().trimmed();
        if (!newText.isEmpty() && newText != m_originalContent) {
            m_message.text = newText;
            m_contentEdit->setPlainText(newText);
            emit editCompleted(m_message.id, newText);
        }
    } else {
        // Restore original content
        m_contentEdit->setPlainText(m_originalContent);
        emit editCancelled(m_message.id);
    }
    
    // Show content, hide edit controls
    m_editLineEdit->hide();
    m_editControls->hide();
    m_contentEdit->show();
    
    hideHoverActions();
}

void MessageWidget::onCopyClicked()
{
    emit copyRequested(m_message.text);
}

void MessageWidget::onEditClicked()
{
    enterEditMode();
}

void MessageWidget::onRegenerateClicked()
{
    emit regenerateRequested(m_message.id);
}

void MessageWidget::onEditSave()
{
    exitEditMode(true);
}

void MessageWidget::onEditCancel()
{
    exitEditMode(false);
}

void MessageWidget::onStopGeneration()
{
    emit stopGenerationRequested();
}

void MessageThreadWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Adjust suggestions layout when window is resized
    adjustSuggestionsLayout();
}

void MessageThreadWidget::adjustSuggestionsLayout()
{
    if (!m_suggestionsGrid) return;
    
    // Get available width for suggestions
    int availableWidth = width() - 80; // Account for margins
    if (availableWidth <= 0) return;
    
    // Calculate optimal number of columns based on width
    int optimalColumns;
    if (availableWidth < 700) {
        optimalColumns = 1; // Single column for narrow screens
    } else if (availableWidth < 1000) {
        optimalColumns = 2; // Two columns for medium screens
    } else {
        optimalColumns = 2; // Keep max 2 columns even for wide screens
    }
    
    // Reorganize grid layout
    QList<QPushButton*> buttons;
    
    // Collect all buttons from the grid
    for (int i = 0; i < m_suggestionsGrid->count(); ++i) {
        QLayoutItem *item = m_suggestionsGrid->itemAt(i);
        if (item && item->widget()) {
            if (QPushButton *btn = qobject_cast<QPushButton*>(item->widget())) {
                buttons.append(btn);
            }
        }
    }
    
    // Clear the grid
    while (m_suggestionsGrid->count() > 0) {
        m_suggestionsGrid->takeAt(0);
        // Don't delete the item, just remove from layout
    }
    
    // Re-add buttons with new column count
    int r = 0, c = 0;
    for (QPushButton *btn : buttons) {
        m_suggestionsGrid->addWidget(btn, r, c);
        if (++c >= optimalColumns) {
            c = 0;
            ++r;
        }
    }
}

} // namespace DesktopApp
