#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "Config.h"
#include "Player.h"
#include "Lane.h"
#include "GrassLane.h"
#include "RoadLane.h"
#include "RiverLane.h"
#include "LaneFactory.h"


// координаты в пикселях
struct PixelPos {
    float x;
    float y;
};


enum class GameState {
    Running,
    GameOver
};


// тип смерти игрока
enum class DeathType {
    None,
    Drown,
    HitFront,
    HitSide
};


// направление последнего прыжка
enum class JumpDir {
    None,
    Up,
    Down,
    Left,
    Right
};


class GameModel {
private:
    // размеры сетки в клетках
    int width_;
    int height_;

    Player player_;
    GameState state_;

    // полосы (трава дорога река) deque удобно - удаляем снизу и добавляем сверху
    std::deque<std::unique_ptr<Lane>> lanes_;

    // фабрика - создаём разные полосы одинаковым способом
    LaneFactory lane_factory_;

    // дробное смещение мира вверх для плавной прокрутки
    float world_offset_;
    float camera_offset_;

    void initLanes();

    void addNewTopLane();

    void scrollWorld(float cells, bool movePlayerWithWorld);

    // проверка что клетка занята препятствием или опасная
    bool isCellBlocked(int row, int col) const;

    // путь
    std::vector<int> path_open_cols_;

    // путь дальше
    std::vector<int> genNextPathOpenCols();

    std::vector<int> makeOpenColsForGrassLane(int lane_row) const;

    int jump_serial_ = 0;

    JumpDir last_jump_dir_ = JumpDir::None;

    DeathType death_type_ = DeathType::None;


public:
    GameModel();

    // размеры сетки
    int gridWidth() const { return width_; }
    int gridHeight() const { return height_; }

    // смещения мира и камеры для отрисовки
    float getWorldOffset() const { return world_offset_; }
    float getCameraOffset() const { return camera_offset_; }

    Player& getPlayer() { return player_; }
    const Player& getPlayer() const { return player_; }

    GameState getState() const { return state_; }

    const std::deque<std::unique_ptr<Lane>>& getLanes() const { return lanes_; }

    int getJumpSerial() const { return jump_serial_; }
    JumpDir getLastJumpDir() const { return last_jump_dir_; }

    DeathType getDeathType() const { return death_type_; }

    void movePlayerUp();
    void movePlayerDown();
    void movePlayerLeft();
    void movePlayerRight();

    void update(float dt);

    void restart();
};
