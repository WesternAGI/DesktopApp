# DesktopApp User Guide

Complete guide to using DesktopApp's AI chat interface and provider system.

## Getting Started

### First Login

1. **Launch the Application**: Run DesktopApp from your desktop or terminal
2. **Enter Credentials**: Use your registered username and password
3. **Authentication**: The app connects to the authentication service
4. **Main Interface**: After successful login, the chat interface appears

**Test Account:**
- Username: `testuser1`
- Password: `password123`

### Main Interface Layout

- **Top Bar**: Provider selection dropdown and conversation controls  
- **Left Panel**: Conversation list with search and navigation
- **Center**: Chat message thread with user and AI responses
- **Bottom**: Enhanced message composer with send functionality

## Provider System

### Available Providers

**Echo Provider**
- Built-in testing provider that echoes your messages
- Perfect for testing and demonstration
- No external dependencies required

**Backend AI Provider**  
- Remote AI service integration
- Requires user authentication (uses your login token)
- Provides intelligent AI responses to your messages

### Switching Providers
1. Use the **dropdown in the top bar** next to the conversation list
2. Select "Echo Provider" for testing or "Backend AI" for AI responses
3. Provider change takes effect immediately for new messages

## Basic Usage

### Starting a Conversation
1. **New Chat**: Click the "New Conversation" button or select from recent conversations
2. **Choose Provider**: Select Echo (for testing) or Backend AI (for AI responses) from the top dropdown
3. **Send Messages**: Type in the message composer and press Enter or click Send
4. **View Responses**: Messages appear instantly with clean, modern styling

### Message Features
- **Copy Messages**: Click any message to copy its content to clipboard
- **Message History**: All conversations are automatically saved and persist between sessions
- **Real-time Updates**: New messages appear immediately without page refresh

### Navigation
- **Conversation Switching**: Click any conversation in the left panel to switch
- **Smooth Scrolling**: Conversations auto-scroll to show new messages
- **Clean Interface**: No distracting status notifications or provider indicators

## Keyboard Shortcuts

- `Enter` - Send message in composer
- `Ctrl+N` - New conversation  
- `Ctrl+Q` - Quit application
- `Ctrl+,` - Open settings (Windows/Linux)
- `Cmd+,` - Open settings (macOS)
- `Tab` - Navigate between interface elements
- `Escape` - Close dialogs and popups

## Features

### Modern Interface
- **ChatGPT-Style Design**: Clean, modern appearance with enhanced message bubbles
- **Responsive Layout**: Adapts to different window sizes and screen resolutions
- **Smooth Interactions**: Optimized scrolling and message rendering
- **Minimal Distractions**: Removed status notifications for cleaner experience

### Themes
- **Light Theme**: Bright, clean appearance with modern design tokens
- **Dark Theme**: Dark background optimized for low-light environments  
- **Dynamic Switching**: Instant theme changes without application restart
- **Consistent Styling**: All components update automatically with theme changes

### Settings
Available in the settings window:
- **Theme Selection**: Light/dark theme toggle
- **Provider Configuration**: AI provider setup and management
- **User Preferences**: Personalization options
- **Application Info**: Version and system information

## Troubleshooting

### Common Issues

**Authentication Issues**
- Verify your username and password are correct
- Check internet connection for Backend AI provider
- Try with Echo provider first to test basic functionality

**Provider Problems**
- **Echo not working**: Check provider selection in top dropdown
- **Backend AI errors**: Ensure you're logged in and have valid authentication
- **Connection issues**: Verify internet connection and try switching providers

**Interface Problems**
- Try switching themes if display appears incorrect
- Restart application if provider dropdown becomes unresponsive
- Check window size - interface is optimized for standard desktop resolutions
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
