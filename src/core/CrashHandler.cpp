#include "CrashHandler.h"
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

namespace DesktopApp {

QString CrashHandler::s_dumpDir;
QString CrashHandler::s_appName;
QString CrashHandler::s_version;

static bool writeMiniDump(EXCEPTION_POINTERS *pExceptionPointers, const QString &path)
{
    HANDLE hFile = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = pExceptionPointers;
    mdei.ClientPointers = FALSE;
    MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
        MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpNormal);
    BOOL res = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, pExceptionPointers ? &mdei : nullptr, nullptr, nullptr);
    CloseHandle(hFile);
    return res == TRUE;
}

LONG WINAPI CrashHandler::sehHandler(EXCEPTION_POINTERS *info)
{
    QDir().mkpath(s_dumpDir);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString dumpPath = QString("%1/%2_%3_%4.dmp").arg(s_dumpDir, s_appName, s_version, timestamp);
    QString logPath = QString("%1/%2.log").arg(s_dumpDir, s_appName);
    bool ok = writeMiniDump(info, dumpPath);
    QFile f(logPath);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&f);
    ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Crash captured; dump=" << dumpPath << " success=" << ok << " code=0x" << QString::number(info->ExceptionRecord->ExceptionCode,16) << '\n';
    // Initialize symbol handler
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    STACKFRAME64 frame = {};
    CONTEXT *ctx = info->ContextRecord;
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
    DWORD imageType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = ctx->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#else
    DWORD imageType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrStack.Mode = AddrModeFlat;
#endif
    ts << "Stack trace:" << '\n';
    for (int i=0; i<64; ++i) {
        if (!StackWalk64(imageType, process, GetCurrentThread(), &frame, ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) break;
        if (frame.AddrPC.Offset == 0) break;
        DWORD64 addr = frame.AddrPC.Offset;
        char symbolBuffer[sizeof(SYMBOL_INFO)+256];
        SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        DWORD64 displacement = 0;
        if (SymFromAddr(process, addr, &displacement, symbol)) {
        ts << QString("  #%1 0x%2 %3 +0x%4").arg(i).arg(addr,0,16).arg(symbol->Name).arg(displacement,0,16) << '\n';
        } else {
        ts << QString("  #%1 0x%2 <no symbol>").arg(i).arg(addr,0,16) << '\n';
        }
    }
    SymCleanup(process);
    }
    return EXCEPTION_EXECUTE_HANDLER; // allow process to terminate
}

void CrashHandler::install(const QString &dumpDir, const QString &appName, const QString &version)
{
    s_dumpDir = dumpDir;
    s_appName = appName;
    s_version = version;
    SetUnhandledExceptionFilter(sehHandler);
}

} // namespace DesktopApp

#else

namespace DesktopApp {
void CrashHandler::install(const QString &, const QString &, const QString &) {}
} // namespace DesktopApp

#endif
