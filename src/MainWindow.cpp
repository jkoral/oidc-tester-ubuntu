#include "MainWindow.h"
#include "JWTDecoder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSettings>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(new QTabWidget(this))
    , m_oidcManager(new OIDCManager(this))
    , m_isAuthenticating(false)
{
    setupUI();
    loadSettings();

    // Connect OIDC manager signals
    connect(m_oidcManager, &OIDCManager::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_oidcManager, &OIDCManager::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_oidcManager, &OIDCManager::tokensReceived, this, &MainWindow::onTokensReceived);
    connect(m_oidcManager, &OIDCManager::logMessage, this, &MainWindow::onLogMessage);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    setWindowTitle("OIDC Tester");
    setMinimumSize(900, 700);
    resize(1000, 800);
    
    // Set background gradient (approximation using stylesheet)
    setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #FAFAFF, stop:1 #F2F3FA); }"
        "QGroupBox { background-color: rgba(255, 255, 255, 200); "
        "border-radius: 12px; padding: 20px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    
    createConfigTab();
    createAuthenticationTab();
    createTokensTab();
    createLogsTab();
    
    setCentralWidget(m_tabWidget);
}

void MainWindow::createConfigTab()
{
    QWidget* configTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(configTab);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(20);
    
    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel("⚙️");
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);
    
    QLabel* titleLabel = new QLabel("Configuration");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    
    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: rgba(0, 122, 255, 0.3);");
    mainLayout->addWidget(divider);
    
    // Scroll area for config fields
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(24);
    
    // OIDC Configuration Group
    QGroupBox* oidcGroup = new QGroupBox("OIDC Configuration");
    QFont groupFont = oidcGroup->font();
    groupFont.setBold(true);
    oidcGroup->setFont(groupFont);
    oidcGroup->setStyleSheet(oidcGroup->styleSheet() + "QGroupBox { color: #007AFF; }");
    
    QFormLayout* oidcLayout = new QFormLayout();
    oidcLayout->setSpacing(16);
    
    m_issuerURLEdit = new QLineEdit();
    m_issuerURLEdit->setPlaceholderText("https://your-oidc-provider.com");
    oidcLayout->addRow("Issuer URL:", m_issuerURLEdit);
    
    m_clientIDEdit = new QLineEdit();
    m_clientIDEdit->setPlaceholderText("Your OIDC client identifier");
    oidcLayout->addRow("Client ID:", m_clientIDEdit);
    
    m_clientSecretEdit = new QLineEdit();
    m_clientSecretEdit->setEchoMode(QLineEdit::Password);
    m_clientSecretEdit->setPlaceholderText("Enter client secret if required");
    oidcLayout->addRow("Client Secret (Optional):", m_clientSecretEdit);
    
    m_acrValueCombo = new QComboBox();
    m_acrValueCombo->addItems({"None", "SSO (com:imprivata:oidc:epic:sso)", "EPCS (com:imprivata:oidc:epic:cw:epcs)"});
    oidcLayout->addRow("ACR Value:", m_acrValueCombo);
    
    m_loginHintEdit = new QLineEdit();
    m_loginHintEdit->setPlaceholderText("Username or email hint");
    oidcLayout->addRow("Login Hint (Optional):", m_loginHintEdit);
    
    m_promptLoginCheck = new QCheckBox("Prompt for login");
    oidcLayout->addRow("Authentication Prompt:", m_promptLoginCheck);
    
    m_redirectURILabel = new QLabel("http://localhost:8080/callback");
    m_redirectURILabel->setStyleSheet("QLabel { background-color: rgba(128, 128, 128, 0.1); "
                                     "border: 1px solid rgba(128, 128, 128, 0.3); "
                                     "border-radius: 6px; padding: 6px 10px; }");
    m_redirectURILabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    oidcLayout->addRow("Redirect URI:", m_redirectURILabel);
    
    m_scopesEdit = new QLineEdit();
    m_scopesEdit->setPlaceholderText("openid profile email");
    m_scopesEdit->setText("openid profile email");
    oidcLayout->addRow("Scopes:", m_scopesEdit);

    m_responseTypeEdit = new QLineEdit();
    m_responseTypeEdit->setPlaceholderText("code");
    m_responseTypeEdit->setText("code");
    oidcLayout->addRow("Response Type:", m_responseTypeEdit);

    m_extraParamsEdit = new QLineEdit();
    m_extraParamsEdit->setPlaceholderText("key1=value1&key2=value2");
    oidcLayout->addRow("Extra Parameters (Optional):", m_extraParamsEdit);

    oidcGroup->setLayout(oidcLayout);
    scrollLayout->addWidget(oidcGroup);

    // Action Button Group
    QGroupBox* actionGroup = new QGroupBox();
    actionGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 150); }");
    QVBoxLayout* actionLayout = new QVBoxLayout();
    actionLayout->setSpacing(16);

    m_beginAuthButton = new QPushButton("Begin Authentication");
    m_beginAuthButton->setMinimumHeight(40);
    m_beginAuthButton->setStyleSheet("QPushButton { background-color: #007AFF; color: white; "
                                    "border-radius: 8px; font-size: 14px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #0051D5; }"
                                    "QPushButton:disabled { background-color: #CCCCCC; }");
    connect(m_beginAuthButton, &QPushButton::clicked, this, &MainWindow::onBeginAuthentication);
    actionLayout->addWidget(m_beginAuthButton);

    m_configErrorLabel = new QLabel();
    m_configErrorLabel->setStyleSheet("QLabel { color: red; }");
    m_configErrorLabel->setWordWrap(true);
    m_configErrorLabel->hide();
    actionLayout->addWidget(m_configErrorLabel);

    actionGroup->setLayout(actionLayout);
    scrollLayout->addWidget(actionGroup);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    m_tabWidget->addTab(configTab, "⚙️ Config");
}

