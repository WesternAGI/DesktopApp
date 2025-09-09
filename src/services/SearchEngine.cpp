#include "SearchEngine.h"
#include "data/JsonStore.h"
#include <QRegularExpression>
#include <QStringList>
#include <QDebug>
#include <QSet>
#include <algorithm>

namespace DesktopApp {

SearchEngine::SearchEngine(JsonStore *conversationStore, QObject *parent)
    : QObject(parent)
    , m_conversationStore(conversationStore)
{
    // Initialize common English stop words
    m_stopWords = {
        "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "by", "from",
        "up", "about", "into", "through", "during", "before", "after", "above", "below", "between",
        "among", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "do", "does",
        "did", "will", "would", "could", "should", "may", "might", "must", "can", "shall",
        "i", "you", "he", "she", "it", "we", "they", "me", "him", "her", "us", "them",
        "my", "your", "his", "her", "its", "our", "their", "this", "that", "these", "those"
    };
    
    qDebug() << "SearchEngine initialized with" << m_stopWords.size() << "stop words";
}

SearchResultList SearchEngine::searchMessages(const QString &query, int limit) const
{
    SearchResultList results;
    
    if (query.trimmed().isEmpty() || !m_conversationStore) {
        return results;
    }
    
    QList<SearchTerm> searchTerms = parseQuery(query);
    if (searchTerms.isEmpty()) {
        return results;
    }
    
    // Get all conversations to search through
    ConversationList conversations = m_conversationStore->getAllConversations();
    
    for (const Conversation &conv : conversations) {
        MessageList messages = m_conversationStore->getMessagesForConversation(conv.id);
        
        for (const Message &message : messages) {
            double relevance = calculateRelevance(message.text, searchTerms);
            
            if (relevance > 0.0) {
                SearchResult result;
                result.messageId = message.id;
                result.conversationId = message.conversationId;
                result.snippet = extractSnippet(message.text, searchTerms);
                result.relevance = relevance;
                result.timestamp = message.createdAt;
                
                results.append(result);
            }
        }
    }
    
    // Sort by relevance (highest first)
    std::sort(results.begin(), results.end(), [](const SearchResult &a, const SearchResult &b) {
        if (std::abs(a.relevance - b.relevance) < 0.001) {
            return a.timestamp > b.timestamp; // More recent first for same relevance
        }
        return a.relevance > b.relevance;
    });
    
    // Limit results
    if (results.size() > limit) {
        results = results.mid(0, limit);
    }
    
    qDebug() << "Search for" << query << "returned" << results.size() << "results";
    return results;
}

ConversationList SearchEngine::searchConversations(const QString &query, int limit) const
{
    ConversationList results;
    
    if (query.trimmed().isEmpty() || !m_conversationStore) {
        return results;
    }
    
    QList<SearchTerm> searchTerms = parseQuery(query);
    if (searchTerms.isEmpty()) {
        return results;
    }
    
    ConversationList allConversations = m_conversationStore->getAllConversations();
    QList<QPair<Conversation, double>> scoredConversations;
    
    for (const Conversation &conv : allConversations) {
        double titleRelevance = calculateRelevance(conv.title, searchTerms) * 2.0; // Weight title higher
        double contentRelevance = 0.0;
        
        // Search through conversation messages
        MessageList messages = m_conversationStore->getMessagesForConversation(conv.id);
        for (const Message &message : messages) {
            contentRelevance += calculateRelevance(message.text, searchTerms);
        }
        
        // Average content relevance
        if (!messages.isEmpty()) {
            contentRelevance /= messages.size();
        }
        
        double totalRelevance = titleRelevance + contentRelevance;
        
        if (totalRelevance > 0.0) {
            scoredConversations.append(qMakePair(conv, totalRelevance));
        }
    }
    
    // Sort by relevance
    std::sort(scoredConversations.begin(), scoredConversations.end(),
              [](const auto &a, const auto &b) {
                  return a.second > b.second;
              });
    
    // Extract conversations and apply limit
    for (int i = 0; i < std::min<int>(limit, static_cast<int>(scoredConversations.size())); ++i) {
        results.append(scoredConversations[i].first);
    }
    
    return results;
}

QStringList SearchEngine::getSearchSuggestions(const QString &partialQuery, int limit) const
{
    QStringList suggestions;
    
    if (partialQuery.length() < 2) {
        return suggestions;
    }
    
    QString normalized = normalizeText(partialQuery);
    QSet<QString> uniqueSuggestions;
    
    // Get all conversations and their messages to build suggestions
    ConversationList conversations = m_conversationStore->getAllConversations();
    
    for (const Conversation &conv : conversations) {
        // Check conversation title
        QStringList titleWords = extractWords(conv.title);
        for (const QString &word : titleWords) {
            if (word.startsWith(normalized, Qt::CaseInsensitive) && word.length() > normalized.length()) {
                uniqueSuggestions.insert(word);
            }
        }
        
        // Check message content (limited to avoid performance issues)
        MessageList recentMessages = m_conversationStore->getMessagesForConversation(conv.id);
        // Limit to recent messages for performance
        if (recentMessages.size() > 10) {
            recentMessages = recentMessages.mid(recentMessages.size() - 10);
        }
        for (const Message &message : recentMessages) {
            QStringList messageWords = extractWords(message.text);
            for (const QString &word : messageWords) {
                if (word.startsWith(normalized, Qt::CaseInsensitive) && word.length() > normalized.length()) {
                    uniqueSuggestions.insert(word);
                    if (uniqueSuggestions.size() >= limit * 2) { // Get extra to have variety
                        break;
                    }
                }
            }
            if (uniqueSuggestions.size() >= limit * 2) {
                break;
            }
        }
    }
    
    suggestions = uniqueSuggestions.values();
    
    // Sort by length (shorter first) and then alphabetically
    std::sort(suggestions.begin(), suggestions.end(), [](const QString &a, const QString &b) {
        if (a.length() != b.length()) {
            return a.length() < b.length();
        }
        return a < b;
    });
    
    // Apply limit
    if (suggestions.size() > limit) {
        suggestions = suggestions.mid(0, limit);
    }
    
    return suggestions;
}

void SearchEngine::indexMessage(const Message &message)
{
    // For now, we do live searching rather than maintaining a separate index
    // In a production system, you might want to maintain an inverted index
    qDebug() << "Message indexed for search:" << message.id;
}

void SearchEngine::removeMessage(const QString &messageId)
{
    // For live searching, no action needed
    qDebug() << "Message removed from search index:" << messageId;
}

bool SearchEngine::rebuildIndex()
{
    // For live searching, no action needed
    qDebug() << "Search index rebuild completed (live search mode)";
    return true;
}

SearchEngine::SearchStats SearchEngine::getSearchStats() const
{
    SearchStats stats;
    stats.totalIndexedMessages = 0;
    stats.totalUniqueWords = 0;
    stats.indexSize = 0;
    
    if (!m_conversationStore) {
        return stats;
    }
    
    ConversationList conversations = m_conversationStore->getAllConversations();
    QSet<QString> uniqueWords;
    
    for (const Conversation &conv : conversations) {
        MessageList messages = m_conversationStore->getMessagesForConversation(conv.id);
        stats.totalIndexedMessages += messages.size();
        
        for (const Message &message : messages) {
            QStringList words = extractWords(message.text);
            for (const QString &word : words) {
                uniqueWords.insert(word);
            }
            stats.indexSize += message.text.length();
        }
    }
    
    stats.totalUniqueWords = uniqueWords.size();
    return stats;
}

QList<SearchEngine::SearchTerm> SearchEngine::parseQuery(const QString &query) const
{
    QList<SearchTerm> terms;
    QString normalized = normalizeText(query);
    
    // Handle quoted phrases
    // Use standard string literal to avoid raw string delimiter issues on MinGW
    QRegularExpression quotedRegex("\"([^\"]+)\"");
    QRegularExpressionMatchIterator quotedMatches = quotedRegex.globalMatch(normalized);
    
    QStringList quotedPhrases;
    while (quotedMatches.hasNext()) {
        QRegularExpressionMatch match = quotedMatches.next();
        QString phrase = match.captured(1);
        quotedPhrases.append(phrase);
        
        SearchTerm term;
        term.word = phrase;
        term.weight = 2.0; // Higher weight for exact phrases
        term.isExact = true;
        terms.append(term);
    }
    
    // Remove quoted phrases from the query
    QString remainingQuery = normalized;
    for (const QString &phrase : quotedPhrases) {
        remainingQuery.remove(QString("\"%1\"").arg(phrase));
    }
    
    // Handle individual words
    QStringList words = extractWords(remainingQuery);
    for (const QString &word : words) {
        if (!isStopWord(word) && word.length() > 1) {
            SearchTerm term;
            term.word = word;
            term.weight = 1.0;
            term.isExact = false;
            terms.append(term);
        }
    }
    
    return terms;
}

double SearchEngine::calculateRelevance(const QString &text, const QList<SearchTerm> &terms) const
{
    if (text.isEmpty() || terms.isEmpty()) {
        return 0.0;
    }
    
    QString normalizedText = normalizeText(text);
    double totalRelevance = 0.0;
    
    for (const SearchTerm &term : terms) {
        double termRelevance = 0.0;
        
        if (term.isExact) {
            // Exact phrase matching
            if (normalizedText.contains(term.word, Qt::CaseInsensitive)) {
                termRelevance = term.weight;
            }
        } else {
            // Word matching with frequency consideration
            int occurrences = normalizedText.count(QRegularExpression(QString(R"(\b%1\b)").arg(QRegularExpression::escape(term.word)), QRegularExpression::CaseInsensitiveOption));
            if (occurrences > 0) {
                // TF (Term Frequency) - logarithmic scaling
                termRelevance = term.weight * (1.0 + std::log(occurrences));
                
                // Boost relevance if word appears early in text
                int firstOccurrence = normalizedText.indexOf(term.word, 0, Qt::CaseInsensitive);
                if (firstOccurrence >= 0) {
                    double positionBoost = 1.0 - (double(firstOccurrence) / normalizedText.length() * 0.5);
                    termRelevance *= positionBoost;
                }
            }
        }
        
        totalRelevance += termRelevance;
    }
    
    // Normalize by text length (shorter texts with matches should score higher)
    if (totalRelevance > 0.0) {
        double lengthNormalization = 100.0 / (100.0 + normalizedText.length());
        totalRelevance *= (1.0 + lengthNormalization);
    }
    
    return totalRelevance;
}

QString SearchEngine::extractSnippet(const QString &text, const QList<SearchTerm> &terms, int maxLength) const
{
    if (text.length() <= maxLength) {
        return text;
    }
    
    QString normalizedText = normalizeText(text);
    int bestStart = 0;
    int bestScore = 0;
    
    // Find the position with the most search term matches
    for (int start = 0; start <= text.length() - maxLength; start += maxLength / 4) {
        QString window = normalizedText.mid(start, maxLength);
        int score = 0;
        
        for (const SearchTerm &term : terms) {
            if (term.isExact) {
                if (window.contains(term.word, Qt::CaseInsensitive)) {
                    score += 10;
                }
            } else {
                score += window.count(QRegularExpression(QString(R"(\b%1\b)").arg(QRegularExpression::escape(term.word)), QRegularExpression::CaseInsensitiveOption));
            }
        }
        
        if (score > bestScore) {
            bestScore = score;
            bestStart = start;
        }
    }
    
    // Extract snippet and clean up
    QString snippet = text.mid(bestStart, maxLength);
    
    // Try to start and end at word boundaries
    if (bestStart > 0) {
        int spacePos = snippet.indexOf(' ');
        if (spacePos > 0 && spacePos < 20) {
            snippet = snippet.mid(spacePos + 1);
        }
        snippet = "..." + snippet;
    }
    
    if (bestStart + maxLength < text.length()) {
        int lastSpace = snippet.lastIndexOf(' ');
        if (lastSpace > snippet.length() - 20) {
            snippet = snippet.left(lastSpace);
        }
        snippet += "...";
    }
    
    return snippet.trimmed();
}

QStringList SearchEngine::extractWords(const QString &text) const
{
    QString normalized = normalizeText(text);
    
    // Split on non-word characters but preserve apostrophes in contractions
    QRegularExpression wordRegex(R"(\b[\w']+\b)");
    QRegularExpressionMatchIterator matches = wordRegex.globalMatch(normalized);
    
    QStringList words;
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString word = match.captured(0).toLower();
        if (word.length() > 1 && !isStopWord(word)) {
            words.append(word);
        }
    }
    
    return words;
}

QString SearchEngine::normalizeText(const QString &text) const
{
    QString normalized = text;
    
    // Remove HTML tags if any
    normalized.remove(QRegularExpression("<[^>]*>"));
    
    // Normalize whitespace
    normalized = normalized.simplified();
    
    return normalized;
}

bool SearchEngine::isStopWord(const QString &word) const
{
    return m_stopWords.contains(word.toLower());
}

} // namespace DesktopApp
