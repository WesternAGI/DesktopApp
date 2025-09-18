#include "MessageComposer.h"
#include "core/Application.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"
#include "services/FileVault.h"
#include "services/AudioRecorder.h"
#include "providers/ProviderManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QContextMenuEvent>
#include <QPainter>
#include <QDebug>

namespace DesktopApp {

MessageComposer::MessageComposer(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_attachmentsWidget(nullptr)
    , m_attachmentsLayout(nullptr)
    , m_inputLayout(nullptr)
    , m_textEdit(nullptr)
    , m_attachButton(nullptr)
    , m_voiceButton(nullptr)
    , m_sendButton(nullptr)
    , m_statusLayout(nullptr)
    , m_wordCountLabel(nullptr)
    , m_promptCombo(nullptr)
    , m_progressBar(nullptr)
    , m_attachmentMenu(nullptr)
    , m_isRecording(false)
{
    setupUI();
    connectSignals();
    setAcceptDrops(true);
}

void MessageComposer::setupUI()
{
    setMinimumHeight(80);
    setMaximumHeight(300);
    
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 8, 16, 8);
    m_mainLayout->setSpacing(8);

    // Top bar with model selection and settings
    QWidget *topBarWidget = new QWidget();
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(8);
    
    // Model selection dropdown
    QLabel *modelLabel = new QLabel("Model:");
    modelLabel->setObjectName("modelLabel");
    
    QComboBox *modelCombo = new QComboBox();
    modelCombo->addItem("🤖 EchoProvider", "echo");
    modelCombo->addItem("💬 ChatGPT (coming soon)", "chatgpt");
    modelCombo->addItem("🧠 Claude (coming soon)", "claude");
    modelCombo->setMinimumWidth(180);
    modelCombo->setObjectName("modelSelector");
    
    topBarLayout->addWidget(modelLabel);
    topBarLayout->addWidget(modelCombo);
    topBarLayout->addStretch();
    
    m_mainLayout->addWidget(topBarWidget);

    // Attachments area (initially hidden)
    m_attachmentsWidget = new QWidget();
    m_attachmentsLayout = new QHBoxLayout(m_attachmentsWidget);
    m_attachmentsLayout->setContentsMargins(0, 0, 0, 0);
    m_attachmentsLayout->setSpacing(8);
    m_attachmentsLayout->addStretch();
    m_attachmentsWidget->hide();
    m_mainLayout->addWidget(m_attachmentsWidget);

    // Input area with enhanced styling
    QWidget *inputWidget = new QWidget();
    inputWidget->setObjectName("inputContainer");
    m_inputLayout = new QHBoxLayout(inputWidget);
    m_inputLayout->setContentsMargins(12, 8, 12, 8);
    m_inputLayout->setSpacing(8);

    // Enhanced text input with auto-resize
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("Type your message... (Shift+Enter for new line, Enter to send)");
    m_textEdit->setMinimumHeight(40);
    m_textEdit->setMaximumHeight(200);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setAcceptRichText(false);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_textEdit->setObjectName("messageInput");
    
    // Auto-resize text edit
    connect(m_textEdit, &QTextEdit::textChanged, [this]() {
        // Auto-resize the text edit based on content
        QTextDocument *doc = m_textEdit->document();
        doc->setTextWidth(m_textEdit->width());
        int height = doc->size().height() + 16; // Add padding
        height = qBound(40, height, 200); // Clamp between min and max
        m_textEdit->setFixedHeight(height);
        
        // Update parent height
        adjustSize();
    });
    
    m_inputLayout->addWidget(m_textEdit, 1);

    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();

    // Attach button with better styling
    m_attachButton = new QToolButton();
    m_attachButton->setIcon(iconRegistry->icon("attach"));
    m_attachButton->setToolTip("Attach files (Ctrl+Shift+A)");
    m_attachButton->setFixedSize(40, 40);
    m_attachButton->setObjectName("composerAction");
    m_inputLayout->addWidget(m_attachButton);

    // Voice button
    m_voiceButton = new QToolButton();
    m_voiceButton->setIcon(iconRegistry->icon("microphone"));
    m_voiceButton->setToolTip("Record voice message (Ctrl+Shift+V)");
    m_voiceButton->setFixedSize(40, 40);
    m_voiceButton->setCheckable(true);
    m_voiceButton->setObjectName("composerAction");
    m_inputLayout->addWidget(m_voiceButton);

    // Send button with enhanced styling
    m_sendButton = new QPushButton();
    m_sendButton->setIcon(iconRegistry->icon("send"));
    m_sendButton->setToolTip("Send message (Enter)");
    m_sendButton->setFixedSize(40, 40);
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setEnabled(false);
    m_inputLayout->addWidget(m_sendButton);

    m_mainLayout->addWidget(inputWidget);

    // Status area with enhanced information
    QWidget *statusWidget = new QWidget();
    m_statusLayout = new QHBoxLayout(statusWidget);
    m_statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLayout->setSpacing(16);

    // Character/word count with better formatting
    m_wordCountLabel = new QLabel("0 characters");
    m_wordCountLabel->setObjectName("characterCount");
    m_statusLayout->addWidget(m_wordCountLabel);

    // Shortcut hints
    QLabel *shortcutHint = new QLabel("💡 Use / for commands, @ for models");
    shortcutHint->setObjectName("shortcutHint");
    m_statusLayout->addWidget(shortcutHint);

    m_statusLayout->addStretch();

    // Provider selector 
    m_providerCombo = new QComboBox();
    m_providerCombo->setToolTip("Select AI provider");
    m_providerCombo->setObjectName("providerSelector");
    m_providerCombo->addItem("Echo Provider", "echo");
    m_providerCombo->addItem("Backend AI", "backend_ai");
    m_statusLayout->addWidget(m_providerCombo);

    // Progress bar for uploads/processing
    m_progressBar = new QProgressBar();
    m_progressBar->setMaximumHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setObjectName("progressBar");
    m_progressBar->hide();
    m_statusLayout->addWidget(m_progressBar);

    m_mainLayout->addWidget(statusWidget);

    // Setup attachment context menu (same as before)
    m_attachmentMenu = new QMenu(this);
    m_removeAttachmentAction = m_attachmentMenu->addAction(iconRegistry->icon("delete"), "Remove");
    m_retryAttachmentAction = m_attachmentMenu->addAction(iconRegistry->icon("send"), "Retry");
    
    // Apply initial styling
    updateStyling();
}

void MessageComposer::connectSignals()
{
    // Text input with enhanced handling
    connect(m_textEdit, &QTextEdit::textChanged, this, &MessageComposer::onTextChanged);
    // Install event filter for enhanced keyboard handling
    m_textEdit->installEventFilter(this);

    // Buttons
    connect(m_sendButton, &QPushButton::clicked, this, &MessageComposer::onSendClicked);
    connect(m_attachButton, &QToolButton::clicked, this, &MessageComposer::onAttachClicked);
    connect(m_voiceButton, &QToolButton::toggled, this, &MessageComposer::onVoiceClicked);

    // Context menu actions
    connect(m_removeAttachmentAction, &QAction::triggered, this, &MessageComposer::onRemoveAttachment);
    connect(m_retryAttachmentAction, &QAction::triggered, this, &MessageComposer::onRetryAttachment);

    // Provider selection
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        QString providerId = m_providerCombo->itemData(index).toString();
        emit providerChanged(providerId);
    });

    // Provider status changes
    auto *app = Application::instance();
    auto *providerManager = app->providerManager();
    if (providerManager) {
        connect(providerManager, &ProviderManager::activeProviderChanged, 
                this, [this](const QString &) { updateSendButton(); });
        connect(providerManager, &ProviderManager::providerStatusChanged, 
                this, [this](AIProvider::Status, const QString &) { updateSendButton(); });
    }
    
    // Theme changes
    auto *themeManager = app->themeManager();
    connect(themeManager, &ThemeManager::themeChanged, this, &MessageComposer::updateStyling);
}

