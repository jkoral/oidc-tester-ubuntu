#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("OIDC Tester");
    QApplication::setOrganizationName("OIDC Tester");
    QApplication::setApplicationVersion("1.0.0");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

