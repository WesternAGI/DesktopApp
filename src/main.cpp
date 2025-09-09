#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include "core/Application.h"
#include "core/CrashHandler.h"
#include "ui/MainWindow.h"
#include "ui/LoginWindow.h"
#include "theme/ThemeManager.h"
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("GadAI");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("GadAI Project");
    QApplication::setOrganizationDomain("gadai.local");
    
    // Clear any default styling before our theme system takes over
    app.setStyleSheet("");
    
    // High DPI attributes deprecated in Qt6 (scaling auto-enabled if QT_ENABLE_HIGHDPI_SCALING); removed deprecated calls
    
    // Initialize application core
    GadAI::Application gadApp;
    if (!gadApp.initialize()) {
        return -1;
    }
    // Install crash handler (mini dumps) to app data crashdumps directory
    GadAI::CrashHandler::install(gadApp.appDataDir() + "/crashdumps", QApplication::applicationName(), QApplication::applicationVersion());
    
    // Check if user wants to skip authentication (for demo purposes)
    bool skipAuth = QApplication::arguments().contains("--skip-auth");
    
    // Always create and show main window first
    GadAI::MainWindow *mainWindow = new GadAI::MainWindow();
    mainWindow->show();
    
    if (!skipAuth) {
        // Show login window as a modal dialog
        GadAI::LoginWindow loginWindow(mainWindow);
        
        // Connect success signal to hide the login window
        QObject::connect(&loginWindow, &GadAI::LoginWindow::loginSuccessful,
                        [&loginWindow](const QString &username, const QString &token) {
            Q_UNUSED(username)
            Q_UNUSED(token)
            // Hide the login window instead of calling accept
            loginWindow.hide();
        });
        
        // Show login dialog modally, but ignore the return value since we handle success differently
        loginWindow.exec();
    }
    
    qDebug() << "main.cpp: Starting application event loop";
    
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        QString logPath = GadAI::Application::instance()->appDataDir() + "/gadai.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Normal exit" << Qt::endl;
        }
    });

    // Structured exception handler to log crashes (Windows-specific minimal handler)
    int rc = 0;
    try {
        rc = app.exec();
    } catch (const std::exception &ex) {
        QString logPath = GadAI::Application::instance()->appDataDir() + "/gadai.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Crash (std::exception): " << ex.what() << Qt::endl;
        }
        rc = -2;
    } catch (...) {
        QString logPath = GadAI::Application::instance()->appDataDir() + "/gadai.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Crash (unknown exception)" << Qt::endl;
        }
        rc = -3;
    }
    if (rc != 0) {
        // Distinguish abnormal termination if aboutToQuit not emitted (log may lack Normal exit line)
        QString logPath = GadAI::Application::instance()->appDataDir() + "/gadai.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Abnormal termination code " << rc << Qt::endl;
        }
    }
    return rc;
}
