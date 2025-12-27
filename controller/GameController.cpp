#include "GameController.h"
#include <SFML/Window/Event.hpp>
// controller принимает ввод и вызывает методы model



void GameController::processEvents()
{
    sf::Event event;

    // читаем все события из окна пока они есть
    while (window_.pollEvent(event))
    {
        // закрытие окна крестиком
        if (event.type == sf::Event::Closed) {
            window_.close();
        }

        // обработка нажатий клавиш
        if (event.type == sf::Event::KeyPressed)
        {
            GameState state = model_.getState();

            // если проигрыш, то даем только restart по enter
            if (state == GameState::GameOver)
            {
                if (event.key.code == sf::Keyboard::Enter)
                {
                    model_.restart();
                }

                // остальные кнопки в game over игнорируем
                return;
            }

            // управление игроком
            // поддержка и стрелок и wasd
            switch (event.key.code)
            {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                model_.movePlayerUp();
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                model_.movePlayerDown();
                break;

            case sf::Keyboard::Left:
            case sf::Keyboard::A:
                model_.movePlayerLeft();
                break;

            case sf::Keyboard::Right:
            case sf::Keyboard::D:
                model_.movePlayerRight();
                break;

            default:
                break;
            }
        }
    }
}


// обновление логики игры
// dt - время кадра в секундах
void GameController::update(float dt)
{
    model_.update(dt);
}


// отрисовка кадра через view
void GameController::render()
{
    view_.draw();
}
