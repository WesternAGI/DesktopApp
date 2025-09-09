// SimpleMarkdown.h - lightweight markdown to HTML converter (subset)
#pragma once
#include <QString>

namespace DesktopApp {
struct MarkdownOptions {
    bool enableTables = true;
    bool enableMath = true; // Placeholder (KaTeX not yet integrated)
};

QString renderMarkdownToHtml(const QString &markdown, const MarkdownOptions &opts = {});
}
