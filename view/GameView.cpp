#include "GameView.h"
#include "../model/Config.h"
#include "../model/Player.h"
#include "../model/Lane.h"
#include "../model/RiverLane.h"
#include "../model/Log.h"
#include <cmath>
#include <iostream>


// view отвечает только за отрисовку и загрузку текстур/звуков
// вся логика игры находится в model
GameView::GameView(GameModel& model, sf::RenderWindow& window)
    : model_(model)
    , window_(window)
{
    // грузим шрифт для надписи game over
    if (!font_.loadFromFile("res/Roboto.ttf")) {
        std::cout << "the file did not load :(\n";
    }

    // настраиваем текст game over один раз, чтобы каждый кадр не пересоздавать
    game_over_text_.setFont(font_);
    game_over_text_.setString("Game Over");
    game_over_text_.setCharacterSize(64);
    game_over_text_.setFillColor(sf::Color::White);

    // ставим текст по центру экрана
    sf::FloatRect bounds = game_over_text_.getLocalBounds();
    float x = (window_.getSize().x - bounds.width) / 2.f - bounds.left;
    float y = (window_.getSize().y - bounds.height) / 2.f - bounds.top;
    game_over_text_.setPosition(x, y);

    // загрузка ресурсов
    loadCarTextures();
    loadLogTextures();
    loadHighlightTex();
    loadNatureTextures();
    loadPlayerTextures();
    loadDeathTextures();
    loadSounds();
}


// простая функция чтобы ограничить значение от 0 до 1
static float clamp01(float x)
{
    if (x < 0.f) return 0.f;
    if (x > 1.f) return 1.f;
    return x;
}


// масштаб игрока во время прыжка
// чтобы прыжок выглядел как небольшое подпрыгивание
static float jump_scale(float t)
{
    t = clamp01(t);
    return 1.f + 0.25f * std::sin(t * 3.1415926f);
}


// прозрачность тени во время прыжка
// когда игрок в воздухе тень чуть слабее
static sf::Uint8 shadow_alpha(float t)
{
    t = clamp01(t);
    float a = 90.f - 40.f * std::sin(t * 3.1415926f);
    if (a < 0.f) a = 0.f;
    if (a > 255.f) a = 255.f;
    return static_cast<sf::Uint8>(a);
}


// масштаб тени во время прыжка
// тень становится меньше когда игрок выше
static float shadow_scale_mul(float t)
{
    t = clamp01(t);
    return 1.f - 0.35f * std::sin(t * 3.1415926f);
}


