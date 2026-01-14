#pragma once

#include <vector>
#include <utility>      // std::pair
#include <functional>   // std::hash, std::equal_to
#include <initializer_list>
#include <stdexcept>    // std::out_of_range
#include <cstddef>      // std::size_t
#include <cstdint>      // std::uint8_t
#include <emmintrin.h>   // SSE2

#include <intrin.h>



// По задаче const key в value_type но вроде это не будет работать


template <
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>
>
class flat_hash_map
{
public:

    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using size_type = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;


    class iterator;
    class const_iterator;




    //================================================Конструкторы, деструктор============================

    flat_hash_map()
        : hasher_(Hash()), key_equal_(KeyEqual()) {
        init_storage(0);
    }

    // Контейнер с заданным количеством бакетов
    explicit flat_hash_map(size_type bucket_count, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
        : hasher_(hash)
        , key_equal_(equal) {

        init_storage(bucket_count);
    }


    // Шаблонный конструктор из диапазона элементов [first, last)
    template <class InputIt>
    flat_hash_map(InputIt first, InputIt last,
        size_type bucket_count = 0,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual())
        : hasher_(hash), key_equal_(equal) {
        init_storage(bucket_count);
        insert(first, last);
    }


    // Из списка инициализации
    flat_hash_map(std::initializer_list<value_type> init, size_type bucket_count = 0,
        const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
        : hasher_(hash)
        , key_equal_(equal)
    {
        init_storage(bucket_count);
        insert(init);
    }

    //Копирующий конструктор
    flat_hash_map(const flat_hash_map& other)
        : values_(other.values_)
        , control_(other.control_)
        , size_(other.size_)
        , capacity_(other.capacity_)
        , hasher_(other.hasher_)
        , key_equal_(other.key_equal_)
        , max_load_factor_(other.max_load_factor_)
    {
    }

    //Перемещающий конструктор
    flat_hash_map(flat_hash_map&& other) noexcept
        : values_(std::move(other.values_))
        , control_(std::move(other.control_))
        , size_(other.size_)
        , capacity_(other.capacity_)
        , hasher_(std::move(other.hasher_))
        , key_equal_(std::move(other.key_equal_))
        , max_load_factor_(other.max_load_factor_)
    {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~flat_hash_map() = default;


    //===============================================Операторы присваивания===================


    // Копирующее присваивание 
    flat_hash_map& operator=(const flat_hash_map& other) {
        if (this == &other) {
            return *this;
        }

        values_ = other.values_;
        control_ = other.control_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        hasher_ = other.hasher_;
        key_equal_ = other.key_equal_;
        max_load_factor_ = other.max_load_factor_;

        return *this;
    }

    //Перемещающее присваивание
    flat_hash_map& operator=(flat_hash_map&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        values_ = std::move(other.values_);
        control_ = std::move(other.control_);
        size_ = other.size_;
        capacity_ = other.capacity_;

        hasher_ = std::move(other.hasher_);
        key_equal_ = std::move(other.key_equal_);
        max_load_factor_ = other.max_load_factor_;

        other.size_ = 0;
        other.capacity_ = 0;

        return *this;
    }

    //Третий это для таких конструкций (список инициализации)
    //flat_hash_map<int, int> mp;
    //mp = { {1, 10}, {2, 20}, {3, 30} };

    flat_hash_map& operator=(std::initializer_list<value_type> init) {
        clear();
        insert(init);
        return *this;
    }



    void swap(flat_hash_map& other) noexcept {
        using std::swap;
        swap(values_, other.values_);
        swap(control_, other.control_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(hasher_, other.hasher_);
        swap(key_equal_, other.key_equal_);
        swap(max_load_factor_, other.max_load_factor_);
    }




    //===================================Итераторы==========================


    iterator begin() noexcept {
        return make_iterator(0);
    }

    const_iterator begin() const noexcept {
        return make_const_iterator(0);
    }

    const_iterator cbegin() const noexcept {
        return make_const_iterator(0);
    }

    iterator end() noexcept {
        return make_iterator(capacity_);
    }

    const_iterator end() const noexcept {
        return make_const_iterator(capacity_);
    }

    const_iterator cend() const noexcept {
        return make_const_iterator(capacity_);
    }




    iterator erase(iterator pos) {
        if (pos.map_ != this) {
            return end();
        }

        if (pos.index_ >= capacity_) {
            return end();
        }

        if (control_[pos.index_] != Control::Occupied) {
            return iterator(this, pos.index_ + 1);
        }

        control_[pos.index_] = Control::Deleted;
        --size_;

        return make_iterator(pos.index_ + 1);
    }





    //======================Мини функции==================
    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    void clear() noexcept {
        if (capacity_ == 0) { size_ = 0; return; }

        for (size_type i = 0; i < capacity_; ++i) {
            control_[i] = Control::Empty;
        }
        size_ = 0;
    }






    //============================Вставка элементов==============================


    std::pair<iterator, bool> insert(const value_type& value) {
        const key_type& key = value.first;
        rehash_if_needed(1);

        size_type idx = find_bucket_for_insert(key);
        if (idx == capacity_) {
            return { end(), false };
        }

        if (control_[idx] == Control::Occupied &&
            key_equal_(values_[idx].first, key))
        {
            return { make_iterator(idx), false };
        }

        control_[idx] = Control::Occupied;
        values_[idx] = value;
        ++size_;

        return { make_iterator(idx), true };
    }


    std::pair<iterator, bool> insert(value_type&& value) {
        const key_type& key = value.first;

        rehash_if_needed(1);

        size_type idx = find_bucket_for_insert(key);
        if (idx == capacity_) {
            return { end(), false };
        }

        if (control_[idx] == Control::Occupied &&
            key_equal_(values_[idx].first, key))
        {
            return { make_iterator(idx), false };
        }

        control_[idx] = Control::Occupied;
        values_[idx] = std::move(value);
        ++size_;

        return { make_iterator(idx), true };
    }

    //Вставка диапазона, шаблон потому что итераторы бывают разные
    template <class InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    void insert(std::initializer_list<value_type> init) {
        insert(init.begin(), init.end());
    }


    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type v(std::forward<Args>(args)...);
        return insert(std::move(v));
    }


    size_type erase(const key_type& key) {
        size_type idx = find_bucket_for_key(key);
        if (idx == capacity_) {
            return 0;
        }

        control_[idx] = Control::Deleted;
        --size_;
        return 1;
    }



    //======================reserve и load factor


    void reserve(size_type n) {
        if (n == 0) return;

        if (max_load_factor_ <= 0.0f) {
            if (n > capacity_) rehash_to(n);
            return;
        }

        // buckets = ceil(n / mlf)
        const float need_f = static_cast<float>(n) / max_load_factor_;
        const size_type need = static_cast<size_type>(need_f) + 1;

        if (need > capacity_) {
            rehash_to(need);
        }
    }




    float load_factor() const noexcept {
        if (capacity_ == 0) {
            return 0.0f;
        }
        return static_cast<float>(size_) / static_cast<float>(capacity_);
    }

    void max_load_factor(float ml) {
        max_load_factor_ = ml;
    }

    float max_load_factor() const noexcept {
        return max_load_factor_;
    }







    //============================================Доступ по ключу=================
    mapped_type& operator[](const key_type& key) {
        rehash_if_needed(1);

        size_type idx = find_bucket_for_insert(key);
        if (idx == capacity_) {
            rehash_if_needed(1);
            idx = find_bucket_for_insert(key);
            if (idx == capacity_) {
                throw std::runtime_error("[ERROR] flat_hash_map::operator[]: no space");
            }
        }

        if (control_[idx] == Control::Occupied &&
            key_equal_(values_[idx].first, key))
        {
            return values_[idx].second;
        }

        // Вставляем новый элемент с default-значением
        control_[idx] = Control::Occupied;
        values_[idx] = value_type(key, mapped_type{});
        ++size_;

        return values_[idx].second;
    }


    mapped_type& operator[](key_type&& key) {
        rehash_if_needed(1);

        size_type idx = find_bucket_for_insert(key);
        if (idx == capacity_) {
            rehash_if_needed(1);
            idx = find_bucket_for_insert(key);
            if (idx == capacity_) {
                throw std::runtime_error("flat_hash_map::operator[]: no space");
            }
        }

        if (control_[idx] == Control::Occupied &&
            key_equal_(values_[idx].first, key))
        {
            return values_[idx].second;
        }

        control_[idx] = Control::Occupied;
        values_[idx] = value_type(std::move(key), mapped_type{});
        ++size_;

        return values_[idx].second;
    }



    mapped_type& at(const key_type& key) {
        size_type idx = find_bucket_for_key(key);
        if (idx == capacity_) {
            throw std::out_of_range("[ERROR] flat_hash_map::at: key not found");
        }
        return values_[idx].second;
    }



    const mapped_type& at(const key_type& key) const {
        size_type idx = find_bucket_for_key(key);
        if (idx == capacity_) {
            throw std::out_of_range("[ERROR] flat_hash_map::at: key not found");
        }
        return values_[idx].second;
    }





    //=======================================ПОИСК======================



    iterator find(const key_type& key) {
        size_type idx = find_bucket_for_key(key);
        if (idx == capacity_) {
            return end();
        }
        return make_iterator(idx);
    }

    const_iterator find(const key_type& key) const {
        size_type idx = find_bucket_for_key(key);
        if (idx == capacity_) {
            return cend();
        }
        return make_const_iterator(idx);
    }



    bool contains(const key_type& key) const {
        return find_bucket_for_key(key) != capacity_;
    }

    size_type count(const key_type& key) const {
        return contains(key) ? 1 : 0;
    }


private:
    enum class Control : std::uint8_t {
        Empty,
        Occupied,
        Deleted
    };

    std::vector<value_type> values_;
    std::vector<Control> control_;
    size_type size_ = 0;
    size_type capacity_ = 0;

    hasher hasher_;
    key_equal key_equal_;
    float max_load_factor_ = 0.5f;


    size_type mask() const noexcept {
        return capacity_ ? (capacity_ - 1) : 0;
    }


    void init_storage(size_type bucket_count) {
        if (bucket_count == 0) {
            capacity_ = 0;
            values_.clear();
            control_.clear();
            size_ = 0;
            return;
        }


        size_type cap = 1;
        while (cap < bucket_count) {
            cap <<= 1;
        }

        capacity_ = cap;
        values_.resize(capacity_);
        control_.assign(capacity_, Control::Empty);
        size_ = 0;
    }


    void rehash_to(size_type new_capacity) {
        if (new_capacity == 0) {
            init_storage(0);
            return;
        }

        // Сохраняем старое
        std::vector<value_type> old_values;
        std::vector<Control>    old_control;
        const size_type old_capacity = capacity_;

        old_values.swap(values_);
        old_control.swap(control_);

        // Создаём новую таблицу
        init_storage(new_capacity); // capacity_ округлится до степени 2, control_=Empty, size_=0

        // Переносим только Occupied
        for (size_type j = 0; j < old_capacity; ++j) {
            if (old_control[j] != Control::Occupied) continue;

            value_type& v = old_values[j];
            const key_type& key = v.first;

            // В новой таблице есть только Empty/Occupied
            size_type idx = static_cast<size_type>(hasher_(key)) & mask();
            while (control_[idx] == Control::Occupied) {
                idx = (idx + 1) & mask();
            }

            control_[idx] = Control::Occupied;
            values_[idx] = std::move(v);
            ++size_;
        }
    }



    void rehash_if_needed(size_type additional = 1) {
        const size_type needed = size_ + additional;

        if (max_load_factor_ <= 0.0f) {
            // “без лимита”: просто гарантируем, что capacity_ >= needed
            if (needed > capacity_) {
                rehash_to(capacity_ == 0 ? 8 : capacity_ * 2);
            }
            return;
        }

        // если таблицы нет — создаём
        if (capacity_ == 0) {
            rehash_to(8);
            return;
        }

        // если после вставки превысим лимит — увеличиваем
        const float limit = max_load_factor_ * static_cast<float>(capacity_);
        if (static_cast<float>(needed) > limit) {
            rehash_to(capacity_ * 2);
        }
    }



    //======================================================FIND BUCKET====================================================



    //Поиск ключа по диапазону
    size_type scan_range_for_key(const key_type& key, size_type begin, size_type end) const {
        size_type i = begin;


        //simd регистры заполненные одним значением
        const __m128i occ_vec = _mm_set1_epi8(static_cast<char>(Control::Occupied));
        const __m128i emp_vec = _mm_set1_epi8(static_cast<char>(Control::Empty));


        for (; i + 15 < end; i += 16) {
            //загружаем блок из памяти
            const __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&control_[i]));

            //cmpeq сравнит и выставит FF где равно, а потом собираем их в int маску где будут 1 если равны
            int occ_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(block, occ_vec));
            while (occ_mask) {//Пока остался хотя бы один occ
                unsigned long bit;
                _BitScanForward(&bit, occ_mask);
                //считает количество нулей справа до единицы
                size_type idx = i + static_cast<size_type>(bit);

                if (key_equal_(values_[idx].first, key)) {//Проверяем по индексу
                    return idx;
                }
                occ_mask &= occ_mask - 1; // Убираем
            }


            int emp_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(block, emp_vec)); //Проверяем на пустоту
            if (emp_mask) {
                return capacity_;
            }
        }

