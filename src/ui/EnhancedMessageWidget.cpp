#include "EnhancedMessageWidget.h"
#include "core/Application.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"

#include <QDateTime>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsDropShadowEffect>
#include <QEnterEvent>

namespace DesktopApp {

EnhancedMessageWidget::EnhancedMessageWidget(const Message& message, QWidget *parent)
    : QWidget(parent)
    , m_message(message)
    , m_isStreaming(false)
    , m_isGenerating(false)
    , m_actionsVisible(false)
{
    setupUI();
    setupActions();
    updateContent(message.text);
    setTimestamp(message.createdAt);
    
    // Connect to theme changes
    auto *themeManager = Application::instance()->themeManager();
    connect(themeManager, &ThemeManager::themeChanged, this, &EnhancedMessageWidget::onThemeChanged);
    
    // Initial styling
    onThemeChanged();
    
    // Start timestamp update timer
    m_timestampTimer = new QTimer(this);
    connect(m_timestampTimer, &QTimer::timeout, this, &EnhancedMessageWidget::updateTimestamp);
    m_timestampTimer->start(60000); // Update every minute
}

void EnhancedMessageWidget::setupUI()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 12, 16, 12);
    m_mainLayout->setSpacing(8);
    
    // Header with author and timestamp
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(8);
    
    m_authorLabel = new QLabel();
    m_authorLabel->setObjectName("messageAuthor");
    
    // Set author based on message role
    if (m_message.role == MessageRole::User) {
        m_authorLabel->setText("You");
    } else if (m_message.role == MessageRole::Assistant) {
        m_authorLabel->setText("Assistant");
    } else if (m_message.role == MessageRole::System) {
        m_authorLabel->setText("System");
    } else {
        m_authorLabel->setText("Unknown");
    }
    
    m_timestampLabel = new QLabel();
    m_timestampLabel->setObjectName("messageTimestamp");
    
    m_headerLayout->addWidget(m_authorLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_timestampLabel);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // Content area
    m_contentText = new QTextEdit();
    m_contentText->setObjectName("messageContent");
    m_contentText->setReadOnly(true);
    m_contentText->setFrameStyle(QFrame::NoFrame);
    m_contentText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_contentText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_contentText->setLineWrapMode(QTextEdit::WidgetWidth);
    m_contentText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    
    // Enable rich text and links
    m_contentText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    
    m_mainLayout->addWidget(m_contentText);
    
    // Actions widget (initially hidden)
    m_actionsWidget = new QWidget();
    m_actionsLayout = new QHBoxLayout(m_actionsWidget);
    m_actionsLayout->setContentsMargins(0, 4, 0, 0);
    m_actionsLayout->setSpacing(4);
    m_actionsLayout->addStretch();
    
    m_mainLayout->addWidget(m_actionsWidget);
    
    // Setup opacity effect for actions
    m_actionsOpacity = new QGraphicsOpacityEffect(m_actionsWidget);
    m_actionsWidget->setGraphicsEffect(m_actionsOpacity);
    m_actionsOpacity->setOpacity(0.0);
    
    // Setup animation
    m_actionsAnimation = new QPropertyAnimation(m_actionsOpacity, "opacity", this);
    m_actionsAnimation->setDuration(200);
    m_actionsAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void EnhancedMessageWidget::setupActions()
{
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();
    
    // Copy button
    m_copyButton = new QPushButton();
    m_copyButton->setIcon(iconRegistry->icon("copy"));
    m_copyButton->setToolTip("Copy message");
    m_copyButton->setFixedSize(32, 32);
    m_copyButton->setObjectName("messageAction");
    connect(m_copyButton, &QPushButton::clicked, this, &EnhancedMessageWidget::onCopyClicked);
    m_actionsLayout->addWidget(m_copyButton);
    
    // Regenerate button (only for assistant messages)
    if (m_message.role == MessageRole::Assistant) {
        m_regenerateButton = new QPushButton();
        m_regenerateButton->setIcon(iconRegistry->icon("refresh"));
        m_regenerateButton->setToolTip("Regenerate response");
        m_regenerateButton->setFixedSize(32, 32);
        m_regenerateButton->setObjectName("messageAction");
        connect(m_regenerateButton, &QPushButton::clicked, this, &EnhancedMessageWidget::onRegenerateClicked);
        m_actionsLayout->addWidget(m_regenerateButton);
    }
    
    // Edit button (only for user messages)
    if (m_message.role == MessageRole::User) {
        m_editButton = new QPushButton();
        m_editButton->setIcon(iconRegistry->icon("edit"));
        m_editButton->setToolTip("Edit message");
        m_editButton->setFixedSize(32, 32);
        m_editButton->setObjectName("messageAction");
        connect(m_editButton, &QPushButton::clicked, this, &EnhancedMessageWidget::onEditClicked);
        m_actionsLayout->addWidget(m_editButton);
    }
    
    // More actions menu
    m_moreButton = new QPushButton();
    m_moreButton->setIcon(iconRegistry->icon("more"));
    m_moreButton->setToolTip("More actions");
    m_moreButton->setFixedSize(32, 32);
    m_moreButton->setObjectName("messageAction");
    
    m_moreMenu = new QMenu(this);
    m_moreMenu->addAction(iconRegistry->icon("delete"), "Delete", this, &EnhancedMessageWidget::onDeleteClicked);
    m_moreMenu->addAction(iconRegistry->icon("share"), "Share", [this]() {
        // TODO: Implement share functionality
    });
    
    m_moreButton->setMenu(m_moreMenu);
    m_actionsLayout->addWidget(m_moreButton);
}