// основной рендер кадра
void GameView::draw()
{
    // чистим экран
    window_.clear(sf::Color::Black);

    // включаем или выключаем звук машин
    // он должен играть только когда реально есть дорожные полосы на экране
    if (sounds_loaded_)
    {
        bool any_road_on_screen = false;

        const auto& lanes = model_.getLanes();
        for (const auto& lanePtr : lanes)
        {
            if (dynamic_cast<RoadLane*>(lanePtr.get()))
            {
                any_road_on_screen = true;
                break;
            }
        }

        bool should_play_cars =
            (model_.getState() == GameState::Running) &&
            any_road_on_screen;

        if (should_play_cars)
        {
            if (!cars_loop_playing_)
            {
                cars_loop_music_.play();
                cars_loop_playing_ = true;
            }
        }
        else
        {
            if (cars_loop_playing_)
            {
                cars_loop_music_.stop();
                cars_loop_playing_ = false;
            }
        }
    }

    // время кадра для анимации прыжка
    float adt = jump_clock_.restart().asSeconds();

    // jump serial меняется когда модель сообщает что был прыжок
    // так view понимает что надо запустить анимацию
    int js = model_.getJumpSerial();
    if (js != prev_jump_serial_)
    {
        prev_jump_serial_ = js;
        jump_active_ = true;
        jump_t_ = 0.f;

        // звук прыжка
        if (sounds_loaded_)
        {
            jump_music_.stop();
            jump_music_.setPlayingOffset(sf::Time::Zero);
            jump_music_.play();
        }
    }

    // обновляем прогресс прыжка 0..1
    if (jump_active_)
    {
        jump_t_ += adt / jump_duration_;
        if (jump_t_ >= 1.f)
        {
            jump_t_ = 1.f;
            jump_active_ = false;
        }
    }

    const auto& lanes = model_.getLanes();
    float cell_offset = model_.getWorldOffset();
    float tile = static_cast<float>(Config::TILE_SIZE);

    // сначала рисуем все полосы фона
    for (std::size_t i = 0; i < lanes.size(); ++i)
    {
        // y считается с учетом world offset чтобы был эффект движения мира
        float y = (static_cast<float>(i) - 1.f + cell_offset) * tile;

        sf::RectangleShape rect;
        rect.setSize({ Config::GRID_WIDTH * tile, tile });
        rect.setPosition(0.f, y);

        auto* roadLane = dynamic_cast<RoadLane*>(lanes[i].get());
        auto* riverLane = dynamic_cast<RiverLane*>(lanes[i].get());

        if (roadLane)
        {
            // дорога
            rect.setFillColor(sf::Color(60, 60, 60));
            window_.draw(rect);

            // полоски на дороге
            float stripe_h = tile / 15.f;
            float stripe_w = tile / 2.f;
            float gap = tile / 1.5f;
            float y_stripe = y + tile / 2.f - stripe_h / 2.f;

            for (float x_stripe = 0.f; x_stripe < Config::GRID_WIDTH * tile; x_stripe += stripe_w + gap)
            {
                sf::RectangleShape stripe;
                stripe.setSize({ stripe_w, stripe_h });
                stripe.setPosition(x_stripe, y_stripe);
                stripe.setFillColor(sf::Color(190, 190, 190));
                window_.draw(stripe);
            }
        }
        else if (riverLane)
        {
            // вода
            float tile_size = tile;
            float lane_width = Config::GRID_WIDTH * tile_size;

            sf::RectangleShape base_water;
            base_water.setSize({ lane_width, tile_size });
            base_water.setPosition(0.f, y);
            base_water.setFillColor(sf::Color(70, 160, 240));
            window_.draw(base_water);

            // затемнение по краям чтобы вода выглядела объемнее
            float border = 2.5f * tile_size;
            float border_right = lane_width - border;

            sf::RectangleShape dark_left;
            dark_left.setPosition(0.f, y);
            dark_left.setSize({ border, tile_size });
            dark_left.setFillColor(sf::Color(50, 140, 220));
            window_.draw(dark_left);

            sf::RectangleShape dark_right;
            dark_right.setPosition(border_right, y);
            dark_right.setSize({ border, tile_size });
            dark_right.setFillColor(sf::Color(50, 140, 220));
            window_.draw(dark_right);

            // блики на воде если текстура загружена
            if (water_highlight_loaded_)
            {
                sf::Sprite L, R;
                L.setTexture(water_highlight_texture_);
                R.setTexture(water_highlight_texture_);

                float tex_w = (float)water_highlight_texture_.getSize().x;
                float tex_h = (float)water_highlight_texture_.getSize().y;

                float target_h = tile_size * 0.90f;
                float scale = target_h / tex_h;

                float scaled_w = tex_w * scale;
                float scaled_h = tex_h * scale;

                float base_center_left = border;
                float base_center_right = border_right;

                float y_pos = y + (tile_size - scaled_h) / 2.f;

                // простая анимация бликов синусом
                float t = anim_clock_.getElapsedTime().asSeconds();

                float wobble_amp = tile_size * 0.04f;
                float wobble = std::sin(t * 3.0f) * wobble_amp;

                float center_left = base_center_left + wobble;
                float center_right = base_center_right - wobble;

                L.setScale(scale, scale);
                L.setPosition(center_left - scaled_w * 0.5f, y_pos);

                // правый блик зеркалим по x через отрицательный scale
                R.setTexture(water_highlight_texture_);
                R.setScale(-scale, scale);
                R.setOrigin(tex_w, 0.f);

                float right_x = center_right - scaled_w * 0.5f;
                R.setPosition(right_x, y_pos);

                window_.draw(L);
                window_.draw(R);
            }
        }
        else
        {
            // трава
            int lane_index = lanes[i]->getIndex();
            if (lane_index % 2 == 0)
                rect.setFillColor(sf::Color(60, 160, 60));
            else
                rect.setFillColor(sf::Color(30, 120, 30));

            window_.draw(rect);

            // рисуем препятствия на траве
            auto* grass = dynamic_cast<GrassLane*>(lanes[i].get());
            if (grass)
            {
                float y_lane = y;

                for (const auto& o : grass->getObstacles())
                {
                    float x = static_cast<float>(o.col) * tile;

                    // если есть текстуры то рисуем ими, иначе просто зеленый квадратик
                    if (nature_textures_loaded_)
                    {
                        const sf::Texture* tex = nullptr;

                        if (o.type == GrassLane::ObstacleType::Tree)
                            tex = &tree_textures_[o.variant % 2];
                        else if (o.type == GrassLane::ObstacleType::Bush)
                            tex = &bush_texture_;
                        else if (o.type == GrassLane::ObstacleType::Rock)
                            tex = &rock_textures_[o.variant % 2];

                        if (tex)
                        {
                            sf::Sprite s;
                            s.setTexture(*tex);

                            float tex_w = static_cast<float>(tex->getSize().x);
                            float tex_h = static_cast<float>(tex->getSize().y);

                            // подгоняем текстуру под размер клетки
                            s.setScale(tile / tex_w, tile / tex_h);
                            s.setPosition(x, y_lane);

                            window_.draw(s);
                        }
                    }
                    else
                    {
                        sf::RectangleShape r;
                        r.setSize({ tile, tile });
                        r.setPosition(x, y_lane);
                        r.setFillColor(sf::Color(20, 80, 20));
                        window_.draw(r);
                    }
                }
            }
        }
    }

    // отдельно рисуем машины поверх дороги
    for (std::size_t i = 0; i < lanes.size(); ++i)
    {
        auto* roadLane = dynamic_cast<RoadLane*>(lanes[i].get());
        if (!roadLane)
            continue;

        float y_lane = (static_cast<float>(i) - 1.f + cell_offset) * tile;

        for (const auto& carPtr : roadLane->getCars())
        {
            const Car& car = *carPtr;

            float car_cell_x = car.getCellX();
            float car_length_cells = car.getWidthCells();

            float x_car = car_cell_x * tile;
            float y_car = y_lane;

            if (car_textures_loaded_)
            {
                // выбираем маленькую или большую машину по длине
                bool is_small = (car_length_cells <= 2.5f);
                int variant = car.getSpriteVariant();

                const sf::Texture* tex = nullptr;
                if (is_small)
                    tex = &small_car_textures_[variant % small_car_textures_.size()];
                else
                    tex = &big_car_textures_[variant % big_car_textures_.size()];

                sf::Sprite sprite;
                sprite.setTexture(*tex);
                float tex_width = static_cast<float>(tex->getSize().x);
                float tex_height = static_cast<float>(tex->getSize().y);

                // растягиваем картинку чтобы она заняла нужное число клеток
                float target_width = car_length_cells * tile;
                float target_height = tile;

                float scale_x = target_width / tex_width;
                float scale_y = target_height / tex_height;

                // если направление влево то зеркалим
                if (car.getDirection() < 0)
                {
                    sprite.setScale(-scale_x, scale_y);
                    sprite.setPosition(x_car + target_width, y_car);
                }
                else
                {
                    sprite.setScale(scale_x, scale_y);
                    sprite.setPosition(x_car, y_car);
                }

                window_.draw(sprite);
            }
            else
            {
                // запасной вариант если текстур нет
                sf::RectangleShape rect;
                rect.setSize({ car_length_cells * tile, tile });
                rect.setPosition(x_car, y_car);
                rect.setFillColor(sf::Color::Red);
                window_.draw(rect);
            }
        }
    }

    // отдельно рисуем бревна поверх воды
    for (std::size_t i = 0; i < lanes.size(); ++i)
    {
        auto* river = dynamic_cast<RiverLane*>(lanes[i].get());
        if (!river)
            continue;

        float y_lane = (static_cast<float>(i) - 1.0f + cell_offset) * tile;

        const auto& logs = river->getLogs();
        for (const auto& logPtr : logs)
        {
            const Log& log = *logPtr;

            float len_cells = log.getWidthCells();
            float x_cells = log.getCellX();

            float x_log = x_cells * tile;
            float w_log = len_cells * tile;

            if (log_textures_loaded_)
            {
                const sf::Texture* tex = nullptr;
                int variant = log.getSpriteVariant();

                // короткое бревно или длинное
                if (len_cells <= 2.5f)
                    tex = &log2_texture_;
                else
                    tex = &log3_textures_[variant % log3_textures_.size()];

                sf::Sprite sprite;
                sprite.setTexture(*tex);

                float tex_w = static_cast<float>(tex->getSize().x);
                float tex_h = static_cast<float>(tex->getSize().y);

                float scale_x = w_log / tex_w;
                float scale_y = tile / tex_h;

                int dir = log.getDirection();

                // если едет влево зеркалим
                if (dir < 0)
                {
                    sprite.setScale(-scale_x, scale_y);
                    sprite.setPosition(x_log + w_log, y_lane);
                }
                else
                {
                    sprite.setScale(scale_x, scale_y);
                    sprite.setPosition(x_log, y_lane);
                }

                window_.draw(sprite);
            }
            else
            {
                // запасной вариант если текстур нет
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(w_log, tile));
                rect.setPosition(x_log, y_lane);
                rect.setFillColor(sf::Color(139, 69, 19));
                window_.draw(rect);
            }
        }
    }

    // координаты игрока
    const Player& p = model_.getPlayer();

    // если игрок стоит на бревне, то берем абсолютную позицию чтобы он ехал вместе с ним
    float center_x = p.isOnLog()
        ? p.getAbsX()
        : static_cast<float>(p.getCol()) + 0.5f;

    float x_player = (center_x - 0.5f) * tile;
    float y_player = (p.getRow() + cell_offset) * tile;

    float cx = x_player + tile * 0.5f;
    float cy = y_player + tile * 0.5f;

    // текущая фаза прыжка 0..1
    float jt = jump_active_ ? jump_t_ : 0.f;

    if (player_textures_loaded_)
    {
        // при утоплении тень не нужна, выглядит странно
        bool hide_shadow =
            (model_.getState() == GameState::GameOver) &&
            (model_.getDeathType() == DeathType::Drown);

        if (!hide_shadow)
        {
            // рисуем тень первой, чтобы игрок был сверху
            sf::Sprite sh;
            sh.setTexture(player_shadow_texture_);

            float tex_w = static_cast<float>(player_shadow_texture_.getSize().x);
            float tex_h = static_cast<float>(player_shadow_texture_.getSize().y);

            sh.setOrigin(tex_w * 0.5f, tex_h * 0.5f);

            float mul = shadow_scale_mul(jt);
            float base_x = tile / tex_w;
            float base_y = tile / tex_h;

            sh.setScale(base_x * mul, base_y * mul);
            sh.setColor(sf::Color(255, 255, 255, shadow_alpha(jt)));

            // тень поворачиваем так же как игрок
            float shadow_rotation_deg = 0.f;
            switch (model_.getLastJumpDir())
            {
            case JumpDir::Up:    shadow_rotation_deg = 0.f;   break;
            case JumpDir::Right: shadow_rotation_deg = 90.f;  break;
            case JumpDir::Down:  shadow_rotation_deg = 180.f; break;
            case JumpDir::Left:  shadow_rotation_deg = -90.f; break;
            default: break;
            }
            sh.setRotation(shadow_rotation_deg);

            // небольшое смещение тени чтобы был эффект объема
            float dx = 0.f;
            float dy = tile * 0.18f;

            switch (model_.getLastJumpDir())
            {
            case JumpDir::Left:
                dx = -tile * 0.18f;
                dy = tile * 0.12f;
                break;

            case JumpDir::Right:
                dx = tile * 0.18f;
                dy = tile * 0.12f;
                break;

            case JumpDir::Down:
                dx = 0.f;
                dy = tile * 0.22f;
                break;

            case JumpDir::Up:
            default:
                dx = 0.f;
                dy = tile * 0.18f;
                break;
            }

            sh.setPosition(cx + dx, cy + dy);
            window_.draw(sh);
        }

        {
            // рисуем игрока или спрайт смерти
            bool is_game_over = (model_.getState() == GameState::GameOver);

            if (is_game_over && death_textures_loaded_)
            {
                const sf::Texture* tex = nullptr;

                switch (model_.getDeathType())
                {
                case DeathType::Drown: tex = &death_drown_texture_; break;
                default: tex = nullptr; break;
                }

                if (tex)
                {
                    // спрайт смерти
                    sf::Sprite pl;
                    pl.setTexture(*tex);

                    float tex_w = (float)tex->getSize().x;
                    float tex_h = (float)tex->getSize().y;
                    pl.setOrigin(tex_w * 0.5f, tex_h * 0.5f);

                    float base_x = tile / tex_w;
                    float base_y = tile / tex_h;

                    pl.setScale(base_x, base_y);
                    pl.setPosition(cx, cy);

                    window_.draw(pl);
                }
                else
                {
                    // если нет текстуры смерти то рисуем обычного игрока
                    float rotation_deg = 0.f;
                    switch (model_.getLastJumpDir())
                    {
                    case JumpDir::Up:    rotation_deg = 0.f;   break;
                    case JumpDir::Right: rotation_deg = 90.f;  break;
                    case JumpDir::Down:  rotation_deg = 180.f; break;
                    case JumpDir::Left:  rotation_deg = -90.f; break;
                    default: break;
                    }

                    sf::Sprite pl;
                    pl.setTexture(player_texture_);

                    float tex_w = (float)player_texture_.getSize().x;
                    float tex_h = (float)player_texture_.getSize().y;
                    pl.setOrigin(tex_w * 0.5f, tex_h * 0.5f);

                    float base_x = tile / tex_w;
                    float base_y = tile / tex_h;

                    float mul = jump_active_ ? jump_scale(jt) : 1.f;
                    pl.setScale(base_x * mul, base_y * mul);

                    pl.setRotation(rotation_deg);
                    pl.setPosition(cx, cy);

                    window_.draw(pl);
                }
            }
            else
            {
                // обычный игрок когда игра идет
                float rotation_deg = 0.f;
                switch (model_.getLastJumpDir())
                {
                case JumpDir::Up:    rotation_deg = 0.f;   break;
                case JumpDir::Right: rotation_deg = 90.f;  break;
                case JumpDir::Down:  rotation_deg = 180.f; break;
                case JumpDir::Left:  rotation_deg = -90.f; break;
                default: break;
                }

                sf::Sprite pl;
                pl.setTexture(player_texture_);

                float tex_w = (float)player_texture_.getSize().x;
                float tex_h = (float)player_texture_.getSize().y;
                pl.setOrigin(tex_w * 0.5f, tex_h * 0.5f);

                float base_x = tile / tex_w;
                float base_y = tile / tex_h;

                float mul = jump_active_ ? jump_scale(jt) : 1.f;
                pl.setScale(base_x * mul, base_y * mul);

                pl.setRotation(rotation_deg);
                pl.setPosition(cx, cy);

                window_.draw(pl);
            }
        }
    }
    else
    {
        // если нет текстуры игрока рисуем желтый квадрат
        sf::RectangleShape rect_player;
        rect_player.setSize({ tile, tile });
        rect_player.setPosition(x_player, y_player);
        rect_player.setFillColor(sf::Color::Yellow);
        window_.draw(rect_player);
    }

    // затемнение экрана и надпись при проигрыше
    if (model_.getState() == GameState::GameOver)
    {
        sf::RectangleShape overlay;
        overlay.setSize({ Config::GRID_WIDTH * tile, Config::GRID_HEIGHT * tile });
        overlay.setPosition(0.f, 0.f);
        overlay.setFillColor(sf::Color(0, 0, 0, 150));

        window_.draw(overlay);
        window_.draw(game_over_text_);
    }

    // показываем кадр
    window_.display();
}