        //Хвост 
        for (; i < end; ++i) {
            Control c = control_[i];
            if (c == Control::Empty) {
                return capacity_;
            }
            if (c == Control::Occupied && key_equal_(values_[i].first, key)) {
                return i;
            }
        }

        return capacity_;
    }


    size_type find_bucket_for_key(const key_type& key) const {
        if (capacity_ == 0) return capacity_;

        const size_type start = hasher_(key) & mask();

        size_type res = scan_range_for_key(key, start, capacity_);
        if (res != capacity_) return res;

        return scan_range_for_key(key, 0, start);
    }


    size_type scan_range_for_insert(const key_type& key, size_type begin, size_type end, size_type& first_deleted) {
        size_type i = begin;

        const __m128i occ_vec = _mm_set1_epi8(static_cast<char>(Control::Occupied));
        const __m128i emp_vec = _mm_set1_epi8(static_cast<char>(Control::Empty));
        const __m128i del_vec = _mm_set1_epi8(static_cast<char>(Control::Deleted));


        for (; i + 15 < end; i += 16) {
            const __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&control_[i]));

            int occ_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(block, occ_vec));
            //Если уже есть то возвращем его
            while (occ_mask) {
                unsigned long bit;
                _BitScanForward(&bit, occ_mask);

                size_type idx = i + static_cast<size_type>(bit);

                if (key_equal_(values_[idx].first, key)) {
                    return idx;
                }
                occ_mask &= occ_mask - 1;
            }

            //Один раз находим первое вхождение deleted
            if (first_deleted == capacity_) {
                int del_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(block, del_vec));
                if (del_mask) {
                    unsigned long bit;
                    _BitScanForward(&bit, del_mask);
                    first_deleted = i + static_cast<size_type>(bit);
                }
            }

            //Если нашли empty, дальше смысла идти нету. Но лучше вставить в deleted 
            int emp_mask = _mm_movemask_epi8(_mm_cmpeq_epi8(block, emp_vec));
            if (emp_mask) {
                unsigned long bit;
                _BitScanForward(&bit, emp_mask);
                size_type empty_idx = i + static_cast<size_type>(bit);
                return (first_deleted != capacity_) ? first_deleted : empty_idx;
            }
        }

        //Добивка хвоста 
        for (; i < end; ++i) {
            Control c = control_[i];

            if (c == Control::Occupied) {
                if (key_equal_(values_[i].first, key)) {
                    return i;
                }
            }
            else if (c == Control::Deleted) {
                if (first_deleted == capacity_) {
                    first_deleted = i;
                }
            }
            else {
                return (first_deleted != capacity_) ? first_deleted : i;
            }
        }

        return capacity_;
    }



    size_type find_bucket_for_insert(const key_type& key) {
        if (capacity_ == 0) {
            return capacity_;
        }

        const size_type start = hasher_(key) & mask();
        size_type first_deleted = capacity_;

        // [start, capacity_)
        size_type res = scan_range_for_insert(key, start, capacity_, first_deleted);
        if (res != capacity_) {
            return res;
        }

        // [0, start)
        res = scan_range_for_insert(key, 0, start, first_deleted);
        if (res != capacity_) {
            return res;
        }

        return first_deleted;
    }


    // Создание итераторов по индексу
    iterator make_iterator(size_type index) {
        return iterator(this, index);
    }
    const_iterator make_const_iterator(size_type index) const {
        return const_iterator(this, index);
    }

    // Итераторам нужен доступ к приватным полям
    friend class iterator;
    friend class const_iterator;
};


#include "flat_hash_map_iterator.h"

