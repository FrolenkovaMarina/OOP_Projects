#pragma once

#include <SFML/Graphics.hpp>
#include "../model/GameModel.h"
#include "../view/GameView.h"

class GameController {
public:
    GameController(GameModel& model,
        GameView& view,
        sf::RenderWindow& window)
        : model_(model)
        , view_(view)
        , window_(window)
    {
    }

    void processEvents();

    void update(float dt);


    void render();

private:
    GameModel& model_;          
    GameView& view_;            
    sf::RenderWindow& window_; 
};
