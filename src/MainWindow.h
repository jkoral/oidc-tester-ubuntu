#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include "OIDCManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBeginAuthentication();
    void onCancelAuthentication();
    void onProgressUpdated(const QString& message);
    void onErrorOccurred(const QString& error);
    void onTokensReceived(const QString& tokens);
    void onLogMessage(const QString& message);

private:
    void setupUI();
    void createConfigTab();
    void createAuthenticationTab();
    void createTokensTab();
    void createLogsTab();
    void loadSettings();
    void saveSettings();
    QString getDecodedTokenDetails();
    
    QTabWidget* m_tabWidget;
    OIDCManager* m_oidcManager;
    
    // Config tab widgets
    QLineEdit* m_issuerURLEdit;
    QLineEdit* m_clientIDEdit;
    QLineEdit* m_clientSecretEdit;
    QComboBox* m_acrValueCombo;
    QLineEdit* m_loginHintEdit;
    QCheckBox* m_promptLoginCheck;
    QLineEdit* m_scopesEdit;
    QLineEdit* m_responseTypeEdit;
    QLineEdit* m_extraParamsEdit;
    QLabel* m_redirectURILabel;
    QPushButton* m_beginAuthButton;
    QLabel* m_configErrorLabel;
    
    // Authentication tab widgets
    QLabel* m_authStatusLabel;
    QLabel* m_authProgressLabel;
    QLabel* m_authErrorLabel;
    QPushButton* m_authBeginButton;
    QPushButton* m_cancelAuthButton;
    QWidget* m_authIdleWidget;
    QWidget* m_authActiveWidget;
    
    // Tokens tab widgets
    QTextEdit* m_rawTokensText;
    QTextEdit* m_decodedTokensText;
    QWidget* m_tokensEmptyWidget;
    QWidget* m_tokensContentWidget;
    
    // Logs tab widgets
    QListWidget* m_logsList;
    QWidget* m_logsEmptyWidget;
    QWidget* m_logsContentWidget;
    
    bool m_isAuthenticating;
    QString m_currentTokens;
};

#endif // MAINWINDOW_H