// загрузка текстур машин
void GameView::loadCarTextures()
{
    car_textures_loaded_ = true;

    // маленькие машины
    for (int i = 0; i < 5; ++i)
    {
        std::string filename = "res/car_small_" + std::to_string(i) + ".png";
        if (!small_car_textures_[i].loadFromFile(filename))
            car_textures_loaded_ = false;
    }

    // большие машины
    for (int i = 0; i < 3; ++i)
    {
        std::string filename = "res/car_big_" + std::to_string(i) + ".png";
        if (!big_car_textures_[i].loadFromFile(filename))
            car_textures_loaded_ = false;
    }
}


// загрузка текстур бревен
void GameView::loadLogTextures()
{
    log_textures_loaded_ = true;

    if (!log2_texture_.loadFromFile("res/log_2_0.png"))
        log_textures_loaded_ = false;

    if (!log3_textures_[0].loadFromFile("res/log_3_0.png"))
        log_textures_loaded_ = false;
    if (!log3_textures_[1].loadFromFile("res/log_3_1.png"))
        log_textures_loaded_ = false;
}


// загрузка бликов на воде
void GameView::loadHighlightTex()
{
    if (!water_highlight_texture_.loadFromFile("res/water_highlight.png")) {
        water_highlight_loaded_ = false;
    }
    else {
        water_highlight_loaded_ = true;
    }
}


