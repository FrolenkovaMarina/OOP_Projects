#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <SFML/Audio.hpp>

#include "../model/GameModel.h"
#include "../model/RoadLane.h"
#include "../model/Car.h"
#include "../model/RiverLane.h"
#include "../model/Log.h"
#include "../model/GrassLane.h"


class GameView {
public:
    GameView(GameModel& model, sf::RenderWindow& window);

    // рисование всего кадра
    void draw();

    // для анимации воды
    sf::Clock anim_clock_;

private:
    // ссылки на model и окно sfml
    GameModel& model_;
    sf::RenderWindow& window_;

    // шрифт и текст для game over
    sf::Font font_;
    sf::Text game_over_text_;

    // текстуры машин
    std::array<sf::Texture, 5> small_car_textures_;
    std::array<sf::Texture, 3> big_car_textures_;
    bool car_textures_loaded_ = false;

    // текстуры бревен
    sf::Texture                log2_texture_;
    std::array<sf::Texture, 2> log3_textures_;
    bool log_textures_loaded_ = false;

    // текстура блика на воде
    sf::Texture water_highlight_texture_;
    bool water_highlight_loaded_ = false;

    // загрузка текстур машин
    void loadCarTextures();

    void loadLogTextures();

    void loadHighlightTex();

    // текстуры природы для травы
    std::array<sf::Texture, 2> tree_textures_;
    sf::Texture bush_texture_;
    std::array<sf::Texture, 2> rock_textures_;
    bool nature_textures_loaded_ = false;

    void loadNatureTextures();

    // текстуры игрока
    sf::Texture player_texture_;
    sf::Texture player_shadow_texture_;
    bool player_textures_loaded_ = false;

    // звуки
    sf::Music jump_music_;
    sf::Music cars_loop_music_;
    bool sounds_loaded_ = false;
    bool cars_loop_playing_ = false;

    void loadSounds();

    // текстуры смерти
    sf::Texture death_drown_texture_;
    sf::Texture death_hit_front_texture_;
    sf::Texture death_hit_side_texture_;
    bool death_textures_loaded_ = false;

    // загрузка текстур смерти
    void loadDeathTextures();

    // старые координаты игрока, если надо отследить перемещение
    int prev_player_row_ = -9999;
    int prev_player_col_ = -9999;

    // анимация прыжка
    bool  jump_active_ = false;
    float jump_t_ = 0.f;
    float jump_duration_ = 0.14f;
    sf::Clock jump_clock_;
    int prev_jump_serial_ = 0;

    void loadPlayerTextures();
};
