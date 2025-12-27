#include "GameModel.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// игровая модель: тут хранится состояние игры и вся логика обновления мира
// view рисует, Controller вызывает методы движения, а GameModel считает коллизии и генерирует полосы



// колонка в заданных границах [min_c; max_c] чтобы путь не уезжал слишком далеко влево/вправо
static int clamp_col(int c, int min_c, int max_c)
{
    if (c < min_c) return min_c;
    if (c > max_c) return max_c;
    return c;
}

// размеры поля, стартовую позицию игрока и начальные параметры камеры
GameModel::GameModel()
    : width_(Config::GRID_WIDTH)
    , height_(Config::GRID_HEIGHT)
    , player_(height_ * 3 / 4, width_ / 2)
    , state_(GameState::Running)
    , world_offset_(0.f)
    , camera_offset_(0.f)

{
    std::srand((unsigned)std::time(nullptr));

    initLanes();
}

// мы идём от низа экрана к верху 
void GameModel::initLanes()
{
    lanes_.clear();

    int start_row = player_.getRow();
    int road_streak = 0;
    int river_streak = 0;


    path_open_cols_.clear();
    path_open_cols_.push_back(player_.getCol());

    // logical_row — логическая строка мира (может быть и отрицательной чтобы заполнять верх)
    for (int logical_row = height_ - 1; logical_row >= -1; --logical_row)
    {
        int index = logical_row + 1;

        if (logical_row >= start_row)
        {
            auto open_cols = makeOpenColsForGrassLane(logical_row);
            lanes_.push_front(lane_factory_.create(LaneId::Grass, index, &open_cols));

            road_streak = 0;
            river_streak = 0;
            continue;
        }

        bool want_river = (std::rand() % 5 == 0);
        bool want_road = (std::rand() % 3 == 0);

        if (river_streak >= 3) want_river = false;
        if (road_streak >= 3)  want_road = false;
        if (want_river) want_road = false;

        if (want_river)
        {
            lanes_.push_front(lane_factory_.create(LaneId::River, index));
            river_streak++;
            road_streak = 0;
        }
        else if (want_road)
        {
            lanes_.push_front(lane_factory_.create(LaneId::Road, index));
            road_streak++;
            river_streak = 0;
        }
        else
        {
            genNextPathOpenCols();

            auto open_cols = makeOpenColsForGrassLane(logical_row);
            lanes_.push_front(lane_factory_.create(LaneId::Grass, index, &open_cols));

            river_streak = 0;
            road_streak = 0;
        }
    }
}


// добавляем новую полосу сверху, когда мир прокручивается
// индекс полосы растёт, чтобы объекты знали, где они в мире
void GameModel::addNewTopLane()
{
    int new_index = 0;
    if (!lanes_.empty()) {
        new_index = lanes_.front()->getIndex() + 1;
    }


    int river_streak = 0;
    int road_streak = 0;

    for (std::size_t i = 0; i < lanes_.size(); ++i)
    {
        if (dynamic_cast<RiverLane*>(lanes_[i].get()))
        {
            if (road_streak > 0) break;
            ++river_streak;
        }
        else if (dynamic_cast<RoadLane*>(lanes_[i].get()))
        {
            if (river_streak > 0) break;
            ++road_streak;
        }
        else
        {
            break;
        }
    }

    bool want_river = (std::rand() % 5 == 0);
    bool want_road = (std::rand() % 3 == 0);

    if (river_streak >= 3) want_river = false;
    if (road_streak >= 3) want_road = false;

    if (want_river) want_road = false;

    std::unique_ptr<Lane> lane;
    if (want_river)
        lane = lane_factory_.create(LaneId::River, new_index);
    else if (want_road)
        lane = lane_factory_.create(LaneId::Road, new_index);
    else
    {
        genNextPathOpenCols();

        int lane_row = new_index - 1;
        auto open_cols = makeOpenColsForGrassLane(lane_row);

        lane = lane_factory_.create(LaneId::Grass, new_index, &open_cols);
    }

    lanes_.push_front(std::move(lane));
}


