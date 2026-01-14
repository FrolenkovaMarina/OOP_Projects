#pragma once
#include <iterator>

template <class Key, class T, class Hash, class KeyEqual>
class flat_hash_map<Key, T, Hash, KeyEqual>::iterator {
public:
    //Стандартный интерфейс итератора
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename flat_hash_map::value_type;
    using difference_type = std::ptrdiff_t; //Тип разности итераторов
    using pointer = value_type*;
    using reference = value_type&;

    using size_type = typename flat_hash_map::size_type;

    iterator() = default;

    reference operator*() const {
        return map_->values_[index_];
    }

    pointer operator->() const {
        return &map_->values_[index_];
    }

    // ++it
    iterator& operator++() {
        ++index_;
        skip_to_occupied();
        return *this;
    }

    // it++
    iterator operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const iterator& a, const iterator& b) { //Почему friend
        return a.map_ == b.map_ && a.index_ == b.index_;
    }

    friend bool operator!=(const iterator& a, const iterator& b) {
        return !(a == b);
    }

private:
    flat_hash_map* map_ = nullptr; //Контейнер к которому привязан итератор
    size_type index_ = 0;

    iterator(flat_hash_map* map, size_type idx)
        : map_(map), index_(idx) {
        skip_to_occupied();
    }

    void skip_to_occupied() {
        if (!map_) return;
        while (index_ < map_->capacity_ && map_->control_[index_] != flat_hash_map::Control::Occupied)
        {
            ++index_;
        }
    }

    friend class flat_hash_map;
};



//Константный
template <class Key, class T, class Hash, class KeyEqual>
class flat_hash_map<Key, T, Hash, KeyEqual>::const_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const typename flat_hash_map::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const typename flat_hash_map::value_type*;
    using reference = const typename flat_hash_map::value_type&;

    const_iterator() = default;

    const_iterator(const iterator& it)
        : map_(it.map_), index_(it.index_) {
        skip_to_occupied();
    }

    reference operator*() const {
        return map_->values_[index_];
    }

    pointer operator->() const {
        return &map_->values_[index_];
    }

    const_iterator& operator++() {
        ++index_;
        skip_to_occupied();
        return *this;
    }

    const_iterator operator++(int) {
        const_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const const_iterator& a, const const_iterator& b) {
        return a.map_ == b.map_ && a.index_ == b.index_;
    }

    friend bool operator!=(const const_iterator& a, const const_iterator& b) {
        return !(a == b);
    }

private:
    const flat_hash_map* map_ = nullptr;
    size_type index_ = 0;

    const_iterator(const flat_hash_map* map, size_type idx)
        : map_(map), index_(idx) {
        skip_to_occupied();
    }

    void skip_to_occupied() {
        if (!map_) return;
        while (index_ < map_->capacity_ && map_->control_[index_] != flat_hash_map::Control::Occupied) {
            ++index_;
        }
    }

    friend class flat_hash_map;
};


