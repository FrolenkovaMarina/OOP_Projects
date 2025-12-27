#include "RoadLane.h"
#include <cstdlib>
#include <ctime>


// чтобы srand вызывался один раз
static void ensureRandomSeed()
{
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }
}


RoadLane::RoadLane(int index)
    : Lane(index)
{
    ensureRandomSeed();

    direction_ = (lane_index_ % 2 == 0) ? 1 : -1;

    float t = static_cast<float>(std::rand()) / RAND_MAX;
    base_speed_ = 1.2f + 1.5f * t;

    // создаем стартовые машины
    initCars();
}


void RoadLane::initCars()
{
    // заново заполняем список машин
    cars_.clear();

    const int width = Config::GRID_WIDTH;

    // идем слева направо и расставляем машины с зазорами
    int cur_col = 0;
    bool first = true;
    int last_end = -100;

    while (cur_col < width)
    {
        int length_cells = (std::rand() % 4 == 0) ? 3 : 2;

        if (!first)
        {
            int min_gap_before = (length_cells == 3) ? 2 : 1;

            if (cur_col - last_end < min_gap_before)
            {
                cur_col = last_end + min_gap_before;
            }
        }

        if (cur_col + length_cells > width)
            break;

        int sprite_variant;
        if (length_cells == 2)
            sprite_variant = std::rand() % 5;
        else
            sprite_variant = std::rand() % 3;

        cars_.push_back(
            std::make_unique<Car>(
                lane_index_,
                cur_col,
                base_speed_,
                direction_,
                static_cast<float>(length_cells),
                sprite_variant
            )
        );

        last_end = cur_col + length_cells;
        first = false;

        // зазор после машины чтобы они не слипались
        int base_gap_after = (length_cells == 3) ? 3 : 2;
        int extra_gap = std::rand() % 2;

        int gap_after = base_gap_after + extra_gap;

        cur_col = last_end + gap_after;
    }
}


void RoadLane::update(float dt)
{
    const int width = Config::GRID_WIDTH;

    // сначала обновляем позиции всех машин
    for (auto& carPtr : cars_)
        carPtr->update(dt);

    // потом проверяем кто уехал за экран и делаем респавн
    for (auto& carPtr : cars_)
    {
        Car& car = *carPtr;

        float x = car.getCellX();
        int len = static_cast<int>(car.getWidthCells());

        if (direction_ > 0)
        {
            if (x - len > width + 2)
            {
                float min_x = 1e9f;
                bool found = false;

                for (const auto& otherPtr : cars_)
                {
                    if (otherPtr.get() == &car) continue;
                    float ox = otherPtr->getCellX();
                    if (ox < min_x) min_x = ox;
                    found = true;
                }

                int base = (len == 3) ? 3 : 2;
                int extra = std::rand() % 2;
                int gap = base + extra;

                float new_x = found
                    ? (min_x - static_cast<float>(gap) - static_cast<float>(len))
                    : (-static_cast<float>(len) - 2.f);

                float offscreen_x = -static_cast<float>(len) - 2.f;
                if (new_x > offscreen_x)
                    new_x = offscreen_x;

                car.setCellX(new_x);
            }
        }
        else
        {
            if (x + len < -2)
            {
                float max_x = -1e9f;
                bool found = false;

                for (const auto& otherPtr : cars_)
                {
                    if (otherPtr.get() == &car) continue;
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

                float offscreen_x = static_cast<float>(width) + 2.f;
                if (new_x < offscreen_x)
                    new_x = offscreen_x;

                car.setCellX(new_x);
            }
        }
    }
}