void MainWindow::createAuthenticationTab()
{
    QWidget* authTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(authTab);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel("🔑");
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);

    QLabel* titleLabel = new QLabel("Authentication");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: rgba(255, 149, 0, 0.3);");
    mainLayout->addWidget(divider);

    // Idle state widget
    m_authIdleWidget = new QWidget();
    QVBoxLayout* idleLayout = new QVBoxLayout(m_authIdleWidget);
    idleLayout->setSpacing(20);

    QGroupBox* idleGroup = new QGroupBox();
    idleGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 150); }");
    QVBoxLayout* idleGroupLayout = new QVBoxLayout();
    idleGroupLayout->setSpacing(20);

    QHBoxLayout* idleHeaderLayout = new QHBoxLayout();
    QLabel* idleIcon = new QLabel("🔐");
    QFont idleIconFont = idleIcon->font();
    idleIconFont.setPointSize(32);
    idleIcon->setFont(idleIconFont);

    QVBoxLayout* idleTextLayout = new QVBoxLayout();
    QLabel* idleTitle = new QLabel("Ready to Authenticate");
    QFont idleTitleFont = idleTitle->font();
    idleTitleFont.setPointSize(16);
    idleTitle->setFont(idleTitleFont);
    idleTitle->setStyleSheet("color: #888888;");

    QLabel* idleDesc = new QLabel("Configure your OIDC settings and begin authentication");
    idleDesc->setStyleSheet("color: #AAAAAA;");

    idleTextLayout->addWidget(idleTitle);
    idleTextLayout->addWidget(idleDesc);

    idleHeaderLayout->addWidget(idleIcon);
    idleHeaderLayout->addLayout(idleTextLayout);
    idleHeaderLayout->addStretch();

    idleGroupLayout->addLayout(idleHeaderLayout);

    m_authBeginButton = new QPushButton("Begin Authentication");
    m_authBeginButton->setMinimumHeight(40);
    m_authBeginButton->setStyleSheet("QPushButton { background-color: #FF9500; color: white; "
                                    "border-radius: 8px; font-size: 14px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #CC7700; }"
                                    "QPushButton:disabled { background-color: #CCCCCC; }");
    connect(m_authBeginButton, &QPushButton::clicked, this, &MainWindow::onBeginAuthentication);
    idleGroupLayout->addWidget(m_authBeginButton);

    m_authErrorLabel = new QLabel();
    m_authErrorLabel->setStyleSheet("QLabel { color: red; background-color: rgba(255, 0, 0, 0.1); "
                                   "border-radius: 8px; padding: 10px; }");
    m_authErrorLabel->setWordWrap(true);
    m_authErrorLabel->hide();
    idleGroupLayout->addWidget(m_authErrorLabel);

    idleGroup->setLayout(idleGroupLayout);
    idleLayout->addWidget(idleGroup);
    idleLayout->addStretch();

    // Active authentication widget
    m_authActiveWidget = new QWidget();
    QVBoxLayout* activeLayout = new QVBoxLayout(m_authActiveWidget);
    activeLayout->setSpacing(24);

    QGroupBox* activeGroup = new QGroupBox();
    activeGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 200); }");
    QVBoxLayout* activeGroupLayout = new QVBoxLayout();
    activeGroupLayout->setSpacing(24);

    QHBoxLayout* activeHeaderLayout = new QHBoxLayout();
    QLabel* activeIcon = new QLabel("🛡️");
    QFont activeIconFont = activeIcon->font();
    activeIconFont.setPointSize(32);
    activeIcon->setFont(activeIconFont);

    m_authStatusLabel = new QLabel("OIDC Authentication in Progress");
    QFont statusFont = m_authStatusLabel->font();
    statusFont.setPointSize(16);
    statusFont.setBold(true);
    m_authStatusLabel->setFont(statusFont);

    activeHeaderLayout->addWidget(activeIcon);
    activeHeaderLayout->addWidget(m_authStatusLabel);
    activeHeaderLayout->addStretch();
    activeGroupLayout->addLayout(activeHeaderLayout);

    m_authProgressLabel = new QLabel("Ready to authenticate");
    QFont progressFont = m_authProgressLabel->font();
    progressFont.setPointSize(14);
    m_authProgressLabel->setFont(progressFont);
    m_authProgressLabel->setAlignment(Qt::AlignCenter);
    m_authProgressLabel->setWordWrap(true);
    m_authProgressLabel->setStyleSheet("QLabel { background-color: rgba(255, 149, 0, 0.05); "
                                      "border-radius: 12px; padding: 20px; }");
    activeGroupLayout->addWidget(m_authProgressLabel);

    QLabel* infoLabel = new QLabel("Complete authentication in the browser window that opens.\n"
                                  "This application will capture the OAuth callback on localhost:8080.");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #888888;");
    activeGroupLayout->addWidget(infoLabel);

    m_cancelAuthButton = new QPushButton("Cancel Authentication");
    m_cancelAuthButton->setMinimumHeight(40);
    m_cancelAuthButton->setStyleSheet("QPushButton { background-color: #FF3B30; color: white; "
                                     "border-radius: 8px; font-size: 14px; font-weight: bold; }"
                                     "QPushButton:hover { background-color: #CC2E26; }");
    connect(m_cancelAuthButton, &QPushButton::clicked, this, &MainWindow::onCancelAuthentication);
    activeGroupLayout->addWidget(m_cancelAuthButton);

    activeGroup->setLayout(activeGroupLayout);
    activeLayout->addWidget(activeGroup);
    activeLayout->addStretch();

    m_authActiveWidget->hide();

    // Add both widgets to main layout
    mainLayout->addWidget(m_authIdleWidget);
    mainLayout->addWidget(m_authActiveWidget);

    // Info card
    QGroupBox* infoGroup = new QGroupBox("Authentication Context");
    infoGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 149, 0, 0.05); color: #FF9500; }");
    QVBoxLayout* infoLayout = new QVBoxLayout();
    QLabel* infoText = new QLabel("This tab manages the OIDC authentication flow. When authentication begins, "
                                 "your system browser will open with your OIDC provider, and this application "
                                 "will listen for the OAuth callback on localhost:8080.");
    infoText->setWordWrap(true);
    infoText->setStyleSheet("color: #888888;");
    infoLayout->addWidget(infoText);
    infoGroup->setLayout(infoLayout);
    mainLayout->addWidget(infoGroup);

    m_tabWidget->addTab(authTab, "🔑 Authentication");
}

