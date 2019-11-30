#include "all_data.h"

all_data::all_data()
{
}

void all_data::update_boss() {
    std::unique_lock<std::mutex> ul(mboss_update);
    is_boss_updated = true;
    boss_update.notify_one();
}
