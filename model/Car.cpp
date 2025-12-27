#include "Car.h"
#include <cmath>


// движение идет в долях клетки чтобы было плавно
Car::Car(int row,
    int start_col,
    float speed_cells_per_sec,
    int direction,
    float length_cells,
    int sprite_variant)
    : GameObject(row, start_col)
    , speed_cells_per_sec_(speed_cells_per_sec)
    // направление всегда либо 1 либо -1
    , direction_(direction >= 0 ? 1 : -1)
    // дробная часть позиции внутри клетки
    , offset_(0.0f)
    // длина машины в клетках
    , length_cells_(length_cells)
    // вариант спрайта чтобы машины выглядели по разному
    , sprite_variant_(sprite_variant)
{
}


void Car::update(float dt)
{
    // сколько клеток проехала машина за этот кадр
    float delta = speed_cells_per_sec_ * dt;
    delta *= static_cast<float>(direction_);

    // текущая позиция в клетках с учетом дробной части
    float x_cells = static_cast<float>(col) + offset_;
    x_cells += delta;

    // целая часть это номер колонки
    int   new_col = static_cast<int>(std::floor(x_cells));
    // дробная часть это смещение внутри клетки
    float new_offset = x_cells - static_cast<float>(new_col);

    col = new_col;
    offset_ = new_offset;
}


// установка позиции машины напрямую в клетках
//  при спавне или принудительном переносе
void Car::setCellX(float x_cells)
{
    int   new_col = static_cast<int>(std::floor(x_cells));
    float new_offset = x_cells - static_cast<float>(new_col);

    col = new_col;
    offset_ = new_offset;
}
