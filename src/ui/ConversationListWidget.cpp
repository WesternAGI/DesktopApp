#include "ConversationListWidget.h"
#include "core/Application.h"
#include "data/JsonStore.h"
#include "theme/ThemeManager.h"
#include "theme/IconRegistry.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include "services/SearchEngine.h"
#include <QTimer>

namespace DesktopApp {

ConversationListWidget::ConversationListWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_newConversationBtn(nullptr)
    , m_conversationsList(nullptr)
    , m_statusLabel(nullptr)
    , m_hoverActionsWidget(nullptr)
    , m_hoverRenameButton(nullptr)
    , m_hoverDeleteButton(nullptr)
    , m_currentHoverItem(nullptr)
    , m_contextMenu(nullptr)
    , m_contextMenuItem(nullptr)
{
    setupUI();
    setupHoverActions();
    connectSignals();
    refreshConversations();
}

void ConversationListWidget::setupUI()
{
    setFixedWidth(320);
    
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);

    // Header with search and new conversation button
    m_headerLayout = new QHBoxLayout();
    
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();

    // Search input
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search conversations...");
    m_searchEdit->setClearButtonEnabled(true);
    m_headerLayout->addWidget(m_searchEdit, 1);

    // New conversation button
    m_newConversationBtn = new QPushButton();
    m_newConversationBtn->setIcon(iconRegistry->icon("new-chat"));
    m_newConversationBtn->setToolTip("New Conversation");
    m_newConversationBtn->setFixedSize(32, 32);
    m_newConversationBtn->setProperty("class", "primary");
    m_headerLayout->addWidget(m_newConversationBtn);

    m_mainLayout->addLayout(m_headerLayout);

    // Conversations list
    m_conversationsList = new QListWidget();
    m_conversationsList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_conversationsList->setAlternatingRowColors(true);
    m_conversationsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mainLayout->addWidget(m_conversationsList, 1);

    // Status label
    m_statusLabel = new QLabel("No conversations");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    // Style will be set by updateThemeStyles() after theme manager is ready
    m_mainLayout->addWidget(m_statusLabel);

    // Setup context menu
    m_contextMenu = new QMenu(this);
    m_renameAction = m_contextMenu->addAction(iconRegistry->icon("edit"), "Rename");
    m_deleteAction = m_contextMenu->addAction(iconRegistry->icon("delete"), "Delete");
}

void ConversationListWidget::connectSignals()
{
    // Search functionality
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &ConversationListWidget::onSearchTextChanged);

    // New conversation button
    connect(m_newConversationBtn, &QPushButton::clicked,
            this, &ConversationListWidget::onNewConversationClicked);

    // List selection
    connect(m_conversationsList, &QListWidget::itemClicked,
            this, &ConversationListWidget::onConversationItemClicked);

    // Context menu
    connect(m_conversationsList, &QListWidget::customContextMenuRequested,
            this, &ConversationListWidget::onConversationContextMenu);

    // Context menu actions
    connect(m_renameAction, &QAction::triggered,
            this, &ConversationListWidget::onRenameConversation);
    connect(m_deleteAction, &QAction::triggered,
            this, &ConversationListWidget::onDeleteConversation);

    // Enable mouse tracking for hover effects
    m_conversationsList->setMouseTracking(true);
    m_conversationsList->installEventFilter(this);

    // Database updates
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    connect(store, &JsonStore::conversationCreated,
            this, [this](const QString &) { refreshConversations(); });
    connect(store, &JsonStore::conversationUpdated,
            this, [this](const QString &) { refreshConversations(); });
    connect(store, &JsonStore::conversationDeleted,
            this, [this](const QString &) { refreshConversations(); });
}

void ConversationListWidget::refreshConversations()
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    m_conversationsList->clear();

    // Get all active conversations (non-archived, non-deleted)
    ConversationList conversations = store->getRecentConversations(100);
    
    // Apply search filter if active
    if (!m_currentFilter.isEmpty()) {
        auto *searchEngine = app->searchEngine();
        conversations = searchEngine->searchConversations(m_currentFilter, 50);
    }

    // Add conversations to list
    for (const auto &conversation : conversations) {
        if (conversation.archived || conversation.deleted) {
            continue; // Skip archived and deleted conversations
        }
        
        // Create list item
        auto *item = new ConversationListItem(conversation, m_conversationsList);
        m_conversationsList->addItem(item);
    }
    
    // Update status
    if (m_conversationsList->count()==0) {
        m_statusLabel->setText(m_currentFilter.isEmpty() ? 
                              "No conversations" : "No matches found");
        m_statusLabel->show();
    } else {
        m_statusLabel->hide();
    }
    
    qDebug() << "Refreshed conversations list (section) items:" << m_conversationsList->count();
}

