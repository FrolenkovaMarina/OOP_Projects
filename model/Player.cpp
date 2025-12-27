#include "Player.h"

// игрок хранит свою позицию и состояние
// вся логика столкновений обрабатывается в model



Player::Player(int start_row, int start_col)
    : GameObject(start_row, start_col)
    , state(PlayerState::Alive)
{
    // абсолютная поз по x для движения вместе с бревнами
    abs_x_ = static_cast<float>(col) + 0.5f;
    on_log_ = false;
}

// перемещение игрока на заданное число клеток
void Player::moveBy(int d_row, int dCol)
{
    row += d_row;
    col += dCol;
}

PlayerState Player::getState() const
{
    return state;
}

void Player::kill()
{
    state = PlayerState::Dead;
}

void Player::revive()
{
    state = PlayerState::Alive;
}

void Player::update(float)
{
}
