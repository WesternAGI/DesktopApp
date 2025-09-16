# DesktopApp User Guide

Complete guide to using DesktopApp desktop chat application.

## Getting Started

### First Login

**Demo Account:**
- Username: `demo`
- Password: `demo123`

**Skip Login (Development):**
```bash
./DesktopApp --skip-auth
```

### Main Interface Layout

- **Left Panel**: Conversation list
- **Center**: Chat messages area
- **Bottom**: Message input box and send button

## Basic Usage

### Starting a Conversation
1. Click on existing conversations in the left panel to switch between them
2. Type messages in the bottom input box
3. Press Enter or click the send button
4. Receive echo responses for testing

### Navigation
- Use the conversation list to switch between chats
- Access settings via the menu or keyboard shortcuts
- Toggle themes using the theme button or settings

## Keyboard Shortcuts

- `Ctrl+Q` - Quit application
- `Ctrl+N` - New conversation
- `Ctrl+,` - Open settings (Windows/Linux)
- `Cmd+,` - Open settings (macOS)
- `Enter` - Send message
- `Tab` - Navigate between interface elements
- `Escape` - Close dialogs

## Features

### Themes
- **Light Theme**: Bright, clean appearance
- **Dark Theme**: Dark background for low-light environments
- Access via settings menu or theme toggle button
- Settings are remembered between sessions

### Settings
Currently available in the settings window:
- Theme selection
- Basic preferences
- Application information

## Troubleshooting

### Common Issues

**Application won't start**
- Verify Qt6 is properly installed
- Check that all required libraries are available
- Run from terminal to see error messages

**Login fails**
- Use exact demo credentials: `demo` / `demo123`
- Try development mode with `--skip-auth` flag

**Interface problems**
- Try switching themes in settings
- Resize the window if elements appear broken
- Restart the application

### Debug Information

**Windows (PowerShell):**
```powershell
.\DesktopApp.exe
```

**macOS/Linux:**
```bash
./DesktopApp
```

Check terminal output for error messages and diagnostic information.

### System Requirements
- Qt6 (6.2 or newer)
- Windows 10+, macOS 12+, or Linux with recent desktop environment
- At least 100MB free disk space
- Internet connection (for authentication, unless using --skip-auth)

## Current Limitations

This is a prototype version with the following limitations:
- Echo responses only (no real AI integration)
- Messages don't persist between sessions
- No file attachments or media sharing
- No search functionality
- Limited customization options

## Tips for Developers

- Use `--skip-auth` flag to bypass login during development
- Monitor console output for debugging information
- Settings are stored in standard system locations
- The application uses Qt's standard behavior for most interactions

## Getting Help

If you encounter issues:
1. Check the terminal output for error messages
2. Verify your Qt installation with `qmake --version`
3. Review the [Installation Guide](INSTALLATION.md) for setup help
4. Check [Project Status](PROJECT_STATUS.md) for known limitations

---

**Note**: DesktopApp is currently a working prototype. Many advanced features are planned but not yet implemented.
