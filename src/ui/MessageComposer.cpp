#include "MessageComposer.h"
#include "core/Application.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"
#include "services/FileVault.h"
#include "services/AudioRecorder.h"

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
    setFixedHeight(120);
    
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 8, 16, 8);
    m_mainLayout->setSpacing(8);

    // Attachments area (initially hidden)
    m_attachmentsWidget = new QWidget();
    m_attachmentsLayout = new QHBoxLayout(m_attachmentsWidget);
    m_attachmentsLayout->setContentsMargins(0, 0, 0, 0);
    m_attachmentsLayout->setSpacing(8);
    m_attachmentsLayout->addStretch();
    m_attachmentsWidget->hide();
    m_mainLayout->addWidget(m_attachmentsWidget);

    // Input area
    QWidget *inputWidget = new QWidget();
    m_inputLayout = new QHBoxLayout(inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(8);

    // Text input
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("Type your message...");
    m_textEdit->setMaximumHeight(80);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setAcceptRichText(false);
    m_inputLayout->addWidget(m_textEdit, 1);

    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();

    // Attach button
    m_attachButton = new QToolButton();
    m_attachButton->setIcon(iconRegistry->icon("attach"));
    m_attachButton->setToolTip("Attach files");
    m_attachButton->setFixedSize(36, 36);
    m_inputLayout->addWidget(m_attachButton);

    // Voice button
    m_voiceButton = new QToolButton();
    m_voiceButton->setIcon(iconRegistry->icon("microphone"));
    m_voiceButton->setToolTip("Record voice message");
    m_voiceButton->setFixedSize(36, 36);
    m_voiceButton->setCheckable(true);
    m_inputLayout->addWidget(m_voiceButton);

    // Send button
    m_sendButton = new QPushButton();
    m_sendButton->setIcon(iconRegistry->icon("send"));
    m_sendButton->setToolTip("Send message");
    m_sendButton->setFixedSize(36, 36);
    m_sendButton->setProperty("class", "primary");
    m_sendButton->setEnabled(false);
    m_inputLayout->addWidget(m_sendButton);

    m_mainLayout->addWidget(inputWidget);

    // Status area
    QWidget *statusWidget = new QWidget();
    m_statusLayout = new QHBoxLayout(statusWidget);
    m_statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLayout->setSpacing(16);

    // Word count
    m_wordCountLabel = new QLabel("0 / 4000");
    m_wordCountLabel->setStyleSheet("color: #6B7280; font-size: 12px;");
    m_statusLayout->addWidget(m_wordCountLabel);

    // Prompt selector
    m_promptCombo = new QComboBox();
    m_promptCombo->addItem("No prompt selected");
    m_promptCombo->setMinimumWidth(150);
    m_statusLayout->addWidget(m_promptCombo);

    m_statusLayout->addStretch();

    // Progress bar (hidden by default)
    m_progressBar = new QProgressBar();
    m_progressBar->setMaximumHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();
    m_statusLayout->addWidget(m_progressBar);

    m_mainLayout->addWidget(statusWidget);

    // Setup attachment context menu
    m_attachmentMenu = new QMenu(this);
    m_removeAttachmentAction = m_attachmentMenu->addAction(iconRegistry->icon("delete"), "Remove");
    m_retryAttachmentAction = m_attachmentMenu->addAction(iconRegistry->icon("send"), "Retry");
}

void MessageComposer::connectSignals()
{
    // Text input
    connect(m_textEdit, &QTextEdit::textChanged, this, &MessageComposer::onTextChanged);
    // Install event filter for Enter-to-send
    m_textEdit->installEventFilter(this);

    // Buttons
    connect(m_sendButton, &QPushButton::clicked, this, &MessageComposer::onSendClicked);
    connect(m_attachButton, &QToolButton::clicked, this, &MessageComposer::onAttachClicked);
    connect(m_voiceButton, &QToolButton::toggled, this, &MessageComposer::onVoiceClicked);

    // Context menu actions
    connect(m_removeAttachmentAction, &QAction::triggered, this, &MessageComposer::onRemoveAttachment);
    connect(m_retryAttachmentAction, &QAction::triggered, this, &MessageComposer::onRetryAttachment);

    // NOTE: Key handling for Ctrl+Enter send would require subclassing QTextEdit; omitted.
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
    
    if (text.isEmpty() && m_attachments.isEmpty()) {
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
    resizeTextEdit();
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
    m_sendButton->setEnabled(hasContent());
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
    
    m_wordCountLabel->setText(QString("%1 / %2").arg(charCount).arg(MAX_CHARS));
    
    // Change color based on usage
    if (charCount > MAX_CHARS * 0.9) {
        m_wordCountLabel->setStyleSheet("color: #EF4444; font-size: 12px;"); // Red
    } else if (charCount > MAX_CHARS * 0.7) {
        m_wordCountLabel->setStyleSheet("color: #F59E0B; font-size: 12px;"); // Orange
    } else {
        m_wordCountLabel->setStyleSheet("color: #6B7280; font-size: 12px;"); // Gray
    }
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
                onSendClicked();
                return true; // consume
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
