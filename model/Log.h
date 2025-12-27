#pragma once
#include "GameObject.h"

class Log : public GameObject {
public:
    Log(int row,
        int start_col,
        float speed_cells_per_sec,
        int direction,
        float length_cells,
        int sprite_variant);

    void update(float dt) override;
    void setCellX(float x_cells);

    float getCellX() const { return static_cast<float>(col) + offset_; }
    float getWidthCells() const { return length_cells_; }

    int   getDirection() const { return direction_; }
    float getSpeed() const { return speed_cells_per_sec_; }
    int   getSpriteVariant() const { return sprite_variant_; }

private:
    float speed_cells_per_sec_;
    int   direction_;
    float length_cells_;
    float offset_; 
    int   sprite_variant_;  
};
