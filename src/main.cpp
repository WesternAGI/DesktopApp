#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
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
    
    GadAI::MainWindow *mainWindow = nullptr;
    
    if (!skipAuth) {
        // Show login window first (no parent window yet)
        GadAI::LoginWindow loginWindow;
        
        bool authSuccessful = false;
        QString authenticatedUser;
        QString authToken;
        
        // Connect success signal to capture authentication result
        QObject::connect(&loginWindow, &GadAI::LoginWindow::loginSuccessful,
                        [&authSuccessful, &authenticatedUser, &authToken, &loginWindow]
                        (const QString &username, const QString &token) {
            qDebug() << "loginSuccessful signal received for user:" << username;
            authenticatedUser = username;
            authToken = token;
            authSuccessful = true;
            qDebug() << "Scheduling loginWindow.accept() with QTimer::singleShot";
            // Use QTimer::singleShot to defer the accept() call to avoid re-entrancy issues
            QTimer::singleShot(100, &loginWindow, [&loginWindow]() {
                qDebug() << "Calling loginWindow.accept() from timer";
                loginWindow.accept();
                qDebug() << "loginWindow.accept() completed from timer";
            });
        });
        
        // Show login dialog modally and wait for result
        qDebug() << "Starting loginWindow.exec()";
        int loginResult = 0;
        try {
            loginResult = loginWindow.exec();
            qDebug() << "loginWindow.exec() returned successfully with result:" << loginResult;
        } catch (const std::exception& e) {
            qDebug() << "Exception during loginWindow.exec():" << e.what();
            return -1;
        } catch (...) {
            qDebug() << "Unknown exception during loginWindow.exec()";
            return -1;
        }
        
        qDebug() << "Login dialog closed with result:" << loginResult << "(QDialog::Accepted=" << QDialog::Accepted << ")";
        qDebug() << "Auth successful:" << authSuccessful;
        qDebug() << "Authenticated user:" << authenticatedUser;
        
        // Check if authentication was successful (ignore dialog result, rely on our flag)
        if (authSuccessful) {
            qDebug() << "Authentication successful for user:" << authenticatedUser;
            qDebug() << "Creating main window...";
            
            try {
                // Now create and show the main window after successful authentication
                mainWindow = new GadAI::MainWindow();
                qDebug() << "Main window created successfully";
                mainWindow->show();
                qDebug() << "Main window shown successfully";
            } catch (const std::exception& e) {
                qDebug() << "Exception during MainWindow creation/show:" << e.what();
                return -1;
            } catch (...) {
                qDebug() << "Unknown exception during MainWindow creation/show";
                return -1;
            }
        } else {
            qDebug() << "Authentication failed or cancelled. Exiting application.";
            return 0; // Exit if authentication failed
        }
    } else {
        // Skip authentication - create and show main window directly
        qDebug() << "Skipping authentication (--skip-auth flag detected)";
        mainWindow = new GadAI::MainWindow();
        mainWindow->show();
    }
    
    // Ensure we have a main window before proceeding
    if (!mainWindow) {
        qDebug() << "No main window created. Exiting application.";
        return 0;
    }
    
    qDebug() << "main.cpp: Starting application event loop";
    
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        // Clean up main window
        if (mainWindow) {
            delete mainWindow;
            mainWindow = nullptr;
        }
        
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
