#include "pch.h"
#include <gtest/gtest.h>

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>


#include "flat_hash_map.h"

// Хешер, создающий много коллизий (чтобы проверить linear probing и Deleted)
struct BadHash {
    std::size_t operator()(int) const noexcept { return 0; }
};

// ---------- Constructors / basic ----------

TEST(FlatHashMapCtor, DefaultCtorEmpty) {
    flat_hash_map<int, int> m;
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
    EXPECT_EQ(m.load_factor(), 0.0f);
}

TEST(FlatHashMapCtor, BucketCountCtorCapacityAtLeast) {
    flat_hash_map<int, int> m(10);
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
    EXPECT_GE(m.capacity(), 10u);
}

TEST(FlatHashMapCtor, InitListCtorWorks) {
    flat_hash_map<int, int> m({ {1, 10}, {2, 20}, {3, 30} }, 0);
    EXPECT_EQ(m.size(), 3u);
    EXPECT_EQ(m.at(2), 20);
}

TEST(FlatHashMapCtor, RangeCtorWorks) {
    std::vector<std::pair<int, int>> v = { {1,10},{2,20},{3,30} };
    flat_hash_map<int, int> m(v.begin(), v.end(), 0);
    EXPECT_EQ(m.size(), 3u);
    EXPECT_TRUE(m.contains(3));
}

// ---------- clear / empty / size ----------

TEST(FlatHashMapBasic, ClearEmptiesButKeepsCapacity) {
    flat_hash_map<int, int> m(8);
    m.insert({ 1, 10 });
    m.insert({ 2, 20 });
    auto cap = m.capacity();

    m.clear();
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
    EXPECT_EQ(m.capacity(), cap);
    EXPECT_FALSE(m.contains(1));
}

// ---------- insert ----------

TEST(FlatHashMapInsert, InsertSingle) {
    flat_hash_map<int, int> m(8);
    auto res = m.insert({ 1, 10 });
    EXPECT_TRUE(res.second);
    EXPECT_EQ(m.size(), 1u);
    EXPECT_TRUE(m.contains(1));
    EXPECT_EQ(m.at(1), 10);
}

TEST(FlatHashMapInsert, InsertDuplicateDoesNotOverwrite) {
    flat_hash_map<int, int> m(8);
    m.insert({ 1, 10 });
    auto res = m.insert({ 1, 999 });
    EXPECT_FALSE(res.second);
    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m.at(1), 10);
}

TEST(FlatHashMapInsert, InsertRvalue) {
    flat_hash_map<int, int> m(8);
    std::pair<int, int> p{ 5, 50 };
    auto res = m.insert(std::move(p));
    EXPECT_TRUE(res.second);
    EXPECT_TRUE(m.contains(5));
    EXPECT_EQ(m.at(5), 50);
}

TEST(FlatHashMapInsert, InsertRange) {
    flat_hash_map<int, int> m(8);
    std::vector<std::pair<int, int>> v = { {1,10},{2,20},{3,30} };
    m.insert(v.begin(), v.end());
    EXPECT_EQ(m.size(), 3u);
    EXPECT_EQ(m.at(3), 30);
}

TEST(FlatHashMapInsert, InsertInitList) {
    flat_hash_map<int, int> m(8);
    m.insert({ {1,10},{2,20} });
    EXPECT_EQ(m.size(), 2u);
    EXPECT_TRUE(m.contains(2));
}

// ---------- emplace ----------

TEST(FlatHashMapEmplace, EmplaceNew) {
    flat_hash_map<int, int> m(8);
    auto res = m.emplace(7, 70);
    EXPECT_TRUE(res.second);
    EXPECT_EQ(m.at(7), 70);
}

TEST(FlatHashMapEmplace, EmplaceDuplicate) {
    flat_hash_map<int, int> m(8);
    m.emplace(7, 70);
    auto res = m.emplace(7, 700);
    EXPECT_FALSE(res.second);
    EXPECT_EQ(m.at(7), 70);
}

// ---------- find / contains / count ----------

TEST(FlatHashMapLookup, FindExistingAndMissing) {
    flat_hash_map<int, int> m(8);
    m.insert({ 1,10 });
    auto it1 = m.find(1);
    EXPECT_NE(it1, m.end());
    EXPECT_EQ(it1->second, 10);

    auto it2 = m.find(999);
    EXPECT_EQ(it2, m.end());
}

TEST(FlatHashMapLookup, ContainsAndCount) {
    flat_hash_map<int, int> m(8);
    m.insert({ 1,10 });
    EXPECT_TRUE(m.contains(1));
    EXPECT_FALSE(m.contains(2));
    EXPECT_EQ(m.count(1), 1u);
    EXPECT_EQ(m.count(2), 0u);
}

// ---------- operator[] / at ----------

TEST(FlatHashMapAccess, BracketsInsertsDefault) {
    flat_hash_map<int, std::string> m(8);

    EXPECT_FALSE(m.contains(5));
    std::string& s = m[5];
    EXPECT_TRUE(m.contains(5));
    EXPECT_EQ(s, "");

    s = "hello";
    EXPECT_EQ(m.at(5), "hello");
}

TEST(FlatHashMapAccess, AtThrowsIfMissing) {
    flat_hash_map<int, int> m(8);
    EXPECT_THROW(m.at(123), std::out_of_range);
}

// ---------- erase(key) ----------

