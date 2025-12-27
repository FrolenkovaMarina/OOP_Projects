#pragma once

#include <vector>
#include <cstdlib>  
#include "Lane.h"
#include "Config.h"


class GrassLane : public Lane
{
public:

    enum class ObstacleType {
        Tree,
        Bush,
        Rock,
        None
    };

    struct Obstacle {
        int col = 0;
        ObstacleType type = ObstacleType::None;
        int variant = 0;
    };

public:
  
    GrassLane(int index, const std::vector<int>& open_cols)
        : Lane(index)
    {
        generateObstacles(open_cols);
    }



    void update(float) override
    {
    }

    const std::vector<Obstacle>& getObstacles() const
    {
        return obstacles_;
    }

    bool isBlocked(int col) const
    {
        for (const auto& o : obstacles_) {
            if (o.col == col)
                return true;
        }
        return false;
    }

    void clearObstacleAt(int col)
    {
        for (std::size_t i = 0; i < obstacles_.size(); )
        {
            if (obstacles_[i].col == col)
                obstacles_.erase(obstacles_.begin() + i);
            else
                ++i;
        }
    }

private:
    std::vector<Obstacle> obstacles_;

private:
    void generateObstacles(const std::vector<int>& open_cols)
    {
        obstacles_.clear();

        const int W = Config::GRID_WIDTH;

        const int P_ANY = 60;
        const int P_TREE = 80;
        const int P_BUSH = 12;
        const int P_ROCK = 8;

        for (int c = 0; c < W; ++c)
        {
            bool open = false;
            for (int oc : open_cols)
            {
                if (oc == c)
                {
                    open = true;
                    break;
                }
            }

            if (open)
                continue;

            int r = std::rand() % 100;
            if (r >= P_ANY)
                continue;

            int t = std::rand() % 100;

            Obstacle o;
            o.col = c;

            if (t < P_TREE)
            {
                o.type = ObstacleType::Tree;
                o.variant = std::rand() % 2;
            }
            else if (t < P_TREE + P_BUSH)
            {
                o.type = ObstacleType::Bush;
                o.variant = 0;
            }
            else
            {
                o.type = ObstacleType::Rock;
                o.variant = std::rand() % 2;
            }

            obstacles_.push_back(o);
        }

    }
};
