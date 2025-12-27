#pragma once

#include <memory>
#include <vector>

#include "SimpleFactory.h"
#include "Lane.h"
#include "GrassLane.h" 
#include "RoadLane.h"
#include "RiverLane.h"


// тип полос
enum class LaneId {
    Grass,
    Road,
    River
};

 
// фабрика для создания полос
// прячет логику создания разных типов lane
class LaneFactory {
public:

    using Creator = std::unique_ptr<Lane>(*)(int, const std::vector<int>*);

    // в конструкторе регистрируем все типы полос
    LaneFactory() {
        factory_.registerType(LaneId::Road, &LaneFactory::createRoad);
        factory_.registerType(LaneId::River, &LaneFactory::createRiver);
        factory_.registerType(LaneId::Grass, &LaneFactory::createGrass);
    }

    // создание полосы по id
    // для grass передаются открытые колонки
    std::unique_ptr<Lane> create(
        LaneId id,
        int index,
        const std::vector<int>* open_cols = nullptr
    ) const {
        return factory_.create(id, index, open_cols);
    }

private:
    static std::unique_ptr<Lane> createRoad(int index, const std::vector<int>*) {
        return std::make_unique<RoadLane>(index);
    }

    static std::unique_ptr<Lane> createRiver(int index, const std::vector<int>*) {
        return std::make_unique<RiverLane>(index);
    }

    static std::unique_ptr<Lane> createGrass(int index, const std::vector<int>* open_cols) {
        if (!open_cols) return nullptr;
        return std::make_unique<GrassLane>(index, *open_cols);
    }

private:
    SimpleFactory<Lane, LaneId, Creator> factory_;
};
