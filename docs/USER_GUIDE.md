# GadAI User Guide

A simple guide to using the GadAI desktop chat application.

## Getting Started

### First Time Setup

1. **Download and Install**
   - Follow the installation instructions in the main README.md
   - Make sure Qt6 is installed on your system
   - Build the application using CMake

2. **Launch the Application**
   - Run `GadAI.exe` on Windows or `./GadAI` on macOS/Linux
   - The login window will appear first

### Logging In

**Demo Accounts Available:**
- **Account 1**: Phone `+15550000001`, Password `demo123`
- **Account 2**: Phone `+15550000002`, Password `admin123`

**Steps to Login:**
1. Enter one of the demo phone numbers
2. Enter the corresponding password
3. Click "Login"
4. Wait for the main window to appear

**Skip Login (For Developers):**
```bash
./GadAI --skip-auth
```

## Main Interface

### Window Layout

The main window has three main areas:

1. **Left Sidebar**: Conversation list
2. **Center Area**: Chat messages and conversation view
3. **Bottom Area**: Message input box and send controls

### Basic Navigation

- **Switch Conversations**: Click on conversations in the left sidebar
- **Scroll Messages**: Use mouse wheel or scrollbar in the center area
- **Type Messages**: Click in the text box at the bottom

## Current Features

### What Works Now

✅ **Login System**
- Demo authentication with phone and password
- Session remembering (stays logged in)
- Logout functionality

✅ **Main Chat Interface**
- Clean, modern chat layout
- Conversation list in sidebar
- Message area ready for chat
- Text input field for typing

✅ **Basic Controls**
- Menu system with File, Edit, View, Help menus
- Settings window (basic version)
- About dialog with app information
- Theme switching (light/dark modes)

✅ **Keyboard Shortcuts**
- `Ctrl+Q` or `Cmd+Q`: Quit application
- `Ctrl+,` or `Cmd+,`: Open settings
- `F11`: Toggle fullscreen (on some platforms)

### What Doesn't Work Yet

❌ **Real Chat Features**
- Messages don't actually send anywhere
- No AI responses yet
- No conversation history saved
- No file attachments

❌ **Advanced Features**
- No search functionality
- No voice messages
- No real network communication
- No data encryption

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
- Try the other demo account
- Use `--skip-auth` to bypass login for testing

**Problem: Window appears blank or broken**
- Try resizing the window
- Check if dark theme is making text invisible
- Switch themes in settings

**Problem: App crashes after login**
- This is a known issue in the current version
- Try using `--skip-auth` mode
- Check the terminal output for error messages

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

### Getting Help
- Check this user guide first
- Read the main README.md for installation help
- Look at the PROJECT_STATUS.md for current feature status
- Try the troubleshooting steps above

### Community
This is currently a single-developer prototype. Community features and support channels will be added as the project grows.

---

**Remember**: GadAI is currently a prototype. Many features shown in the interface are not fully implemented yet. The goal is to get the basic framework working first, then add more features over time.
* Tests / CI

## 5. Dev Tips
| Task | Hint |
|------|------|
| Reset state | Delete app data directory (see README) |
| Add demo users | Modify `AuthenticationService::initializeDemoUsers()` |
| Skip login | Run with `--skip-auth` |

## 6. Planned Message Flow
```
Composer -> Persist -> Provider send -> Stream back -> Append UI
```
Currently ends before provider send.

## 7. Audio Recording
Create `AudioRecorder`, call `startRecording("out.wav")`, later `stopRecording()`. Produces 16‑bit mono WAV.

## 8. Troubleshooting
| Problem | Fix |
|---------|-----|
| Login loops | Use `--skip-auth` to continue development |
| Qt libs missing | Add Qt bin to PATH / LD_LIBRARY_PATH |
| Build fails after edits | Delete `build/` and reconfigure |
| UI very large/small | Adjust system scaling DPI or theme | 

## 9. Roadmap Order (Suggested)
1. Wire composer to provider manager (echo response stub).
2. Persist conversations properly.
3. Introduce a small test harness.
4. Add provider abstraction for real API.
5. Implement attachment handling + file vault.
6. Add search indexing.
7. Harden authentication / storage.

## 10. Disclaimer
Prototype, not production. Remove assumptions and add validation before shipping.

---
See `ARCHITECTURE.md` and `SECURITY.md` for more internal details.