void MessageComposer::updateStyling()
{
    auto *themeManager = Application::instance()->themeManager();
    const auto &tokens = themeManager->tokens();
    
    // Input container styling
    QString inputContainerStyle = QString(R"(
        QWidget#inputContainer {
            background: %1;
            border: 2px solid %2;
            border-radius: %3px;
        }
        QWidget#inputContainer:focus-within {
            border-color: %4;
        }
    )").arg(tokens.surface.name())
       .arg(tokens.border.name())
       .arg(tokens.radiusLarge)
       .arg(tokens.primary.name());
    
    findChild<QWidget*>("inputContainer")->setStyleSheet(inputContainerStyle);
    
    // Text input styling
    QString textInputStyle = QString(R"(
        QTextEdit#messageInput {
            background: transparent;
            border: none;
            font-size: %1px;
            line-height: %2;
            color: %3;
            font-family: %4;
            padding: 0px;
        }
        QTextEdit#messageInput:focus {
            outline: none;
        }
    )").arg(tokens.fontSizeNormal)
       .arg(tokens.lineHeightNormal)
       .arg(tokens.text.name())
       .arg(tokens.fontFamily.family());
    
    m_textEdit->setStyleSheet(textInputStyle);
    
    // Action buttons styling
    QString actionButtonStyle = QString(R"(
        QToolButton#composerAction {
            background: transparent;
            border: none;
            border-radius: %1px;
            color: %2;
            padding: 8px;
        }
        QToolButton#composerAction:hover {
            background: %3;
        }
        QToolButton#composerAction:pressed {
            background: %4;
        }
    )").arg(tokens.radiusMedium)
       .arg(tokens.textSecondary.name())
       .arg(tokens.surfaceHover.name())
       .arg(tokens.border.name());
    
    for (auto *button : findChildren<QToolButton*>()) {
        if (button->objectName() == "composerAction") {
            button->setStyleSheet(actionButtonStyle);
        }
    }
    
    // Send button styling (primary)
    QString sendButtonStyle = QString(R"(
        QPushButton#sendButton {
            background: %1;
            border: none;
            border-radius: %2px;
            color: white;
            font-weight: 600;
            padding: 8px;
        }
        QPushButton#sendButton:hover:enabled {
            background: %3;
        }
        QPushButton#sendButton:pressed:enabled {
            background: %4;
        }
        QPushButton#sendButton:disabled {
            background: %5;
            color: %6;
        }
    )").arg(tokens.primary.name())
       .arg(tokens.radiusMedium)
       .arg(tokens.primaryHover.name())
       .arg(tokens.primary.name())
       .arg(tokens.border.name())
       .arg(tokens.textMuted.name());
    
    m_sendButton->setStyleSheet(sendButtonStyle);
    
    // Status labels styling
    QString statusStyle = QString(R"(
        QLabel#characterCount {
            color: %1;
            font-size: %2px;
        }
        QLabel#shortcutHint {
            color: %3;
            font-size: %4px;
            font-style: italic;
        }
    )").arg(tokens.textMuted.name())
       .arg(tokens.fontSizeSmall)
       .arg(tokens.textMuted.name())
       .arg(tokens.fontSizeXs);
    
    for (auto *label : findChildren<QLabel*>()) {
        if (label->objectName() == "characterCount" || label->objectName() == "shortcutHint") {
            label->setStyleSheet(statusStyle);
        }
    }
}

