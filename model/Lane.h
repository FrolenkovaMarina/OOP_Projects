#pragma once

#include <vector>
#include <memory>
#include "GameObject.h"


// для всех полос на карте: наследуются трава дорога и река


class Lane {
protected:
    // индекс полосы по вертикали
    int lane_index_;

public:
    Lane(int index)
        : lane_index_(index)
    {
    }

    virtual ~Lane() = default;

    // текущий индекс полосы
    int getIndex() const { return lane_index_; }

    // изменить индекс полосы при прокрутке мира
    void setIndex(int idx) { lane_index_ = idx; }

    virtual void update(float dt) = 0;
};