void EnhancedMessageWidget::updateContent(const QString& content)
{
    m_message.text = content;
    
    // Process markdown and set content
    QString processedContent = processMarkdown(content);
    m_contentText->setHtml(processedContent);
    
    // Adjust height to content
    QTextDocument *doc = m_contentText->document();
    doc->setTextWidth(m_contentText->width());
    int height = doc->size().height() + 10; // Add some padding
    m_contentText->setFixedHeight(qMax(30, height));
}

void EnhancedMessageWidget::setStreaming(bool streaming)
{
    m_isStreaming = streaming;
    
    if (streaming && !m_streamingTimer) {
        // Add streaming indicator
        m_streamingTimer = new QTimer(this);
        connect(m_streamingTimer, &QTimer::timeout, [this]() {
            // Add streaming cursor animation
            QString currentText = m_contentText->toPlainText();
            if (currentText.endsWith("▋")) {
                m_contentText->setPlainText(currentText.chopped(1));
            } else {
                m_contentText->setPlainText(currentText + "▋");
            }
        });
        m_streamingTimer->start(500);
    } else if (!streaming && m_streamingTimer) {
        m_streamingTimer->stop();
        m_streamingTimer->deleteLater();
        m_streamingTimer = nullptr;
        
        // Remove streaming cursor
        QString currentText = m_contentText->toPlainText();
        if (currentText.endsWith("▋")) {
            m_contentText->setPlainText(currentText.chopped(1));
        }
    }
}

void EnhancedMessageWidget::setGenerating(bool generating)
{
    m_isGenerating = generating;
    
    // Show/hide regenerate button based on generating state
    if (m_regenerateButton) {
        m_regenerateButton->setEnabled(!generating);
        m_regenerateButton->setToolTip(generating ? "Generating..." : "Regenerate response");
    }
}

void EnhancedMessageWidget::setTimestamp(const QDateTime& timestamp)
{
    m_message.createdAt = timestamp;
    updateTimestamp();
}

void EnhancedMessageWidget::showActions(bool show)
{
    if (m_actionsVisible == show) return;
    
    m_actionsVisible = show;
    animateActions(show);
}

void EnhancedMessageWidget::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    showActions(true);
}

void EnhancedMessageWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    showActions(false);
}

void EnhancedMessageWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    // Custom painting for message bubble effect
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    auto *themeManager = Application::instance()->themeManager();
    const auto &tokens = themeManager->tokens();
    
    // Determine background color based on message role
    QColor bgColor = tokens.surface;
    if (m_message.role == MessageRole::User) {
        bgColor = tokens.userMessage;
    } else if (m_message.role == MessageRole::Assistant) {
        bgColor = tokens.assistantMessage;
    } else if (m_message.role == MessageRole::System) {
        bgColor = tokens.systemMessage;
    }
    
    // Draw rounded background
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), tokens.radiusMedium, tokens.radiusMedium);
}

void EnhancedMessageWidget::onCopyClicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_message.text);
    emit copyRequested(m_message.text);
}

void EnhancedMessageWidget::onRegenerateClicked()
{
    emit regenerateRequested(m_message.id);
}

void EnhancedMessageWidget::onEditClicked()
{
    emit editRequested(m_message.id);
}

void EnhancedMessageWidget::onDeleteClicked()
{
    emit deleteRequested(m_message.id);
}

void EnhancedMessageWidget::onThemeChanged()
{
    updateStyling();
}

