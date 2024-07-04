#include <QApplication>
#include "CampusNavigator.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CampusNavigator navigator;
    navigator.show();

    return app.exec();
}
