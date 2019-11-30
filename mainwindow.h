#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "background_thread.h"

#include <QMainWindow>

QString get_thread_id();

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    background_thread background;
};

void update_status(bool ready, bool canceled, const Ui::MainWindow* ui);
void update_spec(const std::string& path, const std::string& text, all_data& data);

#endif // MAINWINDOW_H