// прокрутка мира вверх (по клеткам)
void GameModel::scrollWorld(float cells, bool movePlayerWithWorld)
{
    world_offset_ += cells;

    // пока накопили целую клетку прокрутки — реально перестраиваем мир
    while (world_offset_ >= 1.f)
    {
        world_offset_ -= 1.f;

        if (movePlayerWithWorld)
            player_.moveBy(1, 0);

        // удаляем нижнюю полосу (она ушла за экран)
        if (!lanes_.empty())
            lanes_.pop_back();


        addNewTopLane();
    }
}

// прыжок вверх. Если дошли до границы (LOCK_ROW), дальше двигаем не игрока, а мир
void GameModel::movePlayerUp()
{
    if (state_ != GameState::Running)
        return;

    if (player_.isOnLog())
    {
        player_.setCol(static_cast<int>(std::floor(player_.getAbsX())));
    }

    int row = player_.getRow();
    int col = player_.getCol();

    const int LOCK_ROW = height_ / 3;

    if (row > LOCK_ROW)
    {
        int new_row = row - 1;

        if (isCellBlocked(new_row, col))
            return;

        player_.moveBy(-1, 0);

        player_.setOnLog(false);
        player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

        last_jump_dir_ = JumpDir::Up;
        ++jump_serial_;
        return;
    }

    int next_row = row - 1;
    if (next_row >= 0)
    {
        if (isCellBlocked(next_row, col))
            return;
    }

    scrollWorld(1.f, false);

    player_.setOnLog(false);
    player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

    last_jump_dir_ = JumpDir::Up;
    ++jump_serial_;
}


// прыжок вниз (только если не выходим за предел поля и клетка не занята на траве)
void GameModel::movePlayerDown()
{
    if (state_ != GameState::Running)
        return;

    if (player_.isOnLog())
    {
        player_.setCol(static_cast<int>(std::floor(player_.getAbsX())));
    }

    int row = player_.getRow();
    int col = player_.getCol();

    int new_row = row + 1;
    if (new_row >= height_)
        return;

    if (isCellBlocked(new_row, col))
        return;

    player_.moveBy(+1, 0);

    player_.setOnLog(false);
    player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

    last_jump_dir_ = JumpDir::Down;
    ++jump_serial_;
}


// прыжок влево. Если стоим на бревне, сначала прибиваем колонку к absX
void GameModel::movePlayerLeft()
{
    if (state_ != GameState::Running)
        return;

    if (player_.isOnLog())
    {
        player_.setCol(static_cast<int>(std::floor(player_.getAbsX())));
    }

    int row = player_.getRow();
    int col = player_.getCol() - 1;

    if (col < 0)
        return;

    if (isCellBlocked(row, col))
        return;

    player_.moveBy(0, -1);

    player_.setOnLog(false);
    player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

    last_jump_dir_ = JumpDir::Left;
    ++jump_serial_;
}


// прыжок вправо. Проверяем границы и блоки на траве
void GameModel::movePlayerRight()
{
    if (state_ != GameState::Running)
        return;

    if (player_.isOnLog())
    {
        player_.setCol(static_cast<int>(std::floor(player_.getAbsX())));
    }

    int row = player_.getRow();
    int col = player_.getCol() + 1;

    if (col >= width_)
        return;

    if (isCellBlocked(row, col))
        return;

    player_.moveBy(0, +1);

    player_.setOnLog(false);
    player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

    last_jump_dir_ = JumpDir::Right;
    ++jump_serial_;
}