// маленький помощник чтобы не писать одно и то же
static bool loadTex(sf::Texture& t, const std::string& path)
{
    if (t.loadFromFile(path)) return true;

    std::cerr << "[Texture] FAILED: " << path << "\n";
    return false;
}


// загрузка деревьев/кустов/камней
void GameView::loadNatureTextures()
{
    nature_textures_loaded_ = true;

    nature_textures_loaded_ = true;

    if (!loadTex(tree_textures_[0], "res/tree_0.png")) nature_textures_loaded_ = false;
    if (!loadTex(tree_textures_[1], "res/tree_1.png")) nature_textures_loaded_ = false;
    if (!loadTex(bush_texture_, "res/bush_0.png")) nature_textures_loaded_ = false;
    if (!loadTex(rock_textures_[0], "res/rock_0.png")) nature_textures_loaded_ = false;
    if (!loadTex(rock_textures_[1], "res/rock_1.png")) nature_textures_loaded_ = false;

    // если что то не загрузилось, выведем в консоль
    if (!nature_textures_loaded_)
        std::cerr << "[Texture] Nature textures NOT loaded.\n";
}


// загрузка игрока и его тени
void GameView::loadPlayerTextures()
{
    player_textures_loaded_ = true;

    if (!player_texture_.loadFromFile("res/player.png"))
        player_textures_loaded_ = false;

    if (!player_shadow_texture_.loadFromFile("res/player_shadow.png"))
        player_textures_loaded_ = false;
}


// загрузка текстур смерти
void GameView::loadDeathTextures()
{
    death_textures_loaded_ = true;

    if (!death_drown_texture_.loadFromFile("res/player_dead_drown.png"))
        death_textures_loaded_ = false;

    if (!death_hit_front_texture_.loadFromFile("res/player_dead_front.png"))
        death_textures_loaded_ = false;

    if (!death_hit_side_texture_.loadFromFile("res/player_dead_side.png"))
        death_textures_loaded_ = false;
}


// загрузка звуков
void GameView::loadSounds()
{
    sounds_loaded_ = true;

    if (!jump_music_.openFromFile("res/jump.mp3"))
        sounds_loaded_ = false;

    if (!cars_loop_music_.openFromFile("res/cars_loop.mp3"))
        sounds_loaded_ = false;

    if (!sounds_loaded_)
        return;

    // прыжок проигрывается один раз
    jump_music_.setLoop(false);
    jump_music_.setVolume(70.f);

    // машины идут циклом пока есть дороги
    cars_loop_music_.setLoop(true);
    cars_loop_music_.setVolume(35.f);
}
