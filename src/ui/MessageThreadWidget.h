#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QEnterEvent>
#include <QResizeEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include "data/Models.h"
#include "providers/ProviderManager.h"

namespace DesktopApp {

class MessageWidget;
class ProviderManager;

/**
 * @brief Animated loading dots widget showing (...) -> (..) -> (.) cycle
 */
class LoadingDotsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingDotsWidget(QWidget *parent = nullptr);
    void startAnimation();
    void stopAnimation();

private slots:
    void updateDots();

private:
    QLabel *m_dotsLabel;
    QTimer *m_animationTimer;
    int m_currentState; // 0=(...), 1=(..), 2=(.)
};

/**
 * @brief Widget displaying the conversation messages thread
 */
class MessageThreadWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessageThreadWidget(QWidget *parent = nullptr);

    /**
     * @brief Load a conversation by ID
     */
    void loadConversation(const QString &conversationId);

    /**
     * @brief Add a user message to the current conversation
     */
    void addUserMessage(const QString &text, const AttachmentList &attachments = AttachmentList());

    /**
     * @brief Clear all messages
     */
    void clearMessages();

    /**
     * @brief Get current conversation ID
     */
    QString currentConversationId() const { return m_currentConversationId; }

signals:
    void conversationUpdated(const QString &conversationId);
    void messageAdded(const QString &messageId);
    // Message action signals
    void messageEditRequested(const QString &messageId, const QString &currentText);
    void messageDeleteRequested(const QString &messageId);
    void messageRetryRequested(const QString &messageId);
    void messageQuoteRequested(const QString &quotedText);

private slots:
    void onProviderResponse(const QString &response, bool isComplete); // legacy no-op
    void onProviderError(const QString &error); // legacy no-op
    void onScrollToBottom();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void connectSignals();
    void populateMessages(const MessageList &messages);
    void addMessageWidget(const Message &message);
    void scrollToBottom();
    void showEmptyState();
    void hideEmptyState();
    void generateResponse(const QString &userMessage);
    void updateFadeOverlays();
    void ensureAutoTitle(const QString &firstUserText);
    QString generateConversationTitle(const QString &userText);
    void refineAutoTitle();
    void editMessage(const QString &messageId, const QString &newText);
    void deleteMessage(const QString &messageId);
    void updateOfflineNotice();
    void adjustSuggestionsLayout(); // Adjust suggestion cards layout based on window size
    void startStreamingAnimation(const QString &fullText);
    void onStreamingTimerTick();
    void hideMessagesAfter(const QString &messageId); // Hide messages after the given message ID
    void updateChatAreaStyling(); // Update chat area colors based on theme

    // UI Components
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_messagesContainer;
    QVBoxLayout *m_messagesLayout;
    QLabel *m_emptyLabel;
    QWidget *m_emptyPanel; // ChatGPT-like suggestion panel
    QWidget *m_emptyStateContainer {nullptr}; // Container for centered empty state
    QGridLayout *m_suggestionsGrid {nullptr}; // Grid layout for suggestion cards
    QWidget *createEmptyPanel();
    QWidget *m_topFade;
    QWidget *m_bottomFade;
    QLabel *m_offlineLabel {nullptr};
    
    // State
    QString m_currentConversationId;
    QString m_currentAssistantMessageId;
    MessageWidget *m_streamingMessageWidget;
    LoadingDotsWidget *m_loadingDotsWidget;
    
    // Streaming animation state
    QTimer *m_streamingTimer;
    QString m_fullResponseText;
    int m_streamingPosition;
    
    // Services
    ProviderManager *m_providerManager;
    
    // Animation
    QTimer *m_scrollTimer;
};

/**
 * @brief Individual message widget within the thread - ChatGPT bubble style
 */
class MessageWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MessageWidget(const Message &message, QWidget *parent = nullptr);

    /**
     * @brief Update message content (for streaming)
     */
    void updateContent(const QString &content);

    /**
     * @brief Set streaming state
     */
    void setStreaming(bool streaming);
    
    /**
     * @brief Set generation state (for send/stop UI)
     */
    void setGenerating(bool generating);
    
    void updateStyling(); // made public for theme refresh

    /**
     * @brief Get the message
     */
    const Message& message() const { return m_message; }

signals:
    void copyRequested(const QString &text);
    void editRequested(const QString &messageId, const QString &newText);
    void editCompleted(const QString &messageId, const QString &newText);
    void editCancelled(const QString &messageId);
    void regenerateRequested(const QString &messageId);
    void deleteRequested(const QString &messageId);
    void stopGenerationRequested();

private slots:
    void onCopyClicked();
    void onEditClicked();
    void onRegenerateClicked();
    void onEditSave();
    void onEditCancel();
    void onStopGeneration();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUI();
    void setupBubbleLayout();
    void setupActions();
    void enterEditMode();
    void exitEditMode(bool save);
    void updateBubbleAlignment();
    void showHoverActions();
    void hideHoverActions();

    Message m_message;
    
    // UI Components - Bubble style
    QHBoxLayout *m_mainLayout;
    QWidget *m_bubbleContainer;
    QVBoxLayout *m_bubbleLayout;
    QTextEdit *m_contentEdit;
    QLineEdit *m_editLineEdit; // For inline editing
    QWidget *m_editControls;   // Cancel/Save buttons for editing
    
    // Actions
    QWidget *m_actionsWidget;
    QPushButton *m_copyButton;
    QPushButton *m_editButton;
    QPushButton *m_regenerateButton;
    QPushButton *m_saveEditButton;
    QPushButton *m_cancelEditButton;
    QPushButton *m_stopButton;
    
    // State
    bool m_isStreaming;
    bool m_isGenerating;
    bool m_inEditMode;
    QString m_originalContent;
    
    // Animation
    QPropertyAnimation *m_actionsOpacity;
    QGraphicsOpacityEffect *m_actionsEffect;
};

} // namespace DesktopApp