void MainWindow::createTokensTab()
{
    QWidget* tokensTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(tokensTab);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel("🔐");
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);

    QLabel* titleLabel = new QLabel("Tokens");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: rgba(52, 199, 89, 0.3);");
    mainLayout->addWidget(divider);

    // Empty state widget
    m_tokensEmptyWidget = new QWidget();
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_tokensEmptyWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QGroupBox* emptyGroup = new QGroupBox();
    emptyGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 100); }");
    QVBoxLayout* emptyGroupLayout = new QVBoxLayout();
    emptyGroupLayout->setAlignment(Qt::AlignCenter);
    emptyGroupLayout->setSpacing(16);

    QLabel* emptyIcon = new QLabel("🔓");
    QFont emptyIconFont = emptyIcon->font();
    emptyIconFont.setPointSize(48);
    emptyIcon->setFont(emptyIconFont);
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyIcon->setStyleSheet("color: rgba(128, 128, 128, 0.5);");

    QLabel* emptyTitle = new QLabel("No tokens yet");
    QFont emptyTitleFont = emptyTitle->font();
    emptyTitleFont.setPointSize(16);
    emptyTitle->setFont(emptyTitleFont);
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyTitle->setStyleSheet("color: #888888;");

    QLabel* emptyDesc = new QLabel("Complete authentication to see tokens here");
    emptyDesc->setAlignment(Qt::AlignCenter);
    emptyDesc->setStyleSheet("color: #AAAAAA;");

    emptyGroupLayout->addWidget(emptyIcon);
    emptyGroupLayout->addWidget(emptyTitle);
    emptyGroupLayout->addWidget(emptyDesc);

    emptyGroup->setLayout(emptyGroupLayout);
    emptyLayout->addWidget(emptyGroup);

    // Content widget
    m_tokensContentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(m_tokensContentWidget);
    contentLayout->setSpacing(20);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(20);

    // Raw tokens group
    QGroupBox* rawGroup = new QGroupBox("Raw Tokens");
    rawGroup->setStyleSheet("QGroupBox { color: #34C759; }");
    QVBoxLayout* rawLayout = new QVBoxLayout();

    m_rawTokensText = new QTextEdit();
    m_rawTokensText->setReadOnly(true);
    m_rawTokensText->setFont(QFont("Monospace", 10));
    m_rawTokensText->setStyleSheet("QTextEdit { background-color: #F5F5F5; border-radius: 8px; padding: 16px; }");
    rawLayout->addWidget(m_rawTokensText);

    rawGroup->setLayout(rawLayout);
    scrollLayout->addWidget(rawGroup);

    // Decoded tokens group
    QGroupBox* decodedGroup = new QGroupBox("Decoded Token Details");
    decodedGroup->setStyleSheet("QGroupBox { color: #34C759; }");
    QVBoxLayout* decodedLayout = new QVBoxLayout();

    m_decodedTokensText = new QTextEdit();
    m_decodedTokensText->setReadOnly(true);
    m_decodedTokensText->setFont(QFont("Monospace", 10));
    m_decodedTokensText->setStyleSheet("QTextEdit { background-color: #F5F5F5; border-radius: 8px; padding: 16px; }");
    decodedLayout->addWidget(m_decodedTokensText);

    decodedGroup->setLayout(decodedLayout);
    scrollLayout->addWidget(decodedGroup);

    scrollArea->setWidget(scrollWidget);
    contentLayout->addWidget(scrollArea);

    m_tokensContentWidget->hide();

    mainLayout->addWidget(m_tokensEmptyWidget);
    mainLayout->addWidget(m_tokensContentWidget);

    m_tabWidget->addTab(tokensTab, "🔐 Tokens");
}

