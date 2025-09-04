#ifndef mainwindow_h
#define mainwindow_h

#include <QtWidgets/QMainWindow>
#include <QtCore/QScopedPointer>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private:
    QScopedPointer<Ui::MainWindow> ui;
};

#endif