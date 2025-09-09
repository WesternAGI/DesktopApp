# GadAI User Guide

Quick guide to using GadAI desktop chat application.

## Getting Started

### First Login
**Demo Account:**
- Username: `demo`
- Password: `demo123`

**Skip Login (Development):**
```bash
./GadAI --skip-auth
```

## Main Interface

### Layout
- **Left Panel:** Conversation list
- **Center:** Chat messages
- **Bottom:** Message input box

### Basic Usage
1. Click conversations to switch between them
2. Type messages in the bottom input box
3. Press Enter to send (echo responses only)
4. Use menu for settings and themes

### Keyboard Shortcuts
- `Ctrl+Q` - Quit application
- `Ctrl+N` - New conversation
- `Ctrl+,` - Open settings

## Features

### Themes
- Switch between light and dark modes
- Access via menu or theme button

### Settings
- Basic preferences and configuration
- Authentication settings
- Theme selection

## Troubleshooting

### Common Issues
- **App won't start:** Check Qt installation
- **Login fails:** Use exact demo credentials
- **UI issues:** Try different theme or restart

### Getting Help
- Check README.md for build issues
- Verify Qt6 installation and paths
- Review PROJECT_STATUS.md for known limitations

## Current Features

## Settings

### Accessing Settings
- **Menu**: File → Preferences
- **Keyboard**: `Ctrl+,` (Windows/Linux) or `Cmd+,` (macOS)

### Available Settings
Currently the settings window is basic and includes:
- Theme selection (light/dark)
- Basic preferences
- About information

*Note: Most advanced settings are not implemented yet.*

## Themes

### Switching Themes
1. Go to settings or use the theme toggle button
2. Choose between:
   - **Light Theme**: Bright, clean appearance
   - **Dark Theme**: Dark background, easier on eyes
   - **System Theme**: Matches your operating system

### Theme Features
- Icons automatically adjust to theme
- All windows use consistent theming
- Settings are remembered between sessions

## Troubleshooting

### Common Problems

**Problem: App won't start**
- Check that Qt6 is properly installed
- Try running from terminal to see error messages
- Make sure all required DLL files are available (Windows)

**Problem: Login doesn't work**
- Make sure you're using the exact demo account details
- Use `--skip-auth` to bypass login for testing

**Problem: Window appears blank or broken**
- Try resizing the window
- Switch themes in settings

### Getting Debug Information

**On Windows (PowerShell):**
```powershell
.\GadAI.exe
# Check the terminal output for error messages
```

**On macOS/Linux:**
```bash
./GadAI
# Error messages will appear in the terminal
```

### Reporting Issues

If you find bugs or problems:
1. Note your operating system and version
2. Record the exact steps that cause the problem
3. Copy any error messages from the terminal
4. Include your Qt version (`qmake --version`)

## Tips and Tricks

### For General Use
- The app is currently a prototype - many features are placeholders
- Demo accounts are reset each time you restart
- Try both light and dark themes to see which you prefer
- The conversation list will be empty when you first start

### For Developers
- Use `--skip-auth` to bypass login during development
- Check the console output for debugging information
- The app uses Qt's standard keyboard shortcuts
- All settings are stored in your system's standard location

### Keyboard Navigation
- `Tab`: Move between interface elements
- `Enter`: Activate buttons and confirm actions
- `Escape`: Close dialogs and cancel operations
- `Ctrl+Q`: Quit the application

## What's Coming Next

### Planned Features (Not Yet Available)
- **Real AI Chat**: Connect to ChatGPT, Claude, or other AI services
- **Message History**: Save and load previous conversations
- **File Sharing**: Send and receive images and documents
- **Search**: Find messages in your conversation history
- **Voice Messages**: Record and play audio messages
- **Better Settings**: More customization options

### Current Limitations
This is a prototype version, so:
- Only demo authentication works
- Messages don't persist between sessions
- No real AI integration yet
- Limited customization options
- Some UI elements are placeholders

## Support

## What's Coming Next

### Planned Features (Not Yet Available)
- **Real AI Chat**: Connect to ChatGPT, Claude, or other AI services
- **Message History**: Save and load previous conversations
- **File Sharing**: Send and receive images and documents
- **Search**: Find messages in your conversation history
- **Voice Messages**: Record and play audio messages
- **Better Settings**: More customization options

### Current Limitations
This is a prototype version, so:
- Only demo authentication works
- Messages don't persist between sessions
- No real AI integration yet
- Limited customization options
- Some UI elements are placeholders

---

**Remember**: GadAI is currently a prototype. Many features shown in the interface are not fully implemented yet.