void MainWindow::createLogsTab()
{
    QWidget* logsTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(logsTab);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel("📄");
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);

    QLabel* titleLabel = new QLabel("Logs");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: rgba(175, 82, 222, 0.3);");
    mainLayout->addWidget(divider);

    // Empty state widget
    m_logsEmptyWidget = new QWidget();
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_logsEmptyWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QGroupBox* emptyGroup = new QGroupBox();
    emptyGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 100); }");
    QVBoxLayout* emptyGroupLayout = new QVBoxLayout();
    emptyGroupLayout->setAlignment(Qt::AlignCenter);
    emptyGroupLayout->setSpacing(16);

    QLabel* emptyIcon = new QLabel("📋");
    QFont emptyIconFont = emptyIcon->font();
    emptyIconFont.setPointSize(48);
    emptyIcon->setFont(emptyIconFont);
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyIcon->setStyleSheet("color: rgba(128, 128, 128, 0.5);");

    QLabel* emptyTitle = new QLabel("No activity yet");
    QFont emptyTitleFont = emptyTitle->font();
    emptyTitleFont.setPointSize(16);
    emptyTitle->setFont(emptyTitleFont);
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyTitle->setStyleSheet("color: #888888;");

    QLabel* emptyDesc = new QLabel("Start authentication to see logs here");
    emptyDesc->setAlignment(Qt::AlignCenter);
    emptyDesc->setStyleSheet("color: #AAAAAA;");

    emptyGroupLayout->addWidget(emptyIcon);
    emptyGroupLayout->addWidget(emptyTitle);
    emptyGroupLayout->addWidget(emptyDesc);

    emptyGroup->setLayout(emptyGroupLayout);
    emptyLayout->addWidget(emptyGroup);

    // Content widget
    m_logsContentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(m_logsContentWidget);

    QGroupBox* logsGroup = new QGroupBox("Activity Log");
    logsGroup->setStyleSheet("QGroupBox { background-color: rgba(255, 255, 255, 150); color: #AF52DE; }");
    QVBoxLayout* logsLayout = new QVBoxLayout();

    m_logsList = new QListWidget();
    m_logsList->setFont(QFont("Monospace", 9));
    m_logsList->setStyleSheet("QListWidget { background-color: rgba(255, 255, 255, 0.3); "
                             "border-radius: 6px; padding: 8px; }"
                             "QListWidget::item { background-color: rgba(255, 255, 255, 0.3); "
                             "border-radius: 6px; padding: 6px 12px; margin: 3px; }");
    logsLayout->addWidget(m_logsList);

    logsGroup->setLayout(logsLayout);
    contentLayout->addWidget(logsGroup);

    mainLayout->addWidget(m_logsEmptyWidget);
    mainLayout->addWidget(m_logsContentWidget);

    // Initially hide content widget, show empty widget
    m_logsContentWidget->hide();

    m_tabWidget->addTab(logsTab, "📄 Logs");
}

