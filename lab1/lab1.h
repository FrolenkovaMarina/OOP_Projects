#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <string_view>


struct Node;

class Trie {
public:
    using size_type = std::size_t;

    // --- конструкторы/деструктор ---
    Trie();                                // Конструктор по умолчанию
    ~Trie();                               // Деструктор

    Trie(const Trie& other);               // Конструктор копирования
    Trie(Trie&& other) noexcept;           // Конструктор перемещения

    Trie& operator=(const Trie& other);    // Оператор присваивания копированием
    Trie& operator=(Trie&& other) noexcept;// Оператор присваивания перемещением

    // --- базовые методы ---
    void insert(std::string_view s);       // Вставка строки
    bool contains(std::string_view s) const;  // Проверка наличия строки
    bool erase(std::string_view s);        // Удаление строки
    size_type prefix_count(std::string_view pref) const; // Количество слов с данным префиксом

    std::vector<std::string> words_with_prefix(
        std::string_view pref, size_type limit = 0) const; // Все слова с данным префиксом

    // --- служебные методы ---
    void clear();                          // Очистка дерева
    bool empty() const;                    // true, если нет слов
    size_type size() const;                // Количество сохранённых слов

    // --- перегрузка операторов ---
    bool operator==(const Trie& other) const; // Равенство по множеству строк
    bool operator!=(const Trie& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Trie& trie);
    // Красивый вывод всех слов в алфавитном порядке
private:
    size_type wordCount_ = 0;
    Node* root_ = nullptr;
};

