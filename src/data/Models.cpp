#include "Models.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDebug>

namespace DesktopApp {

// Helper function to generate UUIDs
QString generateId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// UserPreference implementation
UserPreference::UserPreference()
    : id(generateId())
    , theme("light")
    , fontScale(1.0)
    , telemetryOptIn(false)
{
}

QJsonObject UserPreference::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["theme"] = theme;
    obj["fontScale"] = fontScale;
    obj["telemetryOptIn"] = telemetryOptIn;
    obj["keychainIds"] = QJsonArray::fromStringList(keychainIds);
    return obj;
}

UserPreference UserPreference::fromJson(const QJsonObject &json)
{
    UserPreference pref;
    pref.id = json["id"].toString();
    pref.theme = json["theme"].toString("light");
    pref.fontScale = json["fontScale"].toDouble(1.0);
    pref.telemetryOptIn = json["telemetryOptIn"].toBool(false);
    
    QJsonArray keychainArray = json["keychainIds"].toArray();
    for (const auto &value : keychainArray) {
        pref.keychainIds.append(value.toString());
    }
    
    return pref;
}

// Conversation implementation
Conversation::Conversation()
    : id(generateId())
    , title("New Conversation")
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime())
    , pinned(false)
    , archived(false)
    , deleted(false)
    , sortOrder(0)
    , providerId("echo")
    , modelName("echo-model")
{
}

Conversation::Conversation(const QString &title)
    : Conversation()
{
    this->title = title.isEmpty() ? "New Conversation" : title;
}

QJsonObject Conversation::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
    obj["pinned"] = pinned;
    obj["archived"] = archived;
    obj["deleted"] = deleted;
    obj["sortOrder"] = sortOrder;
    obj["providerId"] = providerId;
    obj["modelName"] = modelName;
    obj["metadata"] = metadata;
    return obj;
}

Conversation Conversation::fromJson(const QJsonObject &json)
{
    Conversation conv;
    conv.id = json["id"].toString();
    conv.title = json["title"].toString();
    conv.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    conv.updatedAt = QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate);
    conv.pinned = json["pinned"].toBool();
    conv.archived = json["archived"].toBool();
    conv.deleted = json["deleted"].toBool(false);
    conv.sortOrder = json["sortOrder"].toInt(0);
    conv.providerId = json["providerId"].toString("echo");
    conv.modelName = json["modelName"].toString("echo-model");
    conv.metadata = json["metadata"].toObject();
    return conv;
}

bool Conversation::isValid() const
{
    return !id.isEmpty() && !title.isEmpty() && createdAt.isValid();
}

void Conversation::updateTimestamp()
{
    updatedAt = QDateTime::currentDateTime();
}

// MessageRole functions
QString messageRoleToString(MessageRole role)
{
    switch (role) {
    case MessageRole::User: return "user";
    case MessageRole::Assistant: return "assistant";
    case MessageRole::System: return "system";
    }
    return "user";
}

MessageRole messageRoleFromString(const QString &roleStr)
{
    QString lower = roleStr.toLower();
    if (lower == "assistant") return MessageRole::Assistant;
    if (lower == "system") return MessageRole::System;
    return MessageRole::User;
}

// Message implementation
Message::Message()
    : id(generateId())
    , role(MessageRole::User)
    , createdAt(QDateTime::currentDateTime())
    , isStreaming(false)
{
}

Message::Message(const QString &convId, MessageRole role, const QString &text)
    : Message()
{
    this->conversationId = convId;
    this->role = role;
    this->text = text;
}

QJsonObject Message::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["conversationId"] = conversationId;
    obj["role"] = messageRoleToString(role);
    obj["text"] = text;
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    obj["metadata"] = metadata;
    obj["parentId"] = parentId;
    obj["isStreaming"] = isStreaming;
    return obj;
}

Message Message::fromJson(const QJsonObject &json)
{
    Message msg;
    msg.id = json["id"].toString();
    msg.conversationId = json["conversationId"].toString();
    msg.role = messageRoleFromString(json["role"].toString());
    msg.text = json["text"].toString();
    msg.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    msg.metadata = json["metadata"].toObject();
    msg.parentId = json["parentId"].toString();
    msg.isStreaming = json["isStreaming"].toBool();
    return msg;
}

bool Message::isValid() const
{
    return !id.isEmpty() && !conversationId.isEmpty() && createdAt.isValid();
}

QString Message::roleString() const
{
    return messageRoleToString(role);
}

// AttachmentType functions
QString attachmentTypeToString(AttachmentType type)
{
    switch (type) {
    case AttachmentType::Image: return "image";
    case AttachmentType::PDF: return "pdf";
    case AttachmentType::Text: return "text";
    case AttachmentType::Audio: return "audio";
    case AttachmentType::Other: return "other";
    }
    return "other";
}

AttachmentType attachmentTypeFromString(const QString &typeStr)
{
    QString lower = typeStr.toLower();
    if (lower == "image") return AttachmentType::Image;
    if (lower == "pdf") return AttachmentType::PDF;
    if (lower == "text") return AttachmentType::Text;
    if (lower == "audio") return AttachmentType::Audio;
    return AttachmentType::Other;
}

