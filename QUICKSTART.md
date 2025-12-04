# Quick Start Guide - OIDC Tester for Ubuntu

## Installation

### 1. Install Dependencies

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential
```

### 2. Build the Application

```bash
cd oidc-tester
mkdir build
cd build
cmake ..
make
```

### 3. Run the Application

```bash
# From the build directory
./oidc-tester

# Or from the project root
./run.sh
```

## Quick Test with a Public OIDC Provider

### Using Google OAuth (Example)

1. **Get Google OAuth Credentials**:
   - Go to [Google Cloud Console](https://console.cloud.google.com/)
   - Create a new project or select existing
   - Enable "Google+ API"
   - Create OAuth 2.0 credentials
   - Add `http://localhost:8080/callback` to authorized redirect URIs

2. **Configure OIDC Tester**:
   - **Issuer URL**: `https://accounts.google.com`
   - **Client ID**: Your Google client ID
   - **Client Secret**: Your Google client secret (if applicable)
   - **Scopes**: `openid profile email`
   - **Response Type**: `code`

3. **Authenticate**:
   - Click "Begin Authentication"
   - Browser will open with Google login
   - Complete authentication
   - Return to OIDC Tester to see tokens

### Using Keycloak (Example)

1. **Configure OIDC Tester**:
   - **Issuer URL**: `https://your-keycloak.com/auth/realms/your-realm`
   - **Client ID**: Your Keycloak client ID
   - **Scopes**: `openid profile email`
   - **Response Type**: `code`

2. **Authenticate**:
   - Click "Begin Authentication"
   - Complete login in browser
   - View tokens in the app

## Troubleshooting

### Port 8080 Already in Use

If port 8080 is already in use, you'll need to:
1. Stop the service using port 8080, or
2. Modify `OIDCManager.h` to use a different port (change `CALLBACK_PORT`)
3. Rebuild the application
4. Update your OIDC provider's redirect URI accordingly

### Browser Doesn't Open

- Ensure you have a default browser configured
- Try running from terminal to see error messages
- Check that `xdg-open` works: `xdg-open https://google.com`

### Tokens Not Appearing

- Check the Logs tab for detailed error messages
- Verify your OIDC provider configuration
- Ensure redirect URI matches exactly: `http://localhost:8080/callback`
- Check that your client ID and secret are correct

### Build Errors

**Qt6 not found**:
```bash
sudo apt install qt6-base-dev qt6-base-dev-tools
```

**CMake version too old**:
```bash
# Install newer CMake from snap
sudo snap install cmake --classic
```

## Features Overview

### Config Tab ⚙️
- Configure all OIDC parameters
- Settings are automatically saved
- Supports custom ACR values and extra parameters

### Authentication Tab 🔑
- Start/cancel authentication
- Real-time progress updates
- Clear error messages

### Tokens Tab 🔐
- View raw tokens
- Automatic JWT decoding
- Pretty-printed JSON
- Copy tokens for external use

### Logs Tab 📄
- Complete activity log
- Timestamped entries
- Error tracking
- Debug information

## Next Steps

- Read the full [README.md](README.md) for detailed documentation
- Check [FEATURE_COMPARISON.md](FEATURE_COMPARISON.md) to see how this compares to the Mac version
- Configure your OIDC provider to use `http://localhost:8080/callback` as redirect URI
- Test your OIDC flows!

## Support

For issues or questions:
- Check the Logs tab for detailed error information
- Review your OIDC provider's documentation
- Ensure all configuration values are correct
- Verify network connectivity to your OIDC provider

