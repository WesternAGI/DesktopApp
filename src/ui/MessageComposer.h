#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QToolButton>
#include <QMenu>
#include "data/Models.h"

namespace GadAI {

/**
 * @brief Message composer widget with attachment and voice support
 */
class MessageComposer : public QWidget
{
    Q_OBJECT

public:
    explicit MessageComposer(QWidget *parent = nullptr);

    /**
     * @brief Set focus to the input field
     */
    void setFocus();

    /**
     * @brief Clear the composer
     */
    void clear();

    /**
     * @brief Set enabled state
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if there's content to send
     */
    bool hasContent() const;

signals:
    void messageSent(const QString &text, const AttachmentList &attachments);
    void attachmentAdded(const QString &filePath);
    void attachmentRemoved(const QString &attachmentId);
    void voiceRecordingStarted();
    void voiceRecordingStopped(const QString &audioPath);

private slots:
    void onSendClicked();
    void onAttachClicked();
    void onVoiceClicked();
    void onTextChanged();
    void onAttachmentContextMenu(const QPoint &pos);
    void onRemoveAttachment();
    void onRetryAttachment();

private:
    void setupUI();
    void connectSignals();
    void updateSendButton();
    void addAttachment(const QString &filePath);
    void removeAttachment(const QString &attachmentId);
    void updateAttachmentsDisplay();
    void updateWordCount();
    void resizeTextEdit();
    bool eventFilter(QObject *obj, QEvent *event) override;

    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Attachments area
    QWidget *m_attachmentsWidget;
    QHBoxLayout *m_attachmentsLayout;
    
    // Input area
    QHBoxLayout *m_inputLayout;
    QTextEdit *m_textEdit;
    QToolButton *m_attachButton;
    QToolButton *m_voiceButton;
    QPushButton *m_sendButton;
    
    // Status area
    QHBoxLayout *m_statusLayout;
    QLabel *m_wordCountLabel;
    QComboBox *m_promptCombo;
    QProgressBar *m_progressBar;
    
    // Context menu for attachments
    QMenu *m_attachmentMenu;
    QAction *m_removeAttachmentAction;
    QAction *m_retryAttachmentAction;
    
    // State
    AttachmentList m_attachments;
    QString m_contextAttachmentId;
    bool m_isRecording;
    
    // Constants
    static const int MAX_CHARS = 4000;
    static const int MIN_HEIGHT = 100;
    static const int MAX_HEIGHT = 300;
};

/**
 * @brief Widget for displaying individual attachments
 */
class AttachmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AttachmentWidget(const Attachment &attachment, QWidget *parent = nullptr);

    const Attachment& attachment() const { return m_attachment; }
    void setError(const QString &error);

signals:
    void removeRequested(const QString &attachmentId);
    void contextMenuRequested(const QPoint &pos, const QString &attachmentId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void updateThumbnail();

    Attachment m_attachment;
    QString m_errorText;
    
    QLabel *m_thumbnailLabel;
    QLabel *m_nameLabel;
    QLabel *m_sizeLabel;
    QPushButton *m_removeButton;
    
    bool m_hasError;
};

} // namespace GadAI
