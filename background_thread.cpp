#include "background_thread.h"
#include "mainwindow.h"
#include <reader.h>

#include <experimental/filesystem>
#include <sstream>
#include <iostream>

namespace fs = std::experimental::filesystem;

background_thread::background_thread() :
    boss([this]()
{
    for (size_t i = 0; i < WORKER_THREADS; ++i) {
        workers[i] = std::thread([this](){ worker_function(data); });
    }

    std::queue<fs::path> paths;
    size_t last_epoch = 0;
    while (true) {
        {
            std::unique_lock<std::mutex> ul1(data.mreq);
            std::unique_lock<std::mutex> ul2(data.mresult);
            bool status_ready = paths.size() == 0 && data.qreq.size() == 0 &&
                    data.qresult.size() == 0 && last_epoch == data.epoch &&
                    data.workers_done == WORKER_THREADS;
            if (status_ready != data.status_ready) {
                data.status_ready = status_ready;
                emit update_condition(status_ready);
            }
        }
        if (paths.size() == 0) {
            std::unique_lock<std::mutex> ul(data.mboss_update);
            while (!data.is_boss_updated) {
                data.boss_update.wait(ul);
            }
            data.is_boss_updated = false;
        }

        if (last_epoch < data.epoch) {
            std::unique_lock<std::mutex> ul1(data.mspec);
            last_epoch = data.epoch;
            while (paths.size() > 0) {
                paths.pop();
            }
            if (!data.spec_path.empty() && !data.spec_text.empty()) {
                paths.push(data.spec_path);
            }
            std::unique_lock<std::mutex> ul2(data.mkmp);
            data.kmp = KMP(data.spec_text);
        }
        if (paths.size() > 0) {
            fs::path path = paths.front();
            paths.pop();
            if (fs::is_directory(path)) {
                for (const auto & entry : fs::directory_iterator(path)) {
                    paths.push(entry.path());
                }
            } else {
                std::unique_lock<std::mutex> ul(data.mreq);
                data.qreq.push(std::make_pair(path, last_epoch));
                data.worker_update.notify_one();
            }
        }
        {
            std::unique_lock<std::mutex> ul(data.mresult);
            while (data.qresult.size() > 0 &&
                   data.qresult.front().second < last_epoch) {
                data.qresult.pop();
            }
            if (data.is_ready_write && data.qresult.size() > 0 &&
                    (data.qresult.size() >= GROUP_SIZE || data.workers_done == WORKER_THREADS)) {
                std::vector<std::string> group_result;
                for (size_t i = 0; i < GROUP_SIZE && data.qresult.size() > 0; ++i) {
                    group_result.push_back(data.qresult.front().first);
                    data.qresult.pop();
                }
                data.is_ready_write = false;
                emit push_result(group_result, last_epoch);
            }
        }
    }
})
{
}

std::pair<fs::path, size_t> next_jobe(all_data& data) {
    std::unique_lock<std::mutex> ul(data.mreq);
    bool was_wait = false;
    if (data.qreq.size() == 0) {
        was_wait = true;
        data.workers_done++;
        data.update_boss();
    }
    while (data.qreq.size() == 0) {
        data.worker_update.wait(ul);
    }
    if (was_wait) {
        data.workers_done--;
    }
    auto result = data.qreq.front();
    data.qreq.pop();
    return result;
}

void worker_function(all_data& data) {
    KMP kmp("");
    size_t last_kmp_epoch = 0;
    while (true) {
        auto p = next_jobe(data);
        std::string s = p.first.string();
        size_t current_epoch = p.second;
        if (current_epoch > last_kmp_epoch) {
            last_kmp_epoch = current_epoch;
            std::unique_lock<std::mutex> ul(data.mkmp);
            kmp = data.kmp;
        }
        try {
            std::vector<std::pair<size_t, size_t> > results;
            reader r(s.c_str());
            size_t now_kmp = 0;
            size_t pos_in_line = 0;
            size_t line = 1;
            while (!r.is_end() && current_epoch == data.epoch) {
                char c = r.read_char();
                if (c == '\n') {
                    now_kmp = 0;
                    pos_in_line = 0;
                    ++line;
                } else {
                    ++pos_in_line;
                    now_kmp = kmp.next(now_kmp, c);
                    if (now_kmp == kmp.len) {
                        results.push_back(std::make_pair(line, pos_in_line));
                    }
                }
            }
            if (results.size() > 0 && current_epoch == data.epoch) {
                std::unique_lock<std::mutex> ul(data.mresult);
                if (current_epoch == data.epoch) {
                    for (const auto& i : results) {
                        std::stringstream sb;
                        sb << s << ':' << i.first << ':' << i.second;
                        data.qresult.push(std::make_pair(sb.str(), current_epoch));
                        data.update_boss();
                    }
                }
            }
        } catch (const read_exception& e) {
            std::cerr << e.reasone << " ERR for file " << s << std::endl;
        }
    }
}
