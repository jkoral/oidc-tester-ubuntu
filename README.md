# OIDC Tester for Ubuntu

A professional Qt6-based application for testing OpenID Connect (OIDC) authentication flows and inspecting JWT tokens on Ubuntu Linux.

This is a C++/Qt port of the [Mac-OIDC-Tester](https://github.com/aaronfreimark/Mac-OIDC-Tester) application, providing the exact same functionality and UI design for Ubuntu users.

## Overview

OIDC Tester is a native Qt application designed to help developers and security professionals test OIDC authentication servers, inspect JWT tokens, and debug authentication flows. The app provides a clean, intuitive interface for configuring OIDC parameters and analyzing authentication responses.

## Features

### 🔐 Authentication Testing

- **Complete OIDC Flow Support**: Authorization Code flow with PKCE (S256)
- **Multiple Authentication Methods**: Support for various ACR (Authentication Context Class Reference) values
- **Real-time Authentication**: Uses system browser for secure authentication
- **Custom Parameters**: Support for additional query parameters and login hints

### 🔍 Token Analysis

- **JWT Token Decoding**: Automatic parsing and display of JWT headers, payloads, and signatures
- **Multiple Token Types**: Support for ID tokens, access tokens, and refresh tokens
- **Readable Format**: Pretty-printed JSON with proper formatting
- **Token Validation**: Basic JWT structure validation and parsing

### 📊 Comprehensive Logging

- **Real-time Activity Logs**: Detailed logging of all authentication steps
- **Error Handling**: Clear error messages and debugging information
- **Session Tracking**: Complete audit trail of authentication attempts
- **Export Capabilities**: Easy copying of logs and token data

### 🎨 Professional UI

- **Modern Design**: Clean, card-based interface with gradient backgrounds
- **Tab Organization**: Organized workflow with Config → Authentication → Tokens → Logs
- **Responsive Layout**: Adaptive interface that works on various screen sizes
- **Qt6 Integration**: Native Qt appearance with proper system integration

## System Requirements

- **OS**: Ubuntu 22.04 or later (or any Linux distribution with Qt6)
- **Qt**: Qt 6.2 or later
- **Compiler**: GCC 9+ or Clang 10+ with C++17 support
- **CMake**: 3.16 or later

## Installation

### Prerequisites

Install Qt6 development packages:

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential
```

### Build from Source

```bash
# Clone or navigate to the repository
cd oidc-tester

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make

# Run
./oidc-tester
```

### Install (Optional)

```bash
# From the build directory
sudo make install

# Run from anywhere
oidc-tester
```

## Usage

### 1. Configuration Tab

Set up your OIDC server parameters:

- **Issuer URL**: Your OIDC provider's base URL
- **Client ID**: Your application's client identifier
- **Client Secret**: (Optional) For confidential clients
- **Scopes**: Space-separated list of requested scopes
- **ACR Values**: Authentication Context Class Reference
- **Login Hint**: Pre-fill username/email for testing

### 2. Authentication Tab

1. Click "Begin Authentication" to start the OIDC flow
2. The app will open your system browser for secure authentication
3. Complete login in the browser
4. The app captures the OAuth callback on localhost:8080
5. Return to the app to see results

### 3. Tokens Tab

- View decoded JWT tokens with full header and payload information
- Copy token values for external testing
- Inspect token expiration and claims
- Analyze token structure and validation

### 4. Logs Tab

- Monitor real-time authentication progress
- Review detailed error messages
- Track API calls and responses
- Export logs for debugging

## Configuration Examples

### Keycloak
```
Issuer URL: https://your-keycloak.com/auth/realms/your-realm
Client ID: your-client-id
Scopes: openid profile email
```

### Azure AD
```
Issuer URL: https://login.microsoftonline.com/your-tenant-id/v2.0
Client ID: your-application-id
Scopes: openid profile email
```

### Auth0
```
Issuer URL: https://your-domain.auth0.com
Client ID: your-client-id
Scopes: openid profile email
```

## Security Features

- **PKCE Support**: Automatic PKCE (S256) code generation for secure flows
- **State Parameter**: CSRF protection with state validation
- **Local Callback Server**: Secure OAuth callback handling on localhost
- **Settings Storage**: Configuration stored securely using Qt Settings
- **No Data Collection**: All processing happens locally on your device

## Supported OIDC Features

- ✅ Authorization Code Flow
- ✅ PKCE (Proof Key for Code Exchange) with S256
- ✅ State parameter for CSRF protection
- ✅ Custom redirect URI (localhost:8080)
- ✅ Multiple response types
- ✅ Custom ACR values
- ✅ Login hints and prompts
- ✅ JWT token parsing and validation

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Based on the [Mac-OIDC-Tester](https://github.com/aaronfreimark/Mac-OIDC-Tester) by Aaron Freimark
- Built with Qt6 and C++17
- Thanks to the OIDC community for specifications and standards

