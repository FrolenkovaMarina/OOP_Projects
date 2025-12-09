#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <string_view>


struct Node;

class Trie {
public:
    using size_type = std::size_t;
    static constexpr size_type DEFAULT_LIMIT = 8;
    // --- ������������/���������� ---
    Trie();                                // ����������� �� ���������
    ~Trie();                               // ����������

    Trie(const Trie& other);               // ����������� �����������
    Trie(Trie&& other) noexcept;           // ����������� �����������

    Trie& operator=(const Trie& other);    // �������� ������������ ������������
    Trie& operator=(Trie&& other) noexcept;// �������� ������������ ������������

    // --- ������� ������ ---
    void insert(std::string_view s);       // ������� ������
    bool contains(std::string_view s) const;  // �������� ������� ������
    bool erase(std::string_view s);        // �������� ������
    size_type prefix_count(std::string_view pref) const; // ���������� ���� � ������ ���������

    std::vector<std::string> words_with_prefix(
        std::string_view pref, size_type limit = 0) const; // ��� ����� � ������ ���������

    // --- ��������� ������ ---
    void clear();                          // ������� ������
    bool empty() const;                    // true, ���� ��� ����
    size_type size() const;                // ���������� ����������� ����

    // --- ���������� ���������� ---
    bool operator==(const Trie& other) const; // ��������� �� ��������� �����
    bool operator!=(const Trie& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Trie& trie);
    // �������� ����� ���� ���� � ���������� �������
private:
    size_type wordCount_ = 0;
    Node* root_ = nullptr;
};

