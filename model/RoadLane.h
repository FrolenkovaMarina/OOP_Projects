#pragma once

#include <vector>
#include <memory>

#include "Lane.h"
#include "Car.h"
#include "Config.h"

class RoadLane : public Lane {
public:
    explicit RoadLane(int index);

    void update(float dt) override;

    const std::vector<std::unique_ptr<Car>>& getCars() const { return cars_; }
    std::vector<std::unique_ptr<Car>>& getCars() { return cars_; }

private:
    std::vector<std::unique_ptr<Car>> cars_;

    float base_speed_;
    int   direction_;

    void initCars();
};
