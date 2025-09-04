#include <iostream>
#include <QtWidgets/QApplication>
#include "mainwindow.hpp"

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) { // start at 1 to throw out the program call
        std::cout << argv[i] << "\n";
    }

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}