#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QThread>
#include "core/Application.h"
#include "core/CrashHandler.h"
#include "ui/MainWindow.h"
#include "ui/LoginWindow.h"
#include "services/AuthenticationService.h"
#include "theme/ThemeManager.h"
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("DesktopApp");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("DesktopApp Project");
    QApplication::setOrganizationDomain("desktopapp.local");
    
    // Clear any default styling before our theme system takes over
    app.setStyleSheet("");
    
    // High DPI attributes deprecated in Qt6 (scaling auto-enabled if QT_ENABLE_HIGHDPI_SCALING); removed deprecated calls
    
    // Initialize application core
    DesktopApp::Application desktopApp;
    if (!desktopApp.initialize()) {
        return -1;
    }
    // Install crash handler (mini dumps) to app data crashdumps directory
    DesktopApp::CrashHandler::install(desktopApp.appDataDir() + "/crashdumps", QApplication::applicationName(), QApplication::applicationVersion());
    
    // Check if user wants to skip authentication (for development purposes)
    bool skipAuth = QApplication::arguments().contains("--skip-auth");
    bool clearAuth = QApplication::arguments().contains("--clear-auth");
    
    // If clear-auth is requested, clear all authentication data and exit
    if (clearAuth) {
        qDebug() << "Clearing all authentication data...";
        DesktopApp::AuthenticationService authService;
        authService.clearCredentials();
        
        // Also clear login preferences
        QSettings loginSettings(QSettings::IniFormat, QSettings::UserScope, "DesktopApp", "ui");
        loginSettings.remove("login/rememberMe");
        loginSettings.remove("login/lastUsername");
        loginSettings.sync();
        
        qDebug() << "Authentication data cleared. Exiting.";
        return 0;
    }
    
    DesktopApp::MainWindow *mainWindow = nullptr;
    
    if (!skipAuth) {
        // Create login window first to initialize authentication service
        DesktopApp::LoginWindow loginWindow;
        
        // Check if user is already authenticated (via Remember Me tokens)
        // The AuthenticationService constructor calls restoreSession() automatically
        bool alreadyAuthenticated = false;
        QString authenticatedUser;
        QString authToken;
        
        // Give the authentication service a moment to restore the session
        QApplication::processEvents();
        
        // Check if we have a valid restored session
        if (loginWindow.getAuthenticationService() && loginWindow.getAuthenticationService()->isAuthenticated()) {
            alreadyAuthenticated = true;
            authenticatedUser = loginWindow.getAuthenticationService()->getCurrentUser().username;
            authToken = loginWindow.getAuthenticationService()->getCurrentToken();
            qDebug() << "User already authenticated via Remember Me:" << authenticatedUser;
        }
        
        bool authSuccessful = alreadyAuthenticated;
        bool dialogClosed = alreadyAuthenticated; // Skip dialog if already authenticated
        
        // Connect success signal to capture authentication result
        QObject::connect(&loginWindow, &DesktopApp::LoginWindow::loginSuccessful,
                        [&authSuccessful, &authenticatedUser, &authToken, &dialogClosed, &loginWindow]
                        (const QString &username, const QString &token) {
            qDebug() << "loginSuccessful signal received for user:" << username;
            authenticatedUser = username;
            authToken = token;
            authSuccessful = true;
            qDebug() << "Closing login window and setting dialogClosed flag";
            loginWindow.close();
            dialogClosed = true;
        });
        
        // Connect rejected signal (user cancels or closes dialog)
        QObject::connect(&loginWindow, &QDialog::rejected, [&dialogClosed]() {
            qDebug() << "Login dialog rejected/cancelled";
            dialogClosed = true;
        });
        
        // Only show login window if not already authenticated
        if (!alreadyAuthenticated) {
            // Show login window (non-modal)
            qDebug() << "Showing login window (non-modal)";
            loginWindow.show();
            
            // Process events until dialog is closed
            qDebug() << "Processing events until dialog closes";
            while (!dialogClosed) {
                QApplication::processEvents();
                QThread::msleep(10); // Small delay to prevent busy loop
            }
        } else {
            qDebug() << "Skipping login window - user already authenticated";
        }
        
        qDebug() << "Dialog closed. Auth successful:" << authSuccessful;
        qDebug() << "Authenticated user:" << authenticatedUser;
        
        // Check if authentication was successful
        if (authSuccessful) {
            qDebug() << "Authentication successful for user:" << authenticatedUser;
            qDebug() << "Creating main window...";
            
            try {
                // Now create and show the main window after successful authentication
                mainWindow = new DesktopApp::MainWindow();
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
        mainWindow = new DesktopApp::MainWindow();
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
        
        QString logPath = DesktopApp::Application::instance()->appDataDir() + "/desktopapp.log";
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
        QString logPath = DesktopApp::Application::instance()->appDataDir() + "/desktopapp.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Crash (std::exception): " << ex.what() << Qt::endl;
        }
        rc = -2;
    } catch (...) {
        QString logPath = DesktopApp::Application::instance()->appDataDir() + "/desktopapp.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Crash (unknown exception)" << Qt::endl;
        }
        rc = -3;
    }
    if (rc != 0) {
        // Distinguish abnormal termination if aboutToQuit not emitted (log may lack Normal exit line)
        QString logPath = DesktopApp::Application::instance()->appDataDir() + "/desktopapp.log";
        QFile f(logPath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << QDateTime::currentDateTime().toString(Qt::ISODate) << " - Abnormal termination code " << rc << Qt::endl;
        }
    }
    return rc;
}