// Attachment implementation
Attachment::Attachment()
    : id(generateId())
    , type(AttachmentType::Other)
    , fileSize(0)
    , createdAt(QDateTime::currentDateTime())
{
}

Attachment::Attachment(const QString &msgId, const QString &fileName, const QString &filePath)
    : Attachment()
{
    this->messageId = msgId;
    this->fileName = fileName;
    this->filePath = filePath;
    
    // Determine type from file extension
    QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "bmp") {
        type = AttachmentType::Image;
    } else if (ext == "pdf") {
        type = AttachmentType::PDF;
    } else if (ext == "txt" || ext == "md" || ext == "doc" || ext == "docx") {
        type = AttachmentType::Text;
    } else if (ext == "mp3" || ext == "wav" || ext == "m4a" || ext == "ogg") {
        type = AttachmentType::Audio;
    }
    
    // Get file size
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists()) {
        fileSize = fileInfo.size();
        mimeType = "application/octet-stream"; // Default, could be improved
    }
}

QJsonObject Attachment::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["messageId"] = messageId;
    obj["type"] = attachmentTypeToString(type);
    obj["fileName"] = fileName;
    obj["filePath"] = filePath;
    obj["mimeType"] = mimeType;
    obj["fileSize"] = fileSize;
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    obj["metadata"] = metadata;
    return obj;
}

Attachment Attachment::fromJson(const QJsonObject &json)
{
    Attachment att;
    att.id = json["id"].toString();
    att.messageId = json["messageId"].toString();
    att.type = attachmentTypeFromString(json["type"].toString());
    att.fileName = json["fileName"].toString();
    att.filePath = json["filePath"].toString();
    att.mimeType = json["mimeType"].toString();
    att.fileSize = json["fileSize"].toVariant().toLongLong();
    att.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    att.metadata = json["metadata"].toObject();
    return att;
}

bool Attachment::isValid() const
{
    return !id.isEmpty() && !messageId.isEmpty() && !fileName.isEmpty();
}

QString Attachment::typeString() const
{
    return attachmentTypeToString(type);
}

bool Attachment::exists() const
{
    return QFileInfo::exists(filePath);
}

// ProviderAccount implementation
ProviderAccount::ProviderAccount()
    : id(generateId())
    , provider("echo")
    , label("Echo Provider")
    , enabled(true)
    , createdAt(QDateTime::currentDateTime())
{
}

ProviderAccount::ProviderAccount(const QString &provider, const QString &label)
    : ProviderAccount()
{
    this->provider = provider;
    this->label = label;
}

QJsonObject ProviderAccount::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["provider"] = provider;
    obj["label"] = label;
    obj["endpoint"] = endpoint;
    obj["apiKeyRef"] = apiKeyRef;
    obj["defaultModel"] = defaultModel;
    obj["parameters"] = parameters;
    obj["enabled"] = enabled;
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    return obj;
}

ProviderAccount ProviderAccount::fromJson(const QJsonObject &json)
{
    ProviderAccount account;
    account.id = json["id"].toString();
    account.provider = json["provider"].toString();
    account.label = json["label"].toString();
    account.endpoint = json["endpoint"].toString();
    account.apiKeyRef = json["apiKeyRef"].toString();
    account.defaultModel = json["defaultModel"].toString();
    account.parameters = json["parameters"].toObject();
    account.enabled = json["enabled"].toBool(true);
    account.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    return account;
}

bool ProviderAccount::isValid() const
{
    return !id.isEmpty() && !provider.isEmpty() && !label.isEmpty();
}

// Prompt implementation
Prompt::Prompt()
    : id(generateId())
    , name("New Prompt")
    , category("General")
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime())
{
}

Prompt::Prompt(const QString &name, const QString &text)
    : Prompt()
{
    this->name = name;
    this->text = text;
}

QJsonObject Prompt::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    obj["text"] = text;
    obj["variables"] = variables;
    obj["category"] = category;
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    obj["updatedAt"] = updatedAt.toString(Qt::ISODate);
    return obj;
}

Prompt Prompt::fromJson(const QJsonObject &json)
{
    Prompt prompt;
    prompt.id = json["id"].toString();
    prompt.name = json["name"].toString();
    prompt.text = json["text"].toString();
    prompt.variables = json["variables"].toObject();
    prompt.category = json["category"].toString("General");
    prompt.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    prompt.updatedAt = QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate);
    return prompt;
}

bool Prompt::isValid() const
{
    return !id.isEmpty() && !name.isEmpty();
}

QString Prompt::processTemplate(const QJsonObject &values) const
{
    QString result = text;
    
    // Simple template processing: replace {{variable}} with values
    for (auto it = values.begin(); it != values.end(); ++it) {
        QString placeholder = QString("{{%1}}").arg(it.key());
        result.replace(placeholder, it.value().toString());
    }
    
    return result;
}

// SearchResult implementation
SearchResult::SearchResult()
    : relevance(0.0)
{
}

SearchResult::SearchResult(const QString &msgId, const QString &convId, 
                          const QString &snippet, double relevance)
    : messageId(msgId)
    , conversationId(convId)
    , snippet(snippet)
    , relevance(relevance)
{
}

bool SearchResult::isValid() const
{
    return !messageId.isEmpty() && !conversationId.isEmpty();
}

} // namespace DesktopApp
