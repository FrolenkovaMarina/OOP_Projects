#include "Log.h"
#include <cmath>


Log::Log(int row,
    int start_col,
    float speed_cells_per_sec,
    int direction,
    float length_cells,
    int sprite_variant)
    : GameObject(row, start_col)
    , speed_cells_per_sec_(speed_cells_per_sec)
    , direction_(direction >= 0 ? 1 : -1)
    , length_cells_(length_cells)
    , offset_(0.0f)
    , sprite_variant_(sprite_variant)
{
}


void Log::update(float dt)
{
    // сколько клеток сдвинулось бревно за этот кадр
    float delta = speed_cells_per_sec_ * dt;
    delta *= (float)direction_;

    // текущая позиция с учетом дробной части
    float x_cells = (float)col + offset_;
    x_cells += delta;

    // целая часть - колонка
    int new_col = (int)std::floor(x_cells);
    //смещение внутри клетки
    float new_offset = x_cells - (float)new_col;

    col = new_col;
    offset_ = new_offset;
}


// установка позиции бревна напрямую при спавне или корректировке
void Log::setCellX(float x_cells)
{
    int new_col = (int)std::floor(x_cells);
    float new_offset = x_cells - (float)new_col;

    col = new_col;
    offset_ = new_offset;
}