void ConversationListWidget::createNewConversation()
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    
    Conversation newConversation("New Conversation");
    
    if (store->createConversation(newConversation)) {
        refreshConversations();
        selectConversation(newConversation.id);
        emit conversationSelected(newConversation.id);
        
        // Auto-rename the conversation
        QTimer::singleShot(100, [this, newConversation]() {
            showRenameDialog(newConversation.id);
        });
    }
}

QString ConversationListWidget::currentConversationId() const
{
    QListWidgetItem *currentItem = m_conversationsList->currentItem();
    if (!currentItem) {
        return QString();
    }
    
    ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(currentItem);
    return convItem ? convItem->conversation().id : QString();
}

void ConversationListWidget::selectConversation(const QString &conversationId)
{
    for (int i = 0; i < m_conversationsList->count(); ++i) {
        QListWidgetItem *item = m_conversationsList->item(i);
        ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(item);
        
        if (convItem && convItem->conversation().id == conversationId) {
            m_conversationsList->setCurrentItem(item);
            break;
        }
    }
}

void ConversationListWidget::onSearchTextChanged(const QString &text)
{
    m_currentFilter = text.trimmed();
    
    // Debounce search to avoid too many updates
    static QTimer *searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(300);
        connect(searchTimer, &QTimer::timeout, this, &ConversationListWidget::refreshConversations);
    }
    
    searchTimer->start();
}

void ConversationListWidget::onNewConversationClicked()
{
    createNewConversation();
}

void ConversationListWidget::onConversationItemClicked(QListWidgetItem *item)
{
    ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(item);
    if (convItem) {
        emit conversationSelected(convItem->conversation().id);
    }
}

void ConversationListWidget::onConversationContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_conversationsList->itemAt(pos);
    if (!item) {
        return;
    }
    
    m_contextMenuItem = item;
    ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(item);
    
    if (!convItem) {
        return;
    }
    
    // Show context menu with rename and delete options
    m_contextMenu->exec(m_conversationsList->mapToGlobal(pos));
}

void ConversationListWidget::onRenameConversation()
{
    if (!m_contextMenuItem) {
        return;
    }
    
    ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(m_contextMenuItem);
    if (convItem) {
        // Inline rename: overlay QLineEdit atop item rect
        if (m_inlineRenameEdit) {
            m_inlineRenameEdit->deleteLater();
            m_inlineRenameEdit = nullptr;
        }
        QRect r = m_conversationsList->visualItemRect(convItem);
        m_inlineRenameEdit = new QLineEdit(m_conversationsList);
        m_inlineRenameEdit->setText(convItem->conversation().title);
        m_inlineRenameEdit->setGeometry(r.adjusted(4,2,-4,-2));
        m_inlineRenameEdit->setFocus();
        m_inlineRenameItem = convItem;
        connect(m_inlineRenameEdit, &QLineEdit::editingFinished, this, &ConversationListWidget::onInlineRenameFinished);
        m_inlineRenameEdit->show();
    }
}

void ConversationListWidget::onDeleteConversation()
{
    if (!m_contextMenuItem) {
        return;
    }
    
    ConversationListItem *convItem = dynamic_cast<ConversationListItem*>(m_contextMenuItem);
    if (!convItem) {
        return;
    }
    
    const Conversation &conv = convItem->conversation();
    
    // Soft delete: mark deleted and offer undo
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    Conversation c = store->getConversation(conv.id);
    c.deleted = true;
    c.updateTimestamp();
    store->updateConversation(c);
    refreshConversations();
    m_pendingSoftDeleteId = c.id;
    emit conversationSoftDeleted(c.id);
    // Show temporary status via status label if available (emit signal for MainWindow to show undo later)
    QTimer::singleShot(5000, this, [store, id=c.id]() mutable {
        // Finalize after undo window (already marked deleted)
        Q_UNUSED(store); Q_UNUSED(id);
    });
}