// главный 
// обновление игрока/полос, автопрокрутка, проверки на смерть (машины/река)
void GameModel::update(float dt)
{
    if (state_ != GameState::Running)
        return;

    // обновляем игрока и все полосы
    player_.update(dt);
    for (auto& lane : lanes_) {
        lane->update(dt);
    }

    // автопрокрутка мира игрок стоит, мир едет
    float auto_cells = Config::WORLD_SCROLL_SPEED * dt;
    if (auto_cells > 0.0f) {
        scrollWorld(auto_cells, true);
    }

    float tile = static_cast<float>(Config::TILE_SIZE);

    int player_row = player_.getRow();
    int player_col = player_.getCol();

    float center_x = player_.isOnLog()
        ? player_.getAbsX()
        : static_cast<float>(player_.getCol()) + 0.5f;

    float center_y = static_cast<float>(player_.getRow()) + world_offset_ + 0.5f;

    float shrink = 0.75f;
    float player_width = tile * shrink;
    float player_height = tile * shrink;

    float x_player = center_x * tile - player_width * 0.5f;
    float y_player = center_y * tile - player_height * 0.5f;

    float y_bottom = y_player + player_height;
    float window_height = static_cast<float>(height_) * tile;

    // если игрок ушёл ниже экрана (мир уехал), то проигрыш
    if (y_bottom >= window_height) {
        state_ = GameState::GameOver;
        player_.kill();
        return;
    }

    // идём сверху вниз по текущим полосам, пока идут реки/дороги подряд
    for (std::size_t i = 0; i < lanes_.size(); ++i)
    {
        auto* roadLane = dynamic_cast<RoadLane*>(lanes_[i].get());
        if (!roadLane)
            continue;

        int lane_row = static_cast<int>(i) - 1;

        if (lane_row != player_row)
            continue;

        float y_lane = (static_cast<float>(i) - 1.0f + world_offset_) * tile;

        const auto& cars = roadLane->getCars();
        for (const auto& carPtr : cars)
        {
            const Car& car = *carPtr;

            float car_cell_x = car.getCellX();
            float car_length_cells = car.getWidthCells();

            float shrink = 0.8f;
            float shift = 0.1f;

            float x_car = (car_cell_x + car_length_cells * shift) * tile;
            float y_car = y_lane;

            float car_width = car_length_cells * tile * shrink;
            float car_height = tile;

            bool overlap_x =
                x_player < x_car + car_width &&
                x_player + player_width > x_car;

            bool overlap_y =
                y_player < y_car + car_height &&
                y_player + player_height > y_car;

            // проверка пересечения прямоугольников
            if (overlap_x && overlap_y)
            {
                death_type_ = DeathType::None;

                state_ = GameState::GameOver;
                player_.kill();
                return;
            }
        }
    }

    // небольшая погрешность, чтобы не было дрожания на границах брёвен
    const float eps = 0.01f;

    bool player_on_river = false;
    bool player_on_log = false;

    // идём сверху вниз по текущим полосам, пока идут реки/дороги подряд
    for (std::size_t i = 0; i < lanes_.size(); ++i)
    {
        auto* riverLane = dynamic_cast<RiverLane*>(lanes_[i].get());
        if (!riverLane)
            continue;

        int lane_row = static_cast<int>(i) - 1;
        if (lane_row != player_row)
            continue;

        player_on_river = true;
        player_on_log = false;
        player_.setOnLog(false);

        float center_x = player_.getAbsX();

        const auto& logs = riverLane->getLogs();

        for (const auto& logPtr : logs)
        {
            const Log& log = *logPtr;

            float x_log = log.getCellX();
            float len = log.getWidthCells();

            float left = x_log;
            float right = x_log + len;


            if (center_x + eps >= left && center_x - eps <= right)
            {
                player_on_log = true;
                player_.setOnLog(true);

                // сдвигаем игрока вместе с бревном
                float delta = log.getSpeed() * log.getDirection() * dt;

                float new_center = center_x + delta;

                int new_col = static_cast<int>(std::floor(new_center));

                // если бревно унесло игрока за край — считаем проигрыш
                if (new_col < 0 || new_col >= width_)
                {
                    death_type_ = DeathType::None;
                    state_ = GameState::GameOver;
                    player_.kill();
                    return;
                }

                player_.setAbsX(new_center);
                player_.setCol(new_col);

                break;
            }
        }

        // на реке без бревна — утонул
        if (!player_on_log)
        {
            death_type_ = DeathType::Drown;
            state_ = GameState::GameOver;
            player_.kill();
            return;
        }

        break;
    }

    if (!player_on_river)
    {
        player_.setOnLog(false);
        player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);
    }

}