void MainWindow::onBeginAuthentication()
{
    // Validate inputs
    if (m_issuerURLEdit->text().trimmed().isEmpty()) {
        m_configErrorLabel->setText("Please enter an Issuer URL");
        m_configErrorLabel->show();
        return;
    }

    if (m_clientIDEdit->text().trimmed().isEmpty()) {
        m_configErrorLabel->setText("Please enter a Client ID");
        m_configErrorLabel->show();
        return;
    }

    m_configErrorLabel->hide();
    m_authErrorLabel->hide();

    // Switch to authentication tab
    m_tabWidget->setCurrentIndex(1);

    // Show active authentication widget
    m_isAuthenticating = true;
    m_authIdleWidget->hide();
    m_authActiveWidget->show();
    m_beginAuthButton->setEnabled(false);
    m_authBeginButton->setEnabled(false);

    // Start authentication
    m_oidcManager->startAuthentication(
        m_issuerURLEdit->text().trimmed(),
        m_clientIDEdit->text().trimmed(),
        m_clientSecretEdit->text().trimmed(),
        m_scopesEdit->text().trimmed(),
        m_acrValueCombo->currentText(),
        m_loginHintEdit->text().trimmed(),
        m_promptLoginCheck->isChecked(),
        m_responseTypeEdit->text().trimmed(),
        m_extraParamsEdit->text().trimmed()
    );
}

void MainWindow::onCancelAuthentication()
{
    m_oidcManager->cancelAuthentication();

    m_isAuthenticating = false;
    m_authIdleWidget->show();
    m_authActiveWidget->hide();
    m_beginAuthButton->setEnabled(true);
    m_authBeginButton->setEnabled(true);
}

