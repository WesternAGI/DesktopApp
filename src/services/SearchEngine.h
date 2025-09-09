#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include "data/Models.h"

namespace GadAI {

class JsonStore;

/**
 * @brief Full-text search engine for messages
 */
class SearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit SearchEngine(JsonStore *conversationStore, QObject *parent = nullptr);

    /**
     * @brief Search for messages containing the query
     * @param query Search terms
     * @param limit Maximum number of results
     * @return List of search results ordered by relevance
     */
    SearchResultList searchMessages(const QString &query, int limit = 50) const;

    /**
     * @brief Search conversations by title or content
     * @param query Search terms
     * @param limit Maximum number of results
     * @return List of conversations containing the search terms
     */
    ConversationList searchConversations(const QString &query, int limit = 20) const;

    /**
     * @brief Get search suggestions based on partial query
     * @param partialQuery Partial search term
     * @param limit Maximum number of suggestions
     * @return List of suggested search terms
     */
    QStringList getSearchSuggestions(const QString &partialQuery, int limit = 10) const;

    /**
     * @brief Index a new message for searching
     * @param message The message to index
     */
    void indexMessage(const Message &message);

    /**
     * @brief Remove a message from the search index
     * @param messageId The ID of the message to remove
     */
    void removeMessage(const QString &messageId);

    /**
     * @brief Rebuild the entire search index
     */
    bool rebuildIndex();

    /**
     * @brief Get search statistics
     */
    struct SearchStats {
        int totalIndexedMessages;
        int totalUniqueWords;
        qint64 indexSize;
    };
    SearchStats getSearchStats() const;

private:
    struct SearchTerm {
        QString word;
        double weight;
        bool isExact;
    };

    QList<SearchTerm> parseQuery(const QString &query) const;
    double calculateRelevance(const QString &text, const QList<SearchTerm> &terms) const;
    QString extractSnippet(const QString &text, const QList<SearchTerm> &terms, int maxLength = 150) const;
    QStringList extractWords(const QString &text) const;
    QString normalizeText(const QString &text) const;
    bool isStopWord(const QString &word) const;

    JsonStore *m_conversationStore;
    QStringList m_stopWords;
};

} // namespace GadAI