TEST(FlatHashMapErase, EraseKeyExistingAndMissing) {
    flat_hash_map<int, int> m(8);
    m.insert({ 1,10 });
    EXPECT_EQ(m.erase(1), 1u);
    EXPECT_EQ(m.erase(1), 0u);
    EXPECT_FALSE(m.contains(1));
    EXPECT_EQ(m.size(), 0u);
}

// Проверка tombstone: удаление не ломает поиск дальше в цепочке
TEST(FlatHashMapErase, TombstoneDoesNotBreakSearch) {
    flat_hash_map<int, int, BadHash> m(8);

    m.insert({ 1,10 });
    m.insert({ 2,20 });
    m.insert({ 3,30 });

    EXPECT_EQ(m.erase(1), 1u);

    // Должны находиться
    EXPECT_TRUE(m.contains(2));
    EXPECT_TRUE(m.contains(3));
    EXPECT_EQ(m.at(2), 20);
    EXPECT_EQ(m.at(3), 30);
}

// ---------- erase(iterator) ----------

TEST(FlatHashMapErase, EraseIterator) {
    flat_hash_map<int, int> m(16);
    m.insert({ 1,10 });
    m.insert({ 2,20 });
    m.insert({ 3,30 });

    auto it = m.find(2);
    ASSERT_NE(it, m.end());

    auto next = m.erase(it);
    EXPECT_FALSE(m.contains(2));
    EXPECT_EQ(m.size(), 2u);

    // next может быть end или следующий occupied — просто проверим, что корректен
    if (next != m.end()) {
        EXPECT_TRUE(next->first == 1 || next->first == 3);
    }
}

// ---------- iterators ----------

TEST(FlatHashMapIterators, IterateAllKeys) {
    flat_hash_map<int, int> m(16);
    m.insert({ 1,10 });
    m.insert({ 2,20 });
    m.insert({ 3,30 });

    std::vector<int> keys;
    for (auto it = m.begin(); it != m.end(); ++it) {
        keys.push_back(it->first);
    }
    std::sort(keys.begin(), keys.end());
    EXPECT_EQ(keys, (std::vector<int>{1, 2, 3}));
}

TEST(FlatHashMapIterators, ConstIteratorsWork) {
    flat_hash_map<int, int> m(16);
    m.insert({ 1,10 });
    m.insert({ 2,20 });

    const auto& cm = m;
    int sum = 0;
    for (auto it = cm.cbegin(); it != cm.cend(); ++it) {
        sum += it->second;
    }
    EXPECT_EQ(sum, 30);
}

// ---------- reserve / max_load_factor ----------

TEST(FlatHashMapCapacity, ReserveKeepsElements) {
    flat_hash_map<int, int, BadHash> m(8);
    for (int i = 0; i < 30; ++i) {
        m.insert({ i, i * 10 });
    }
    m.reserve(200);

    for (int i = 0; i < 30; ++i) {
        EXPECT_TRUE(m.contains(i));
        EXPECT_EQ(m.at(i), i * 10);
    }
}

TEST(FlatHashMapCapacity, MaxLoadFactorAffectsGrowth) {
    flat_hash_map<int, int, BadHash> m(8);
    m.max_load_factor(0.25f);

    auto cap0 = m.capacity();
    for (int i = 0; i < 100; ++i) {
        m.insert({ i, i });
    }
    EXPECT_GT(m.capacity(), cap0);
    EXPECT_EQ(m.size(), 100u);
}

// ---------- copy / move / assign / swap ----------

TEST(FlatHashMapSpecial, CopyCtor) {
    flat_hash_map<int, int> a(8);
    a.insert({ 1,10 });
    a.insert({ 2,20 });

    flat_hash_map<int, int> b(a);
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(b.at(1), 10);
}

TEST(FlatHashMapSpecial, MoveCtor) {
    flat_hash_map<int, int> a(8);
    a.insert({ 1,10 });
    a.insert({ 2,20 });

    flat_hash_map<int, int> b(std::move(a));
    EXPECT_EQ(b.size(), 2u);
    EXPECT_TRUE(b.contains(2));
}

TEST(FlatHashMapSpecial, CopyAssign) {
    flat_hash_map<int, int> a(8);
    a.insert({ 1,10 });
    flat_hash_map<int, int> b(8);
    b = a;
    EXPECT_TRUE(b.contains(1));
    EXPECT_EQ(b.at(1), 10);
}

TEST(FlatHashMapSpecial, MoveAssign) {
    flat_hash_map<int, int> a(8);
    a.insert({ 1,10 });
    flat_hash_map<int, int> b(8);
    b = std::move(a);
    EXPECT_TRUE(b.contains(1));
    EXPECT_EQ(b.at(1), 10);
}

TEST(FlatHashMapSpecial, AssignInitList) {
    flat_hash_map<int, int> m(8);
    m = { {3,30},{4,40} };
    EXPECT_EQ(m.size(), 2u);
    EXPECT_TRUE(m.contains(4));
}

TEST(FlatHashMapSpecial, Swap) {
    flat_hash_map<int, int> a(8), b(8);
    a.insert({ 1,10 });
    b.insert({ 2,20 });

    a.swap(b);
    EXPECT_TRUE(a.contains(2));
    EXPECT_FALSE(a.contains(1));
    EXPECT_TRUE(b.contains(1));
}
