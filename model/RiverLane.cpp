#include "RiverLane.h"
#include <cstdlib>
#include <ctime>


// чтобы srand вызывался один раз и рандом не сбивался каждый раз
static void ensureRiverSeed()
{
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }
}


RiverLane::RiverLane(int index)
    : Lane(index)
{
    ensureRiverSeed();

    // направление разное через одну полосу
    direction_ = (lane_index_ % 2 == 0) ? 1 : -1;

    // скорость случайно
    float t = static_cast<float>(std::rand()) / RAND_MAX;
    base_speed_ = 0.8f + 1.2f * t;

    // создаем бревна
    initLogs();
}


void RiverLane::initLogs()
{
    // заново наполняем полосу бревнами
    logs_.clear();

    const int width = Config::GRID_WIDTH;

    // идем слева направо и пытаемся ставить бревна с зазорами
    int cur_col = 0;
    bool first = true;
    int last_end = -100;

    while (cur_col < width)
    {
        int length_cells = (std::rand() % 3 == 0) ? 3 : 2;
        int min_gap_before = (length_cells == 3 ? 2 : 1);

        if (!first)
        {
            if (cur_col - last_end < min_gap_before)
                cur_col = last_end + min_gap_before;
        }

        if (cur_col + length_cells > width)
            break;

        int sprite_variant = (length_cells == 3)
            ? std::rand() % 2
            : 0;

        logs_.push_back(std::make_unique<Log>(
            lane_index_,
            cur_col,
            base_speed_,
            direction_,
            static_cast<float>(length_cells),
            sprite_variant
        ));

        last_end = cur_col + length_cells;
        first = false;

        int gap = (length_cells == 3 ? 3 : 2) + (std::rand() % 2);
        cur_col = last_end + gap;
    }
}


void RiverLane::update(float dt)
{
    const int width = Config::GRID_WIDTH;

    // обновляем каждое бревно и делаем респавн когда оно уехало за экран
    for (auto& logPtr : logs_)
    {
        Log& log = *logPtr;
        log.update(dt);

        float x = log.getCellX();
        int   len = static_cast<int>(log.getWidthCells());

        if (direction_ > 0)
        {
            if (x - len > width + 2)
            {
                // ищем самое левое бревно, чтобы поставить это еще левее с зазором
                float min_x = 1e9f;
                bool found = false;

                for (const auto& otherPtr : logs_)
                {
                    if (otherPtr.get() == &log) continue;
                    float ox = otherPtr->getCellX();
                    if (ox < min_x) min_x = ox;
                    found = true;
                }

                int base = (len == 3) ? 3 : 2;
                int extra = std::rand() % 2;
                int gap = base + extra;

                // если нашли другое бревно, ставим левее него
                // если нет, ставим просто за экран
                float new_x = found
                    ? (min_x - static_cast<float>(gap) - static_cast<float>(len))
                    : (-static_cast<float>(len) - 2.f);

                // гарантируем что новое положение точно вне экрана
                float offscreen_x = -static_cast<float>(len) - 2.f;
                if (new_x > offscreen_x)
                    new_x = offscreen_x;

                log.setCellX(new_x);
            }
        }
        else
        {
            // движение влево
            if (x + len < -2)
            {
                float max_x = -1e9f;
                bool found = false;

                for (const auto& otherPtr : logs_)
                {
                    if (otherPtr.get() == &log) continue;
                    float ox = otherPtr->getCellX();
                    if (ox > max_x) max_x = ox;
                    found = true;
                }

                int base = (len == 3) ? 3 : 2;
                int extra = std::rand() % 2;
                int gap = base + extra;

                float new_x = found
                    ? (max_x + static_cast<float>(gap) + static_cast<float>(len))
                    : (static_cast<float>(width) + 2.f);

                // гарантируем что новое положение точно вне экрана
                float offscreen_x = static_cast<float>(width) + 2.f;
                if (new_x < offscreen_x)
                    new_x = offscreen_x;

                log.setCellX(new_x);
            }
        }
    }
}
