#pragma once

#include "GameObject.h"


class Car : public GameObject {
public:

    Car(int row,
        int start_col,
        float speed_cells_per_sec,
        int direction,
        float length_cells,
        int sprite_variant);

    void update(float dt) override;

    float getCellX() const { return static_cast<float>(col) + offset_; }

    float getWidthCells() const { return length_cells_; }

    void setCellX(float x_cells);


    int getSpriteVariant() const { return sprite_variant_; }

    float getSpeed() const { return speed_cells_per_sec_; }
    int getDirection() const { return direction_; }

private:

    float speed_cells_per_sec_;

    int direction_;

    float offset_;

    float length_cells_;

    int sprite_variant_;
};