void ConversationListWidget::onInlineRenameFinished()
{
    if (!m_inlineRenameEdit || !m_inlineRenameItem) return;
    QString newTitle = m_inlineRenameEdit->text().trimmed();
    ConversationListItem *cli = dynamic_cast<ConversationListItem*>(m_inlineRenameItem);
    if (cli && !newTitle.isEmpty() && newTitle != cli->conversation().title) {
        auto *app = Application::instance();
        auto *store = app->conversationStore();
        Conversation c = store->getConversation(cli->conversation().id);
        if (c.isValid()) {
            c.title = newTitle;
            c.updateTimestamp();
            store->updateConversation(c);
            emit conversationRenamed(c.id, newTitle);
        }
    }
    m_inlineRenameEdit->deleteLater();
    m_inlineRenameEdit = nullptr;
    m_inlineRenameItem = nullptr;
}

void ConversationListWidget::populateConversations(const ConversationList &conversations)
{
    m_conversationsList->clear();
    
    for (const Conversation &conv : conversations) {
        auto *item = new ConversationListItem(conv, m_conversationsList);
        m_conversationsList->addItem(item);
    }
}

void ConversationListWidget::showRenameDialog(const QString &conversationId)
{
    auto *app = Application::instance();
    auto *store = app->conversationStore();
    
    Conversation conv = store->getConversation(conversationId);
    if (!conv.isValid()) {
        return;
    }
    
    bool ok;
    QString newTitle = QInputDialog::getText(this, "Rename Conversation",
                                           "Enter new conversation title:",
                                           QLineEdit::Normal,
                                           conv.title,
                                           &ok);
    
    if (ok && !newTitle.trimmed().isEmpty() && newTitle != conv.title) {
        conv.title = newTitle.trimmed();
        // Don't update timestamp for renames - only for new messages
        
        if (store->updateConversation(conv)) {
            emit conversationRenamed(conversationId, newTitle);
        }
    }
}

// ConversationListItem implementation
ConversationListItem::ConversationListItem(const Conversation &conversation, QListWidget *parent)
    : QListWidgetItem(parent)
    , m_conversation(conversation)
{
    updateDisplay();
}

void ConversationListItem::updateFromConversation(const Conversation &conversation)
{
    m_conversation = conversation;
    updateDisplay();
}

void ConversationListItem::updateDisplay()
{
    // Set main text in the base widget
    setText(m_conversation.title);
    
    // Set tooltip with more details
    QString tooltip = QString("Title: %1\nCreated: %2\nUpdated: %3\nProvider: %4")
                      .arg(m_conversation.title)
                      .arg(m_conversation.createdAt.toString("yyyy-MM-dd hh:mm"))
                      .arg(m_conversation.updatedAt.toString("yyyy-MM-dd hh:mm"))
                      .arg(m_conversation.providerId);
    
    setToolTip(tooltip);
    
    // Set item data for easy access
    setData(Qt::UserRole, m_conversation.id);
    
    // Visual indicators
    QFont font = this->font();
    if (m_conversation.pinned) {
        font.setBold(true);
    }
    setFont(font);
    
    // Different styling for archived conversations
    if (m_conversation.archived) {
        QColor textColor = QApplication::palette().color(QPalette::Text);
        textColor.setAlpha(128);
        setForeground(QBrush(textColor));
    }
}

void ConversationListWidget::updateThemeStyles()
{
    auto *app = Application::instance();
    auto *themeManager = app->themeManager();
    const auto &tokens = themeManager->tokens();

    // Update status label
    m_statusLabel->setStyleSheet(QString("color: %1; font-style: italic;")
        .arg(tokens.textMuted.name()));
}

