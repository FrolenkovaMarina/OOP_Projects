#pragma once

#include <vector>
#include <memory>

#include "Lane.h"
#include "Log.h"
#include "Config.h"

class RiverLane : public Lane {
public:
    explicit RiverLane(int index);

    void update(float dt) override;

    const std::vector<std::unique_ptr<Log>>& getLogs() const { return logs_; }

private:
    std::vector<std::unique_ptr<Log>> logs_;

    float base_speed_;
    int direction_;

    void initLogs();
};
