#include <gtest/gtest.h>
#include "lab1.h"

// Базовое состояние: пустой Trie
TEST(TrieBasic, InitiallyEmpty) {
    Trie t;
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0u);

    EXPECT_FALSE(t.contains(""));
    EXPECT_FALSE(t.contains("a"));

    EXPECT_EQ(t.prefix_count(""), 0u);
    EXPECT_EQ(t.prefix_count("a"), 0u);

    auto all = t.words_with_prefix("");
    EXPECT_TRUE(all.empty());
}

// --- ПУСТЫЕ СТРОКИ ---

// Вставка пустой строки
TEST(TrieEmptyString, InsertEmptyOnce) {
    Trie t;
    t.insert("");

    EXPECT_FALSE(t.empty());
    EXPECT_EQ(t.size(), 1u);

    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.prefix_count(""), 1u);   // одно слово в дереве — пустое

    auto all = t.words_with_prefix("");
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0], "");               // именно пустая строка
}

// Повторная вставка пустой строки
TEST(TrieEmptyString, InsertEmptyTwice) {
    Trie t;
    t.insert("");
    t.insert("");            // должна быть идемпотентна

    EXPECT_EQ(t.size(), 1u);           // не увеличилось
    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.prefix_count(""), 1u); // счётчик слов тоже не удвоился
}

// Удаление пустой строки
TEST(TrieEmptyString, EraseEmpty) {
    Trie t;
    t.insert("");
    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.size(), 1u);

    EXPECT_TRUE(t.erase(""));   // должно удалиться
    EXPECT_FALSE(t.contains(""));
    EXPECT_EQ(t.size(), 0u);
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.prefix_count(""), 0u);
}

// Попытка удалить пустую строку из пустого дерева
TEST(TrieEmptyString, EraseEmptyFromEmptyTrie) {
    Trie t;
    EXPECT_FALSE(t.erase(""));  // ничего не было — ничего не удалили
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0u);
}

// --- ПОВТОРНЫЕ ВСТАВКИ (НЕПУСТЫЕ СЛОВА) ---

TEST(TrieDuplicates, InsertSameWordManyTimes) {
    Trie t;
    t.insert("abc");
    EXPECT_TRUE(t.contains("abc"));
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.prefix_count(""), 1u);
    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 1u);
    EXPECT_EQ(t.prefix_count("abc"), 1u);

    // Повторные вставки того же слова
    t.insert("abc");
    t.insert("abc");
    t.insert("abc");

    // Размер и префиксные счётчики не должны вырасти
    EXPECT_TRUE(t.contains("abc"));
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.prefix_count(""), 1u);
    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 1u);
    EXPECT_EQ(t.prefix_count("abc"), 1u);

    auto all = t.words_with_prefix("");
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0], "abc");
}

// --- УДАЛЕНИЕ «ВИСЯЧИХ» УЗЛОВ (листов/веток) ---

// Сценарий: "a", "ab" → erase("ab")
// Должен удалиться узел 'b', но слово "a" остаться.
TEST(TriePrune, RemoveLeafWordButKeepPrefixWord) {
    Trie t;
    t.insert("a");
    t.insert("ab");

    EXPECT_TRUE(t.contains("a"));
    EXPECT_TRUE(t.contains("ab"));
    EXPECT_EQ(t.size(), 2u);

    EXPECT_EQ(t.prefix_count("a"), 2u);
    EXPECT_EQ(t.prefix_count("ab"), 1u);

    EXPECT_TRUE(t.erase("ab"));

    EXPECT_TRUE(t.contains("a"));
    EXPECT_FALSE(t.contains("ab"));
    EXPECT_EQ(t.size(), 1u);

    // Теперь в поддереве 'a' только одно слово
    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 0u);

    auto words_a = t.words_with_prefix("a");
    ASSERT_EQ(words_a.size(), 1u);
    EXPECT_EQ(words_a[0], "a");
}

// Сценарий: "ab", "ac" → erase("ab")
// Должен удалиться лист 'b', но общая часть ('a') и ветка 'c' остаться.
TEST(TriePrune, RemoveOneBranchButKeepSibling) {
    Trie t;
    t.insert("ab");
    t.insert("ac");

    EXPECT_TRUE(t.contains("ab"));
    EXPECT_TRUE(t.contains("ac"));
    EXPECT_EQ(t.size(), 2u);

    EXPECT_EQ(t.prefix_count("a"), 2u);
    EXPECT_EQ(t.prefix_count("ab"), 1u);
    EXPECT_EQ(t.prefix_count("ac"), 1u);

    EXPECT_TRUE(t.erase("ab"));

    EXPECT_FALSE(t.contains("ab"));
    EXPECT_TRUE(t.contains("ac"));
    EXPECT_EQ(t.size(), 1u);

    // В поддереве "a" остался только "ac"
    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 0u);
    EXPECT_EQ(t.prefix_count("ac"), 1u);

    auto words_a = t.words_with_prefix("a");
    ASSERT_EQ(words_a.size(), 1u);
    EXPECT_EQ(words_a[0], "ac");
}

// Сценарий: "abc", "ab" → erase("abc")
// Должен удалиться лист 'c', но "ab" остаться, а 'b' не удаляться.
TEST(TriePrune, RemoveDeeperLeafButKeepShorterWord) {
    Trie t;
    t.insert("ab");
    t.insert("abc");

    EXPECT_TRUE(t.contains("ab"));
    EXPECT_TRUE(t.contains("abc"));
    EXPECT_EQ(t.size(), 2u);

    EXPECT_EQ(t.prefix_count("ab"), 2u);
    EXPECT_EQ(t.prefix_count("abc"), 1u);

    EXPECT_TRUE(t.erase("abc"));

    EXPECT_TRUE(t.contains("ab"));
    EXPECT_FALSE(t.contains("abc"));
    EXPECT_EQ(t.size(), 1u);

    EXPECT_EQ(t.prefix_count("ab"), 1u);
    EXPECT_EQ(t.prefix_count("abc"), 0u);

    auto words_ab = t.words_with_prefix("ab");
    ASSERT_EQ(words_ab.size(), 1u);
    EXPECT_EQ(words_ab[0], "ab");
}

// Попытка удалить слово, которого нет
TEST(TrieErase, EraseNonExistingWordDoesNothing) {
    Trie t;
    t.insert("hello");

    EXPECT_FALSE(t.erase("world"));   // вернёт false
    EXPECT_TRUE(t.contains("hello"));
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.prefix_count(""), 1u);
}

// Дополнительно: words_with_prefix("") возвращает все слова
TEST(TrieWordsWithPrefix, AllWordsWithEmptyPrefix) {
    Trie t;
    t.insert("car");
    t.insert("cat");
    t.insert("dog");

    auto all = t.words_with_prefix("");
    ASSERT_EQ(all.size(), 3u);
    // порядок должен быть лексикографический: "car", "cat", "dog"
    EXPECT_EQ(all[0], "car");
    EXPECT_EQ(all[1], "cat");
    EXPECT_EQ(all[2], "dog");
}
