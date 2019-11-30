#ifndef BACKGROUND_THREAD_H
#define BACKGROUND_THREAD_H

#include "all_data.h"
#include "constants.h"

#include <QObject>
#include <thread>
#include <string>

class background_thread : public QObject
{
private:
    Q_OBJECT
public:
    explicit background_thread();
    all_data data;
signals:
    void update_condition(bool ready);
    void push_result(std::vector<std::string>, size_t);
public slots:

private:
    std::thread boss;
    std::thread workers[WORKER_THREADS];
};

void worker_function(all_data&);

#endif // BACKGROUND_THREAD_H