void MessageComposer::setFocus()
{
    m_textEdit->setFocus();
}

void MessageComposer::clear()
{
    m_textEdit->clear();
    m_attachments.clear();
    updateAttachmentsDisplay();
    updateSendButton();
    updateWordCount();
}

void MessageComposer::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    m_textEdit->setEnabled(enabled);
    m_sendButton->setEnabled(enabled && hasContent());
    m_attachButton->setEnabled(enabled);
    m_voiceButton->setEnabled(enabled);
}

bool MessageComposer::hasContent() const
{
    return !m_textEdit->toPlainText().trimmed().isEmpty() || !m_attachments.isEmpty();
}

void MessageComposer::onSendClicked()
{
    QString text = m_textEdit->toPlainText().trimmed();
    
    // Don't send if empty
    if (text.isEmpty() && m_attachments.isEmpty()) {
        return;
    }
    
    // Check provider availability before sending
    auto *app = Application::instance();
    auto *providerManager = app->providerManager();
    
    if (!providerManager || !providerManager->activeProvider()) {
        qWarning() << "Cannot send message: no provider available";
        return;
    }
    
    if (providerManager->activeProvider()->status() != AIProvider::Status::Connected) {
        qWarning() << "Cannot send message: provider not ready";
        return;
    }

    emit messageSent(text, m_attachments);
    clear();
}

void MessageComposer::onAttachClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "Select files to attach",
        QString(),
        "All supported files (*.jpg *.jpeg *.png *.gif *.bmp *.pdf *.txt *.md *.doc *.docx *.mp3 *.wav *.m4a);;All files (*.*)"
    );

    for (const QString &fileName : fileNames) {
        addAttachment(fileName);
    }
}

void MessageComposer::onVoiceClicked()
{
    if (m_isRecording) {
        // Stop recording
        m_voiceButton->setChecked(false);
        m_voiceButton->setIcon(Application::instance()->iconRegistry()->icon("microphone"));
        m_voiceButton->setToolTip("Record voice message");
        m_isRecording = false;
        
        emit voiceRecordingStopped(""); // TODO: Implement actual recording
    } else {
        // Start recording
        m_voiceButton->setIcon(Application::instance()->iconRegistry()->icon("stop"));
        m_voiceButton->setToolTip("Stop recording");
        m_isRecording = true;
        
        emit voiceRecordingStarted();
    }
}

