#include <ctime>
#include <cstdlib>
#include <SFML/Graphics.hpp>

#include "model/Config.h"
#include "model/GameModel.h"
#include "view/GameView.h"
#include "controller/GameController.h"


int main()
{
    // задаем seed для rand, генерация разная при каждом запуске
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    unsigned int window_width =
        static_cast<unsigned int>(Config::GRID_WIDTH * Config::TILE_SIZE);
    unsigned int window_height =
        static_cast<unsigned int>(Config::GRID_HEIGHT * Config::TILE_SIZE);

    sf::RenderWindow window(
        sf::VideoMode(window_width, window_height),
        "Crossy Road",
        sf::Style::Close
    );

    GameModel model;
    GameView view(model, window);
    GameController controller(model, view, window);

    // таймер для dt
    sf::Clock clock;

    while (window.isOpen())
    {
        // dt время кадра в секундах
        float dt = clock.restart().asSeconds();

        controller.processEvents();

        controller.update(dt);

        // отрисовка
        controller.render();
    }

    return 0;
}