// есть ли в клетке препятствие (куст/камень и т.п.)
// на дорогах/реках тут всегда false — там столкновения считаем отдельно
bool GameModel::isCellBlocked(int row, int col) const
{

    int i = row + 1;
    if (i < 0 || i >= (int)lanes_.size()) return false;

    auto* grass = dynamic_cast<GrassLane*>(lanes_[i].get());
    if (!grass) return false;

    return grass->isBlocked(col);
}


// перезапуск игры
void GameModel::restart()
{
    state_ = GameState::Running;
    death_type_ = DeathType::None;

    player_.setRow(height_ * 3 / 4);
    player_.setCol(width_ / 2);
    player_.revive();

    player_.setOnLog(false);
    player_.setAbsX(static_cast<float>(player_.getCol()) + 0.5f);

    last_jump_dir_ = JumpDir::Up;


    world_offset_ = 0.f;
    camera_offset_ = 0.f;

    initLanes();
}


// генерация проходимых колонок на следующей траве
// path_open_cols_ хранит 1 или 2 колонки
std::vector<int> GameModel::genNextPathOpenCols()
{
    const int W = width_;

    // ограничиваем путь центральной частью экрана, чтобы игрока не загоняло в край
    const int min_c = W / 4;
    const int max_c = (3 * W) / 4;

    if (path_open_cols_.empty())
    {
        int start_col = player_.getCol();
        if (start_col < min_c) start_col = min_c;
        if (start_col > max_c) start_col = max_c;

        path_open_cols_ = { start_col };
        return path_open_cols_;
    }

    std::vector<int> cur = path_open_cols_;
    if (cur.size() == 2 && cur[0] > cur[1])
        std::swap(cur[0], cur[1]);

    // roll — просто число 0..99, по нему выбираем вариант (оставить/расширить/сдвинуть путь)
    int roll = std::rand() % 100;

    if (cur.size() == 1)
    {
        int c = clamp_col(cur[0], min_c, max_c);

        if (roll < 55)
        {
            path_open_cols_ = { c };
            return path_open_cols_;
        }

        if (roll < 90)
        {
            int dir = (std::rand() % 2) ? +1 : -1;
            int c2 = clamp_col(c + dir, min_c, max_c);

            if (c2 == c)
                c2 = clamp_col(c + 1, min_c, max_c);

            if (c2 < c)
                std::swap(c, c2);

            path_open_cols_ = { c, c2 };
            return path_open_cols_;
        }

        int dir = (std::rand() % 2) ? +1 : -1;
        int shifted = clamp_col(c + dir, min_c, max_c);


        if (shifted == c)
        {
            path_open_cols_ = { c };
        }
        else
        {
            int a = c;
            int b = shifted;
            if (a > b) std::swap(a, b);
            path_open_cols_ = { a, b };
        }

        return path_open_cols_;
    }

    int a = clamp_col(cur[0], min_c, max_c);
    int b = clamp_col(cur[1], min_c, max_c);

    if (roll < 45)
    {
        path_open_cols_ = { a, b };
        return path_open_cols_;
    }

    if (roll < 75)
    {
        path_open_cols_ = { a };
    }
    else
    {
        path_open_cols_ = { b };
    }

    return path_open_cols_;
}


// для травы: собираем список колонок, которые точно должны остаться свободными
// на строке игрока принудительно добавляем его колонку, чтобы не заспавнить препятствие под ним
std::vector<int> GameModel::makeOpenColsForGrassLane(int lane_row) const
{

    std::vector<int> open = path_open_cols_;

    if (lane_row == player_.getRow()) {
        int pc = player_.getCol();
        bool exists = false;
        for (int x : open) if (x == pc) exists = true;
        if (!exists) open.push_back(pc);
    }

    return open;
}