void MessageComposer::onTextChanged()
{
    updateSendButton();
    updateWordCount();
    // Auto-resize is now handled in setupUI lambda
}

void MessageComposer::onAttachmentContextMenu(const QPoint &pos)
{
    // Find which attachment widget was clicked
    QWidget *widget = childAt(pos);
    AttachmentWidget *attachmentWidget = qobject_cast<AttachmentWidget*>(widget);
    
    if (attachmentWidget) {
        m_contextAttachmentId = attachmentWidget->attachment().id;
        m_attachmentMenu->exec(mapToGlobal(pos));
    }
}

void MessageComposer::onRemoveAttachment()
{
    if (!m_contextAttachmentId.isEmpty()) {
        removeAttachment(m_contextAttachmentId);
        m_contextAttachmentId.clear();
    }
}

void MessageComposer::onRetryAttachment()
{
    // TODO: Implement attachment retry logic
    qDebug() << "Retry attachment:" << m_contextAttachmentId;
}

void MessageComposer::updateSendButton()
{
    auto *app = Application::instance();
    auto *providerManager = app->providerManager();
    
    bool hasContent = this->hasContent();
    bool hasProvider = providerManager && providerManager->activeProvider();
    bool isProviderOnline = hasProvider && 
        (providerManager->activeProvider()->status() == AIProvider::Status::Connected);
    
    // Enable send button only if we have content AND have a provider that's ready
    m_sendButton->setEnabled(hasContent && isProviderOnline);
    
    // Update tooltip and visual state based on connection status
    if (!hasContent) {
        m_sendButton->setToolTip("Type a message to send");
        m_sendButton->setProperty("state", "disabled");
    } else if (!hasProvider) {
        m_sendButton->setToolTip("No provider available - check connection");
        m_sendButton->setProperty("state", "offline");
    } else if (!isProviderOnline) {
        m_sendButton->setToolTip("Provider offline - trying to reconnect");
        m_sendButton->setProperty("state", "connecting");
    } else {
        m_sendButton->setToolTip("Send message (Enter)");
        m_sendButton->setProperty("state", "ready");
    }
    
    // Trigger style update for state-based styling
    m_sendButton->style()->unpolish(m_sendButton);
    m_sendButton->style()->polish(m_sendButton);
}

void MessageComposer::addAttachment(const QString &filePath)
{
    auto *app = Application::instance();
    auto *fileVault = app->fileVault();
    
    // Store file in vault
    QString vaultPath = fileVault->storeFile(filePath);
    if (vaultPath.isEmpty()) {
        qWarning() << "Failed to store attachment:" << filePath;
        return;
    }
    
    // Create attachment object
    Attachment attachment("", QFileInfo(filePath).fileName(), vaultPath);
    m_attachments.append(attachment);
    
    updateAttachmentsDisplay();
    updateSendButton();
    
    emit attachmentAdded(filePath);
}

void MessageComposer::removeAttachment(const QString &attachmentId)
{
    for (int i = 0; i < m_attachments.size(); ++i) {
        if (m_attachments[i].id == attachmentId) {
            m_attachments.removeAt(i);
            break;
        }
    }
    
    updateAttachmentsDisplay();
    updateSendButton();
    
    emit attachmentRemoved(attachmentId);
}

void MessageComposer::updateAttachmentsDisplay()
{
    // Clear existing attachment widgets
    while (m_attachmentsLayout->count() > 1) { // Keep the stretch
        QLayoutItem *item = m_attachmentsLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Add attachment widgets
    for (const Attachment &attachment : m_attachments) {
        auto *attachmentWidget = new AttachmentWidget(attachment, this);
        connect(attachmentWidget, &AttachmentWidget::removeRequested,
                this, &MessageComposer::removeAttachment);
        connect(attachmentWidget, &AttachmentWidget::contextMenuRequested,
                this, &MessageComposer::onAttachmentContextMenu);
        
        // Insert before stretch
        int insertIndex = m_attachmentsLayout->count() - 1;
        m_attachmentsLayout->insertWidget(insertIndex, attachmentWidget);
    }
    
    // Show/hide attachments area
    m_attachmentsWidget->setVisible(!m_attachments.isEmpty());
    
    // Adjust composer height
    int newHeight = 120;
    if (!m_attachments.isEmpty()) {
        newHeight += 60; // Space for attachments
    }
    setFixedHeight(newHeight);
}

void MessageComposer::updateWordCount()
{
    QString text = m_textEdit->toPlainText();
    int charCount = text.length();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).length();
    
    // Show both character and word count
    if (wordCount == 0) {
        m_wordCountLabel->setText("0 characters");
    } else {
        m_wordCountLabel->setText(QString("%1 characters • %2 words").arg(charCount).arg(wordCount));
    }
    
    // Apply theme-aware coloring based on usage
    auto *themeManager = Application::instance()->themeManager();
    const auto &tokens = themeManager->tokens();
    
    QString color = tokens.textMuted.name();
    if (charCount > 3000) {
        color = tokens.error.name(); // Red for very long messages
    } else if (charCount > 2000) {
        color = tokens.warning.name(); // Orange for long messages
    }
    
    m_wordCountLabel->setStyleSheet(QString("color: %1; font-size: %2px;")
                                   .arg(color).arg(tokens.fontSizeSmall));
}

