#include <gtest/gtest.h>
#include "lab1.h"

TEST(TrieBasic, EmptyState) {
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

TEST(TrieEmpty, InsertOnce) {
    Trie t;
    t.insert("");

    EXPECT_FALSE(t.empty());
    EXPECT_EQ(t.size(), 1u);

    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.prefix_count(""), 1u);

    auto all = t.words_with_prefix("");
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0], "");
}

TEST(TrieEmpty, InsertTwice) {
    Trie t;
    t.insert("");
    t.insert("");

    EXPECT_EQ(t.size(), 1u);
    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.prefix_count(""), 1u);
}

TEST(TrieEmpty, EraseEmpty) {
    Trie t;
    t.insert("");
    EXPECT_TRUE(t.contains(""));
    EXPECT_EQ(t.size(), 1u);

    EXPECT_TRUE(t.erase(""));
    EXPECT_FALSE(t.contains(""));
    EXPECT_EQ(t.size(), 0u);
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.prefix_count(""), 0u);
}

TEST(TrieEmpty, EraseEmptyFromEmpty) {
    Trie t;
    EXPECT_FALSE(t.erase(""));
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0u);
}

TEST(TrieDup, InsertSameWord) {
    Trie t;
    t.insert("abc");

    EXPECT_TRUE(t.contains("abc"));
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.prefix_count(""), 1u);
    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 1u);
    EXPECT_EQ(t.prefix_count("abc"), 1u);

    t.insert("abc");
    t.insert("abc");
    t.insert("abc");

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

TEST(TriePrune, EraseLeafKeepPrefix) {
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

    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 0u);

    auto w = t.words_with_prefix("a");
    ASSERT_EQ(w.size(), 1u);
    EXPECT_EQ(w[0], "a");
}

TEST(TriePrune, EraseBranchKeepSibling) {
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

    EXPECT_EQ(t.prefix_count("a"), 1u);
    EXPECT_EQ(t.prefix_count("ab"), 0u);
    EXPECT_EQ(t.prefix_count("ac"), 1u);

    auto w = t.words_with_prefix("a");
    ASSERT_EQ(w.size(), 1u);
    EXPECT_EQ(w[0], "ac");
}

TEST(TriePrune, EraseDeepLeafKeepShorter) {
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

    auto w = t.words_with_prefix("ab");
    ASSERT_EQ(w.size(), 1u);
    EXPECT_EQ(w[0], "ab");
}

TEST(TrieErase, EraseMissing) {
    Trie t;
    t.insert("hello");

    EXPECT_FALSE(t.erase("world"));
    EXPECT_TRUE(t.contains("hello"));
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.prefix_count(""), 1u);
}

TEST(TrieWords, AllWithEmptyPrefix) {
    Trie t;
    t.insert("car");
    t.insert("cat");
    t.insert("dog");

    auto all = t.words_with_prefix("");
    ASSERT_EQ(all.size(), 3u);

    EXPECT_EQ(all[0], "car");
    EXPECT_EQ(all[1], "cat");
    EXPECT_EQ(all[2], "dog");
}