void ConversationListWidget::setupHoverActions()
{
    qDebug() << "Setting up hover actions...";
    auto *app = Application::instance();
    auto *iconRegistry = app->iconRegistry();
    
    // Create hover actions widget
    m_hoverActionsWidget = new QWidget(this);
    m_hoverActionsWidget->setFixedSize(80, 40); // Large enough to see buttons
    m_hoverActionsWidget->hide();
    m_hoverActionsWidget->setStyleSheet("background: rgba(255,255,255,240); border: 1px solid #D1D5DB; border-radius: 6px;"); // Clean white background
    m_hoverActionsWidget->raise(); // Bring to front
    
    QHBoxLayout *actionsLayout = new QHBoxLayout(m_hoverActionsWidget);
    actionsLayout->setContentsMargins(4, 4, 4, 4);
    actionsLayout->setSpacing(4);
    
    // Rename button
    m_hoverRenameButton = new QPushButton();
    m_hoverRenameButton->setIcon(iconRegistry->icon("edit"));
    m_hoverRenameButton->setToolTip("Rename");
    m_hoverRenameButton->setFixedSize(24, 24);
    m_hoverRenameButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #D1D5DB;
            border-radius: 4px;
            background: white;
            padding: 2px;
        }
        QPushButton:hover {
            background: #F9FAFB;
            border-color: #9CA3AF;
        }
    )");
    connect(m_hoverRenameButton, &QPushButton::clicked, this, &ConversationListWidget::onHoverRenameClicked);
    actionsLayout->addWidget(m_hoverRenameButton);
    
    // Delete button
    m_hoverDeleteButton = new QPushButton();
    m_hoverDeleteButton->setIcon(iconRegistry->icon("delete"));
    m_hoverDeleteButton->setToolTip("Delete");
    m_hoverDeleteButton->setFixedSize(24, 24);
    m_hoverDeleteButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #FCA5A5;
            border-radius: 4px;
            background: white;
            padding: 2px;
        }
        QPushButton:hover {
            background: #FEF2F2;
            border-color: #F87171;
        }
    )");
    connect(m_hoverDeleteButton, &QPushButton::clicked, this, &ConversationListWidget::onHoverDeleteClicked);
    actionsLayout->addWidget(m_hoverDeleteButton);
    
    qDebug() << "Hover actions widget created successfully";
}

void ConversationListWidget::showHoverActions(QListWidgetItem *item)
{
    if (!item) {
        hideHoverActions();
        return;
    }
    
    qDebug() << "showHoverActions called for item:" << item->text();
    m_currentHoverItem = item;
    m_hoverActionsWidget->show();
    m_hoverActionsWidget->raise(); // Bring to front
    updateHoverActionsPosition();
    qDebug() << "Hover actions widget positioned at:" << m_hoverActionsWidget->pos() << "size:" << m_hoverActionsWidget->size();
    qDebug() << "Parent widget size:" << this->size();
    qDebug() << "Widget visible:" << m_hoverActionsWidget->isVisible();
    qDebug() << "Widget geometry:" << m_hoverActionsWidget->geometry();
}

void ConversationListWidget::hideHoverActions()
{
    m_hoverActionsWidget->hide();
    m_currentHoverItem = nullptr;
}

void ConversationListWidget::updateHoverActionsPosition()
{
    if (!m_currentHoverItem || !m_hoverActionsWidget->isVisible()) {
        return;
    }
    
    QRect itemRect = m_conversationsList->visualItemRect(m_currentHoverItem);
    QRect listGeometry = m_conversationsList->geometry();
    
    // Position the hover actions on the right side of the item
    int x = listGeometry.x() + itemRect.right() - m_hoverActionsWidget->width() - 8;
    int y = listGeometry.y() + itemRect.y() + (itemRect.height() - m_hoverActionsWidget->height()) / 2;
    
    qDebug() << "Positioning hover actions at x=" << x << "y=" << y;
    qDebug() << "Item rect:" << itemRect << "List geometry:" << listGeometry;
    
    m_hoverActionsWidget->move(x, y);
}

void ConversationListWidget::onConversationItemEntered(QListWidgetItem *item)
{
    showHoverActions(item);
}

void ConversationListWidget::onHoverRenameClicked()
{
    if (m_currentHoverItem) {
        m_contextMenuItem = m_currentHoverItem;
        onRenameConversation();
    }
}

void ConversationListWidget::onHoverDeleteClicked()
{
    if (m_currentHoverItem) {
        m_contextMenuItem = m_currentHoverItem;
        onDeleteConversation();
    }
}

bool ConversationListWidget::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_conversationsList) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QListWidgetItem *item = m_conversationsList->itemAt(mouseEvent->pos());
            
            if (item) {
                // Show hover actions if we're over an item (regardless if it's the same item)
                if (item != m_currentHoverItem) {
                    qDebug() << "Hovering over item:" << item->text();
                    showHoverActions(item);
                }
                // If it's the same item, do nothing (keep showing the actions)
            } else {
                // Hide hover actions if we're not over any item
                if (m_currentHoverItem) {
                    qDebug() << "Mouse left item area";
                    hideHoverActions();
                }
            }
        } else if (event->type() == QEvent::Leave) {
            qDebug() << "Mouse left list widget";
            hideHoverActions();
        }
    }
    
    return QWidget::eventFilter(object, event);
}

} // namespace DesktopApp
