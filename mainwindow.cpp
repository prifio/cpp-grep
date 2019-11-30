#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <thread>
#include <sstream>
#include <iostream>
#include <string>

QString get_thread_id() {
    auto myid = std::this_thread::get_id();
    std::stringstream ss;
    ss << myid;
    return QString::fromStdString(ss.str());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->ready_status->setPlainText("ready");

    connect(ui->button, &QPushButton::released, this, [this]() {
        std::string path = ui->input_path->toPlainText().toStdString();
        std::string text = ui->input_text->toPlainText().toStdString();
        background.data.is_canceled_spec = false;
        update_spec(path, text, background.data);
        ui->output_result->clear();
        ui->output_path->setPlainText(QString::fromStdString(path));
        ui->output_text->setPlainText(QString::fromStdString(text));
        update_status(false, false, ui);
    });

    connect(ui->cancel_button, &QPushButton::released, this, [this]() {
        background.data.is_canceled_spec = true;
        update_spec("", "", background.data);
        update_status(true, true, ui);
    });

    connect(&background, &background_thread::push_result, this,
    [this](std::vector<std::string> result, size_t epoch){
        if (epoch == background.data.epoch) {
            for (auto& i : result) {
                ui->output_result->appendPlainText(QString::fromStdString(i));
            }
        }
        background.data.is_ready_write = true;
        background.data.update_boss();
    });

    connect(&background, &background_thread::update_condition, this, [this](bool ready){
        update_status(ready, background.data.is_canceled_spec, ui);
    });
    background.data.is_ready_write = true;
}

void update_spec(const std::string& path, const std::string& text, all_data& data) {
    std::unique_lock<std::mutex> ul(data.mspec);
    data.epoch++;
    data.spec_path = path;
    data.spec_text = text;
    data.update_boss();
}

void update_status(bool ready, bool canceled, const Ui::MainWindow* ui) {
    if (canceled) {
        ui->ready_status->setPlainText("canceled");
    } else if (ready) {
        ui->ready_status->setPlainText("ready");
    } else {
        ui->ready_status->setPlainText("not ready");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
