#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <QString>

namespace GadAI {

class CrashHandler {
public:
    static void install(const QString &dumpDir, const QString &appName, const QString &version);
private:
#ifdef _WIN32
    static LONG WINAPI sehHandler(EXCEPTION_POINTERS *info);
    static QString s_dumpDir;
    static QString s_appName;
    static QString s_version;
#endif
};

} // namespace GadAI
