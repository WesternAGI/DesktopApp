#include "SimpleMarkdown.h"
#include <QRegularExpression>

namespace GadAI {

static QString escapeHtml(const QString &in) {
    QString s = in;
    s.replace('&', "&amp;");
    s.replace('<', "&lt;");
    s.replace('>', "&gt;");
    return s;
}

QString renderMarkdownToHtml(const QString &markdown, const MarkdownOptions &opts) {
    QString html;
    QStringList lines = markdown.split('\n');
    bool inCode = false; QString codeLang; QStringList codeLines;
    bool inTable = false; QStringList headerCells; QStringList tableRows;

    auto flushCode = [&]() {
        if (!inCode) return;
        QString code = escapeHtml(codeLines.join("\n"));
        html += QString("<div class='code-block'><div class='code-lang'>%1</div><pre><code class='lang-%2'>%3</code></pre></div>")
            .arg(codeLang.isEmpty()?"code":codeLang, codeLang, code);
        inCode = false; codeLang.clear(); codeLines.clear();
    };

    auto flushTable = [&]() {
        if (!inTable) return;
        html += "<table class='md-table'><thead><tr>";
        for (const auto &h : headerCells) html += "<th>" + escapeHtml(h.trimmed()) + "</th>";
        html += "</tr></thead><tbody>";
        for (const auto &row : tableRows) {
            html += "<tr>";
            for (const auto &c : row.split('|')) html += "<td>" + escapeHtml(c.trimmed()) + "</td>";
            html += "</tr>";
        }
        html += "</tbody></table>";
        inTable = false; headerCells.clear(); tableRows.clear();
    };

    QRegularExpression headingRe("^(#{1,6})\\s+(.*)$");
    QRegularExpression fencedRe("^```(.*)$");
    QRegularExpression tableSepRe("^\\n?\\s*\\|?(\\s*:?[-]+:?\\s*\\|)+\\s*$");

    for (int i=0;i<lines.size();++i) {
        QString line = lines[i];
        auto fencedMatch = fencedRe.match(line);
        if (fencedMatch.hasMatch()) {
            if (inCode) { flushCode(); continue; }
            flushTable();
            inCode = true; codeLang = fencedMatch.captured(1).trimmed();
            continue;
        }
        if (inCode) { codeLines << line; continue; }
        // Table detection
        if (opts.enableTables && line.contains('|')) {
            // If we are not yet in table, next line might be separator
            if (!inTable) {
                // Look ahead for separator
                if (i+1 < lines.size() && tableSepRe.match(lines[i+1]).hasMatch()) {
                    inTable = true;
                    headerCells = line.split('|');
                    ++i; // Skip separator
                    continue;
                }
            } else {
                if (line.trimmed().isEmpty()) { flushTable(); continue; }
                tableRows << line;
                continue;
            }
        } else if (inTable) {
            flushTable();
        }
        auto hMatch = headingRe.match(line);
        if (hMatch.hasMatch()) {
            flushCode(); flushTable();
            int level = hMatch.captured(1).length();
            QString text = hMatch.captured(2).trimmed();
            html += QString("<h%1>%2</h%1>").arg(level).arg(escapeHtml(text));
            continue;
        }
        if (line.trimmed().isEmpty()) { html += "<p></p>"; continue; }
        QString p = escapeHtml(line);
        // Bold/italic
        p.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
        p.replace(QRegularExpression("\\*(.+?)\\*"), "<em>\\1</em>");
        // Inline code
        p.replace(QRegularExpression("`([^`]+)`"), "<code class='inline'>\\1</code>");
        if (opts.enableMath) {
            // Math inline $...$ placeholder (no KaTeX yet)
            p.replace(QRegularExpression("\\$([^$]+)\\$"), "<span class='math'>\\1</span>");
        }
        html += "<p>" + p + "</p>";
    }
    flushCode(); flushTable();
    return html;
}

} // namespace GadAI
