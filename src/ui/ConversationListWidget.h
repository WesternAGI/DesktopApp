#pragma once

#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QEnterEvent>
#include "data/Models.h"

namespace DesktopApp {

/**
 * @brief Widget for displaying and managing conversations list
 */
class ConversationListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConversationListWidget(QWidget *parent = nullptr);

    /**
     * @brief Refresh the conversations list
     */
    void refreshConversations();
    void refreshSectioned();

    /**
     * @brief Create a new conversation
     */
    void createNewConversation();

    /**
     * @brief Get the currently selected conversation
     */
    QString currentConversationId() const;

    /**
     * @brief Select a conversation by ID
     */
    void selectConversation(const QString &conversationId);

    /**
     * @brief Update widget styles based on current theme
     */
    void updateThemeStyles();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

signals:
    void conversationSelected(const QString &conversationId);
    void conversationRenamed(const QString &conversationId, const QString &newTitle);
    void conversationDeleted(const QString &conversationId);
    void conversationSoftDeleted(const QString &conversationId);
    void undoRequested(const QString &conversationId);

private slots:
    void onSearchTextChanged(const QString &text);
    void onNewConversationClicked();
    void onConversationItemClicked(QListWidgetItem *item);
    void onInlineRenameFinished();
    void onConversationContextMenu(const QPoint &pos);
    void onRenameConversation();
    void onDeleteConversation();
    void onConversationItemEntered(QListWidgetItem *item);
    void onHoverRenameClicked();
    void onHoverDeleteClicked();

private:
    void setupUI();
    void setupHoverActions();
    void showHoverActions(QListWidgetItem *item);
    void hideHoverActions();
    void updateHoverActionsPosition();
    void connectSignals();
    void populateConversations(const ConversationList &conversations);
    void updateConversationItem(QListWidgetItem *item, const Conversation &conversation);
    Conversation getConversationFromItem(QListWidgetItem *item) const;
    void showRenameDialog(const QString &conversationId);

    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QLineEdit *m_searchEdit;
    QPushButton *m_newConversationBtn;
    QListWidget *m_conversationsList;
    QLineEdit *m_inlineRenameEdit {nullptr};
    QListWidgetItem *m_inlineRenameItem {nullptr};
    QLabel *m_statusLabel;

    // Hover action buttons
    QWidget *m_hoverActionsWidget;
    QPushButton *m_hoverRenameButton;
    QPushButton *m_hoverDeleteButton;
    QListWidgetItem *m_currentHoverItem;

    // Context menu
    QMenu *m_contextMenu;
    QAction *m_renameAction;
    QAction *m_deleteAction;
    QAction *m_pinAction;
    QAction *m_archiveAction;

    // Current state
    QString m_currentFilter;
    QListWidgetItem *m_contextMenuItem;
    QString m_pendingSoftDeleteId;
};

/**
 * @brief Custom list widget item for conversations
 */
class ConversationListItem : public QListWidgetItem
{
public:
    explicit ConversationListItem(const Conversation &conversation, QListWidget *parent = nullptr);

    void updateFromConversation(const Conversation &conversation);
    const Conversation& conversation() const { return m_conversation; }

private:
    void updateDisplay();

    Conversation m_conversation;
};

} // namespace DesktopApp
