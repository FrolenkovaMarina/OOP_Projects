#pragma once

#include "GameObject.h"

enum class PlayerState {
    Alive,
    Dead
};

class Player : public GameObject {
private:
    PlayerState state;
    float abs_x_ = 0.f;
    bool on_log_ = false;


public:

    Player(int start_row, int start_col);

    void moveBy(int d_row, int dCol);
 
    PlayerState getState() const;

    void kill();
    void revive();

    void update(float dt) override;

    float getAbsX() const { return abs_x_; }
    void setAbsX(float x) { abs_x_ = x; }

    void setOnLog(bool v) { on_log_ = v; }
    bool isOnLog() const { return on_log_; }


};