void MainWindow::onProgressUpdated(const QString& message)
{
    m_authProgressLabel->setText(message);
}

void MainWindow::onErrorOccurred(const QString& error)
{
    m_authErrorLabel->setText(QString("Error: %1").arg(error));
    m_authErrorLabel->show();

    m_isAuthenticating = false;
    m_authIdleWidget->show();
    m_authActiveWidget->hide();
    m_beginAuthButton->setEnabled(true);
    m_authBeginButton->setEnabled(true);
}

void MainWindow::onTokensReceived(const QString& tokens)
{
    m_currentTokens = tokens;

    // Update raw tokens
    m_rawTokensText->setPlainText(tokens);

    // Update decoded tokens
    QString decoded = getDecodedTokenDetails();
    if (!decoded.isEmpty()) {
        m_decodedTokensText->setPlainText(decoded);
    } else {
        m_decodedTokensText->setPlainText("No JWT tokens to decode.");
    }

    // Show tokens content
    m_tokensEmptyWidget->hide();
    m_tokensContentWidget->show();

    // Switch to tokens tab
    m_tabWidget->setCurrentIndex(2);

    // Reset authentication state
    m_isAuthenticating = false;
    m_authIdleWidget->show();
    m_authActiveWidget->hide();
    m_beginAuthButton->setEnabled(true);
    m_authBeginButton->setEnabled(true);
}

void MainWindow::onLogMessage(const QString& message)
{
    // Add to logs list
    m_logsList->addItem("● " + message);
    m_logsList->scrollToBottom();

    // Show logs content on first message
    if (m_logsEmptyWidget->isVisible() || !m_logsContentWidget->isVisible()) {
        m_logsEmptyWidget->hide();
        m_logsContentWidget->show();
    }
}

QString MainWindow::getDecodedTokenDetails()
{
    if (m_currentTokens.isEmpty()) {
        return QString();
    }

    QString result;
    QStringList lines = m_currentTokens.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("ID Token: ")) {
            QString tokenValue = line.mid(QString("ID Token: ").length()).trimmed();
            if (!tokenValue.isEmpty()) {
                result += "=== ID TOKEN DETAILS ===\n";
                result += JWTDecoder::formatTokenDetails(tokenValue);
                result += "\n";
            }
        } else if (line.startsWith("Access Token: ")) {
            QString tokenValue = line.mid(QString("Access Token: ").length()).trimmed();
            if (!tokenValue.isEmpty()) {
                result += "=== ACCESS TOKEN DETAILS ===\n";
                result += JWTDecoder::formatTokenDetails(tokenValue);
                result += "\n";
            }
        }
    }

    return result;
}

void MainWindow::loadSettings()
{
    QSettings settings;

    m_issuerURLEdit->setText(settings.value("issuerURL", "").toString());
    m_clientIDEdit->setText(settings.value("clientID", "").toString());
    m_clientSecretEdit->setText(settings.value("clientSecret", "").toString());
    m_acrValueCombo->setCurrentText(settings.value("acrValue", "None").toString());
    m_loginHintEdit->setText(settings.value("loginHint", "").toString());
    m_promptLoginCheck->setChecked(settings.value("promptLogin", false).toBool());
    m_scopesEdit->setText(settings.value("scopes", "openid profile email").toString());
    m_responseTypeEdit->setText(settings.value("responseType", "code").toString());
    m_extraParamsEdit->setText(settings.value("extraParams", "").toString());
}

void MainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue("issuerURL", m_issuerURLEdit->text());
    settings.setValue("clientID", m_clientIDEdit->text());
    settings.setValue("clientSecret", m_clientSecretEdit->text());
    settings.setValue("acrValue", m_acrValueCombo->currentText());
    settings.setValue("loginHint", m_loginHintEdit->text());
    settings.setValue("promptLogin", m_promptLoginCheck->isChecked());
    settings.setValue("scopes", m_scopesEdit->text());
    settings.setValue("responseType", m_responseTypeEdit->text());
    settings.setValue("extraParams", m_extraParamsEdit->text());
}

