#pragma once

#include <map>
#include <memory>
#include <vector>


// хранит соответствие id и функций создания объектов
template<class AbstractProduct, class IdentifierType, class ProductCreator>
class SimpleFactory {
public:
    using ProductPtr = std::unique_ptr<AbstractProduct>;

    bool registerType(const IdentifierType& id, ProductCreator creator) {
        creators_[id] = creator;
        return true;
    }

    // создание объекта по id
    ProductPtr create(const IdentifierType& id, int index, const std::vector<int>* open_cols) const {
        auto it = creators_.find(id);
        if (it == creators_.end())
            return nullptr;

        return (it->second)(index, open_cols);
    }

private:
    // таблица соответствия id и функций создания
    std::map<IdentifierType, ProductCreator> creators_;
};
