#include "lab1.h"
#include <map>


//create node
struct Node {
	bool endOfWorld = false;
	std::size_t subwords = 0;
	std::map<unsigned char, Node*> children;
};

// рекурсивное удаление поддерева
void deleteSubtree(Node* v) {
	if (!v) return;
	for (auto& kv : v->children) {
		deleteSubtree(kv.second);
	}
	delete v;
}

// глубокое копирование поддерева
Node* cloneNode(const Node* src) {
	if (!src) return nullptr;
	Node* dst = new Node();
	dst->endOfWorld = src->endOfWorld;
	dst->subwords = src->subwords;
	for (const auto& [b, child] : src->children) {
		dst->children[b] = cloneNode(child);
	}
	return dst;
}

// DFS: собираем слова из поддерева (лексикографически по возрастанию байтов-ключей)
void collectWords(const Node* v,
	std::string& cur,
	std::vector<std::string>& out,
	std::size_t limit /*0 = без лимита*/) {
	if (!v) return;
	if (v->endOfWorld) {
		out.push_back(cur);
		if (limit && out.size() >= limit) return;
	}
	for (const auto& [b, child] : v->children) {
		cur.push_back(static_cast<char>(b));
		collectWords(child, cur, out, limit);
		if (limit && out.size() >= limit) { cur.pop_back(); return; }
		cur.pop_back();
	}
}

// попытка удалить «висячего» ребёнка по байту b
bool tryPruneChild(Node* parent, unsigned char b) {
    auto it = parent->children.find(b);
    if (it == parent->children.end()) return false;
    Node* c = it->second;
    if (c->subwords != 0) return false;          // в поддереве ещё есть слова
    if (!c->children.empty()) return false;      // есть потомки — не висячий
    delete c;
    parent->children.erase(it);
    return true;
}


// ===== Реализация Trie =====

Trie::Trie() : root_(new Node), wordCount_(0) {} // Конструктор по умолчанию

Trie::~Trie() { // Деструктор
	clear();
	delete root_;
	root_ = nullptr;
}

Trie::Trie(const Trie& other) :  // Конструктор копирования
	root_(cloneNode(other.root_)), wordCount_(other.wordCount_) {}
//Полностью копирует всё дерево: создаёт новый Node
// копирует все флаги(endOfWorld, subwords), 
// рекурсивно клонирует всех детей и возвращает новый корень


Trie::Trie(Trie&& other) noexcept  // Конструктор перемещения
	: root_(other.root_), wordCount_(other.wordCount_) { //просто копируем указатель
	other.root_ = new Node; // старый объект получает пустой корень
	other.wordCount_ = 0;
}

Trie& Trie::operator=(const Trie& other) { // Оператор присваивания копированием
	if (this == &other) return *this;
	Node* newRoot = cloneNode(other.root_);
	clear();
	delete root_;
	root_ = newRoot;
	wordCount_ = other.wordCount_;
	return *this;
}

Trie& Trie::operator=(Trie&& other) noexcept { // Оператор присваивания перемещением
	if (this == &other) return* this;
	clear();
	delete root_;
	root_ = other.root_;
	wordCount_ = other.wordCount_;
	other.root_ = new Node;
	other.wordCount_ = 0;
	return *this;
}

void Trie::clear() {
	// удалить всё поддерево корня, но корень оставить «пустым»
	for (auto& kv : root_->children) {
		deleteSubtree(kv.second);
	}
	root_->children.clear();
	root_->endOfWorld = false;
	root_->subwords = 0;
	wordCount_ = 0;
}

bool Trie::empty() const { return wordCount_ == 0; } // true, если нет слов

Trie::size_type Trie::size() const { return wordCount_; } // Количество сохранённых слов

// Вставка строки
void Trie::insert(std::string_view s) {
	Node* v = root_;
	std::vector<Node*> path;
	path.reserve(s.size() + 1);
	path.push_back(v);

	bool createdNewNode = false;

	// Спускаемся по дереву, создавая недостающие узлы
	for (unsigned char b : s) {
		auto it = v->children.find(b);
		if (it == v->children.end()) {
			Node* nw = new Node();
			v->children.emplace(b, nw);
			v = nw;
			createdNewNode = true;         // это важно!
		}
		else {
			v = it->second;
		}
		path.push_back(v);
	}

	// Если слово уже было — ничего не делаем
	if (v->endOfWorld) {
		return;
	}

	// Помечаем терминал
	v->endOfWorld = true;

	// Увеличиваем subwords по всему пути
	for (Node* u : path) {
		u->subwords += 1;
	}
	++wordCount_;
}

// Проверка наличия строки
bool Trie::contains(std::string_view s) const {
	const Node* v = root_;
	for (unsigned char b : s) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return false;
		v = it->second;
	}
	return v->endOfWorld;
}

// Удаление строки
bool Trie::erase(std::string_view s) {
	if (!contains(s)) return false;

	// сохраним путь от корня для обратного прохода (схлопывание)
	std::vector<Node*> path;
	path.reserve(s.size() + 1);
	Node* v = root_;
	path.push_back(v);
	for (unsigned char b : s) {
		v = v->children[b];        // существует
		path.push_back(v);
	}

	// снять терминальность
	v->endOfWorld = false;

	// уменьшить subwords вдоль пути
	for (Node* u : path) {
		u->subwords -= 1;
	}

	// снизу вверх пробуем удалить «висячие» узлы
	for (std::size_t i = s.size(); i > 0; --i) {
		Node* parent = path[i - 1];
		unsigned char b = static_cast<unsigned char>(s[i - 1]);
		tryPruneChild(parent, b);
	}

	--wordCount_;
	return true;
}

// Количество слов с данным префиксом
Trie::size_type Trie::prefix_count(std::string_view pref) const {
	const Node* v = root_;
	for (unsigned char b : pref) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return 0;
		v = it->second;
	}
	return v->subwords;
}

// Все слова с данным префиксом
std::vector<std::string> Trie::words_with_prefix(std::string_view pref, size_type limit) const {
	const Node* v = root_;
	for (unsigned char b : pref) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return {};
		v = it->second;
	}
	std::vector<std::string> out;
	out.reserve(limit ? limit : 8);
	std::string cur(pref); // начнём с самого префикса
	collectWords(v, cur, out, limit);
	return out;
}

// Равенство по множеству строк
bool Trie::operator==(const Trie& other) const {
	if (size() != other.size()) return false;
	auto a = words_with_prefix("");
	auto b = other.words_with_prefix("");
	return a == b;
}

bool Trie::operator!=(const Trie& other) const {
	return !(*this == other);
}

// Красивый вывод всех слов в алфавитном порядке
std::ostream& operator<<(std::ostream& os, const Trie& trie) {
	auto all = trie.words_with_prefix("");
	for (std::size_t i = 0; i < all.size(); ++i) {
		os << all[i];
		if (i + 1 < all.size()) os << '\n';
	}
	return os;
}