void EnhancedMessageWidget::updateStyling()
{
    auto *themeManager = Application::instance()->themeManager();
    const auto &tokens = themeManager->tokens();
    
    // Author label styling
    m_authorLabel->setStyleSheet(QString(R"(
        QLabel#messageAuthor {
            font-size: %1px;
            font-weight: 600;
            color: %2;
            margin-bottom: 2px;
        }
    )").arg(tokens.fontSizeSmall)
       .arg(tokens.textSecondary.name()));
    
    // Timestamp label styling
    m_timestampLabel->setStyleSheet(QString(R"(
        QLabel#messageTimestamp {
            font-size: %1px;
            color: %2;
        }
    )").arg(tokens.fontSizeXs)
       .arg(tokens.textMuted.name()));
    
    // Content text styling
    m_contentText->setStyleSheet(QString(R"(
        QTextEdit#messageContent {
            background: transparent;
            border: none;
            font-size: %1px;
            line-height: %2;
            color: %3;
            font-family: "%4";
            selection-background-color: %5;
        }
    )").arg(tokens.fontSizeNormal)
       .arg(tokens.lineHeightNormal)
       .arg(tokens.text.name())
       .arg(tokens.fontFamily.family())
       .arg(tokens.primary.name()));
    
    // Action buttons styling
    QString actionButtonStyle = QString(R"(
        QPushButton#messageAction {
            background: %1;
            border: 1px solid %2;
            border-radius: %3px;
            color: %4;
            padding: 6px;
        }
        QPushButton#messageAction:hover {
            background: %5;
            border-color: %6;
        }
        QPushButton#messageAction:pressed {
            background: %7;
        }
    )").arg(tokens.surface.name())
       .arg(tokens.border.name())
       .arg(tokens.radiusSmall)
       .arg(tokens.textSecondary.name())
       .arg(tokens.surfaceHover.name())
       .arg(tokens.primary.name())
       .arg(tokens.border.name());
    
    for (auto *button : m_actionsWidget->findChildren<QPushButton*>()) {
        button->setStyleSheet(actionButtonStyle);
    }
}

void EnhancedMessageWidget::updateTimestamp()
{
    m_timestampLabel->setText(formatTimestamp(m_message.createdAt));
}

void EnhancedMessageWidget::animateActions(bool show)
{
    m_actionsAnimation->stop();
    m_actionsAnimation->setStartValue(m_actionsOpacity->opacity());
    m_actionsAnimation->setEndValue(show ? 1.0 : 0.0);
    m_actionsAnimation->start();
}

QString EnhancedMessageWidget::formatTimestamp(const QDateTime& timestamp) const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = timestamp.secsTo(now);
    
    if (secondsAgo < 60) {
        return "Just now";
    } else if (secondsAgo < 3600) {
        int minutes = secondsAgo / 60;
        return QString("%1m ago").arg(minutes);
    } else if (secondsAgo < 86400) {
        int hours = secondsAgo / 3600;
        return QString("%1h ago").arg(hours);
    } else if (secondsAgo < 604800) {
        int days = secondsAgo / 86400;
        return QString("%1d ago").arg(days);
    } else {
        return timestamp.toString("MMM dd");
    }
}

QString EnhancedMessageWidget::processMarkdown(const QString& content) const
{
    // Use the existing SimpleMarkdown processor but enhance it
    // Note: SimpleMarkdown is not accessible here, so we'll do basic HTML processing
    QString html = content;
    
    // Convert newlines to HTML breaks
    html.replace("\n", "<br>");
    
    // Basic markdown processing
    // Bold text
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<strong>\\1</strong>");
    
    // Italic text
    html.replace(QRegularExpression("\\*(.*?)\\*"), "<em>\\1</em>");
    
    // Code blocks (triple backticks)
    html.replace(QRegularExpression("```([\\s\\S]*?)```"), "<pre><code>\\1</code></pre>");
    
    // Inline code
    html.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    
    // Links
    html.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^\\)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    
    auto *themeManager = Application::instance()->themeManager();
    const auto &tokens = themeManager->tokens();
    
    // Add custom styling for code blocks
    html.replace("<pre>", QString("<pre style=\"background: %1; border: 1px solid %2; border-radius: %3px; padding: %4px; margin: %5px 0; font-family: %6; font-size: %7px;\">")
                 .arg(tokens.codeBackground.name())
                 .arg(tokens.codeBorder.name())
                 .arg(tokens.radiusSmall)
                 .arg(tokens.spacingS)
                 .arg(tokens.spacingS)
                 .arg(tokens.fontFamilyMono.family())
                 .arg(tokens.fontSizeSmall));
    
    // Style inline code
    html.replace("<code>", QString("<code style=\"background: %1; border-radius: %2px; padding: 2px 4px; font-family: %3; font-size: %4px;\">")
                 .arg(tokens.codeBackground.name())
                 .arg(tokens.radiusSmall)
                 .arg(tokens.fontFamilyMono.family())
                 .arg(tokens.fontSizeSmall));
    
    // Style links
    html.replace("<a ", QString("<a style=\"color: %1; text-decoration: none;\" ")
                 .arg(tokens.linkColor.name()));
    
    return html;
}

} // namespace DesktopApp