void MessageComposer::resizeTextEdit()
{
    // Auto-resize text edit based on content
    QTextDocument *doc = m_textEdit->document();
    int height = doc->size().height() + 10;
    
    height = qMax(MIN_HEIGHT - 40, height); // Account for margins
    height = qMin(MAX_HEIGHT - 40, height);
    
    m_textEdit->setFixedHeight(height);
}

bool MessageComposer::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            bool shift = keyEvent->modifiers().testFlag(Qt::ShiftModifier);
            bool ctrl = keyEvent->modifiers().testFlag(Qt::ControlModifier) || keyEvent->modifiers().testFlag(Qt::MetaModifier);
            
            // Rule: Enter sends if no Shift; Shift+Enter inserts newline. Ctrl+Enter always sends.
            if (!shift || ctrl) {
                // Only send if button is enabled (which checks provider status)
                if (m_sendButton->isEnabled()) {
                    onSendClicked();
                }
                return true; // consume the event either way
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// AttachmentWidget implementation
AttachmentWidget::AttachmentWidget(const Attachment &attachment, QWidget *parent)
    : QWidget(parent)
    , m_attachment(attachment)
    , m_hasError(false)
{
    setupUI();
    setFixedSize(100, 60);
}

void AttachmentWidget::setupUI()
{
    setStyleSheet(R"(
        AttachmentWidget {
            background-color: #F3F4F6;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
        }
        AttachmentWidget:hover {
            background-color: #E5E7EB;
        }
    )");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(2);
    
    // Thumbnail or icon
    m_thumbnailLabel = new QLabel();
    m_thumbnailLabel->setAlignment(Qt::AlignCenter);
    m_thumbnailLabel->setFixedSize(32, 32);
    m_thumbnailLabel->setStyleSheet("background-color: #D1D5DB; border-radius: 4px;");
    layout->addWidget(m_thumbnailLabel, 0, Qt::AlignCenter);
    
    // File name (truncated)
    m_nameLabel = new QLabel();
    QString fileName = m_attachment.fileName;
    if (fileName.length() > 12) {
        fileName = fileName.left(9) + "...";
    }
    m_nameLabel->setText(fileName);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("font-size: 10px; color: #374151;");
    layout->addWidget(m_nameLabel);
    
    updateThumbnail();
}

void AttachmentWidget::updateThumbnail()
{
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();
    
    QString iconName;
    switch (m_attachment.type) {
    case AttachmentType::Image:
        iconName = "image";
        break;
    case AttachmentType::Audio:
        iconName = "microphone";
        break;
    default:
        iconName = "attach";
        break;
    }
    
    // For now, use icons. In a full implementation, generate actual thumbnails for images
    QPixmap pixmap = iconRegistry->pixmap(iconName, QSize(24, 24));
    m_thumbnailLabel->setPixmap(pixmap);
}

void AttachmentWidget::setError(const QString &error)
{
    m_errorText = error;
    m_hasError = true;
    setStyleSheet(R"(
        AttachmentWidget {
            background-color: #FEE2E2;
            border: 1px solid #FECACA;
            border-radius: 6px;
        }
    )");
    update();
}

void AttachmentWidget::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(event->pos(), m_attachment.id);
}

void AttachmentWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    if (m_hasError) {
        QPainter painter(this);
        painter.setPen(QPen(QColor("#EF4444"), 1));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(rect().adjusted(2, 2, -2, -2), Qt::AlignBottom | Qt::TextWordWrap, "Error");
    }
}

} // namespace DesktopApp
