#include "lab1.h"
#include <map>


struct Node {
	bool endOfWorld = false;
	std::size_t subwords = 0;
	std::map<unsigned char, Node*> children;
};

void deleteSubtree(Node* v) {
	if (!v) return;
	for (auto& kv : v->children) {
		deleteSubtree(kv.second);
	}
	delete v;
}

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

void collectWords(const Node* v,
	std::string& cur,
	std::vector<std::string>& out,
	std::size_t limit) {
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

bool tryPruneChild(Node* parent, unsigned char b) {
	auto it = parent->children.find(b);
	if (it == parent->children.end()) return false;
	Node* c = it->second;
	if (c->subwords != 0) return false;
	if (!c->children.empty()) return false;
	delete c;
	parent->children.erase(it);
	return true;
}


Trie::Trie() : root_(new Node), wordCount_(0) {}

Trie::~Trie() {
	clear();
	delete root_;
	root_ = nullptr;
}

Trie::Trie(const Trie& other) :
	root_(cloneNode(other.root_)), wordCount_(other.wordCount_) {
}


Trie::Trie(Trie&& other) noexcept
	: root_(other.root_), wordCount_(other.wordCount_) {
	other.root_ = new Node;
	other.wordCount_ = 0;
}

Trie& Trie::operator=(const Trie& other) {
	if (this == &other) return *this;
	Node* newRoot = cloneNode(other.root_);
	clear();
	delete root_;
	root_ = newRoot;
	wordCount_ = other.wordCount_;
	return *this;
}

Trie& Trie::operator=(Trie&& other) noexcept {
	if (this == &other) return*this;
	clear();
	delete root_;
	root_ = other.root_;
	wordCount_ = other.wordCount_;
	other.root_ = new Node;
	other.wordCount_ = 0;
	return *this;
}

void Trie::clear() {
	for (auto& kv : root_->children) {
		deleteSubtree(kv.second);
	}
	root_->children.clear();
	root_->endOfWorld = false;
	root_->subwords = 0;
	wordCount_ = 0;
}

bool Trie::empty() const { return wordCount_ == 0; }

Trie::size_type Trie::size() const { return wordCount_; }

void Trie::insert(std::string_view s) {
	Node* v = root_;
	std::vector<Node*> path;
	path.reserve(s.size() + 1);
	path.push_back(v);


	for (unsigned char b : s) {
		auto it = v->children.find(b);
		if (it == v->children.end()) {
			Node* nw = new Node();
			v->children.emplace(b, nw);
			v = nw;
		}
		else {
			v = it->second;
		}
		path.push_back(v);
	}

	if (v->endOfWorld) {
		return;
	}

	v->endOfWorld = true;

	for (Node* u : path) {
		u->subwords += 1;
	}
	++wordCount_;
}

bool Trie::contains(std::string_view s) const {
	const Node* v = root_;
	for (unsigned char b : s) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return false;
		v = it->second;
	}
	return v->endOfWorld;
}

bool Trie::erase(std::string_view s) {
	if (!contains(s)) return false;

	std::vector<Node*> path;
	path.reserve(s.size() + 1);
	Node* v = root_;
	path.push_back(v);
	for (unsigned char b : s) {
		v = v->children[b];
		path.push_back(v);
	}

	v->endOfWorld = false;

	for (Node* u : path) {
		u->subwords -= 1;
	}

	for (std::size_t i = s.size(); i > 0; --i) {
		Node* parent = path[i - 1];
		unsigned char b = static_cast<unsigned char>(s[i - 1]);
		tryPruneChild(parent, b);
	}

	--wordCount_;
	return true;
}

Trie::size_type Trie::prefix_count(std::string_view pref) const {
	const Node* v = root_;
	for (unsigned char b : pref) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return 0;
		v = it->second;
	}
	return v->subwords;
}

std::vector<std::string> Trie::words_with_prefix(std::string_view pref, size_type limit) const {
	const Node* v = root_;
	for (unsigned char b : pref) {
		auto it = v->children.find(b);
		if (it == v->children.end()) return {};
		v = it->second;
	}
	std::vector<std::string> out;
	out.reserve(limit ? limit : 8);
	std::string cur(pref);
	collectWords(v, cur, out, limit);
	return out;
}

bool Trie::operator==(const Trie& other) const {
	if (size() != other.size()) return false;
	auto a = words_with_prefix("");
	auto b = other.words_with_prefix("");
	return a == b;
}

bool Trie::operator!=(const Trie& other) const {
	return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Trie& trie) {
	auto all = trie.words_with_prefix("");
	for (std::size_t i = 0; i < all.size(); ++i) {
		os << all[i];
		if (i + 1 < all.size()) os << '\n';
	}
	return os;
}
