#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <map>
#include <cstddef>

int dateToInt(const std::string& date) {
    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6, 4));
    return year * 10000 + month * 100 + day;
}

struct ZagsRecord {
    std::string groom_fio;
    std::string groom_birth;
    std::string bride_fio;
    std::string bride_birth;
    std::string marriage_date;
    int         marriage_date_int;
    int         zags_number;

    int compare(const ZagsRecord& o) const {
        if (zags_number != o.zags_number)
            return zags_number < o.zags_number ? -1 : 1;
        if (marriage_date_int != o.marriage_date_int)
            return marriage_date_int < o.marriage_date_int ? -1 : 1;
        if (groom_fio != o.groom_fio)
            return groom_fio < o.groom_fio ? -1 : 1;
        return 0;
    }

    bool operator==(const ZagsRecord& o) const { return compare(o) == 0; }
    bool operator< (const ZagsRecord& o) const { return compare(o) < 0; }
    bool operator> (const ZagsRecord& o) const { return compare(o) > 0; }
    bool operator<=(const ZagsRecord& o) const { return compare(o) <= 0; }
    bool operator>=(const ZagsRecord& o) const { return compare(o) >= 0; }
};

std::vector<ZagsRecord> loadCSV(const std::string& filename, int maxRows = -1) {
    std::vector<ZagsRecord> result;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл \"" << filename << "\"\n";
        return result;
    }
    std::string line;
    std::getline(file, line);
    int rowsRead = 0;
    while (std::getline(file, line)) {
        if (maxRows != -1 && rowsRead >= maxRows) break;
        if (line.empty()) continue;
        std::istringstream ss(line);
        ZagsRecord rec;
        std::string token;
        std::getline(ss, rec.groom_fio, ',');
        std::getline(ss, rec.groom_birth, ',');
        std::getline(ss, rec.bride_fio, ',');
        std::getline(ss, rec.bride_birth, ',');
        std::getline(ss, rec.marriage_date, ',');
        std::getline(ss, token, ',');
        rec.zags_number = std::stoi(token);
        rec.marriage_date_int = dateToInt(rec.marriage_date);
        result.push_back(rec);
        ++rowsRead;
    }
    return result;
}

//  1. ЛИНЕЙНЫЙ
std::vector<ZagsRecord> linearSearch(
    const std::vector<ZagsRecord>& data, const std::string& key)
{
    std::vector<ZagsRecord> res;
    for (const auto& r : data)
        if (r.groom_fio == key) res.push_back(r);
    return res;
}

//  2. БИНАРНОЕ ДЕРЕВО
struct BSTNode {
    std::string             key;
    std::vector<ZagsRecord> records;
    BSTNode* left = nullptr;
    BSTNode* right = nullptr;
    explicit BSTNode(const std::string& k) : key(k) {}
};

class BST {
public:
    BST() = default;
    ~BST() { destroy(root); }
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    void insert(const ZagsRecord& rec) {
        if (!root) {
            root = new BSTNode(rec.groom_fio);
            root->records.push_back(rec);
            return;
        }
        BSTNode* cur = root;
        for (;;) {
            if (rec.groom_fio < cur->key) {
                if (!cur->left) { cur->left = new BSTNode(rec.groom_fio); cur->left->records.push_back(rec); return; }
                cur = cur->left;
            }
            else if (rec.groom_fio > cur->key) {
                if (!cur->right) { cur->right = new BSTNode(rec.groom_fio); cur->right->records.push_back(rec); return; }
                cur = cur->right;
            }
            else {
                cur->records.push_back(rec);
                return;
            }
        }
    }

    std::vector<ZagsRecord> search(const std::string& key) const {
        BSTNode* cur = root;
        while (cur) {
            if (key == cur->key) return cur->records;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return {};
    }

private:
    BSTNode* root = nullptr;
    void destroy(BSTNode* n) { 
        if (!n) return; 
        destroy(n->left); 
        destroy(n->right); 
        delete n; }
};

//  3. КРАСНО-ЧЁРНОЕ ДЕРЕВО
enum class RBColor : bool { RED = true, BLACK = false };

struct RBNode {
    std::string             key;
    std::vector<ZagsRecord> records;
    RBColor                 color = RBColor::RED;
    RBNode* left = nullptr;
    RBNode* right = nullptr;
    RBNode* parent = nullptr;
    explicit RBNode(const std::string& k) : key(k) {}
};

class RBTree {
public:
    RBTree() {
        nil = new RBNode("");
        nil->color = RBColor::BLACK;
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }
    ~RBTree() { destroyTree(root); delete nil; }
    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;

    void insert(const ZagsRecord& rec) {
        RBNode* ex = findNode(rec.groom_fio);
        if (ex != nil) { ex->records.push_back(rec); return; }

        RBNode* z = new RBNode(rec.groom_fio);
        z->records.push_back(rec);
        z->left = z->right = nil;
        z->color = RBColor::RED;

        RBNode* y = nil, * x = root;
        while (x != nil) { y = x; x = (z->key < x->key) ? x->left : x->right; }
        z->parent = y;
        if (y == nil)           root = z;
        else if (z->key < y->key)    y->left = z;
        else                         y->right = z;
        fixInsert(z);
    }

    std::vector<ZagsRecord> search(const std::string& key) const {
        RBNode* n = findNode(key);
        return (n != nil) ? n->records : std::vector<ZagsRecord>{};
    }
     
private:
    RBNode* nil;
    RBNode* root;

    RBNode* findNode(const std::string& key) const {
        RBNode* cur = root;
        while (cur != nil) {
            if (key == cur->key) return cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return nil;
    }

    void rotateLeft(RBNode* x) {
        RBNode* y = x->right; x->right = y->left;
        if (y->left != nil) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nil)       root = y;
        else if (x == x->parent->left)   x->parent->left = y;
        else                             x->parent->right = y;
        y->left = x; x->parent = y;
    }

    void rotateRight(RBNode* y) {
        RBNode* x = y->left; y->left = x->right;
        if (x->right != nil) x->right->parent = y;
        x->parent = y->parent;
        if (y->parent == nil)       root = x;
        else if (y == y->parent->left)   y->parent->left = x;
        else                             y->parent->right = x;
        x->right = y; y->parent = x;
    }

    void fixInsert(RBNode* z) {
        while (z->parent->color == RBColor::RED) {
            if (z->parent == z->parent->parent->left) {
                RBNode* u = z->parent->parent->right;
                if (u->color == RBColor::RED) {
                    z->parent->color = u->color = RBColor::BLACK;
                    z->parent->parent->color = RBColor::RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->right) { z = z->parent; rotateLeft(z); }
                    z->parent->color = RBColor::BLACK;
                    z->parent->parent->color = RBColor::RED;
                    rotateRight(z->parent->parent);
                }
            }
            else {
                RBNode* u = z->parent->parent->left;
                if (u->color == RBColor::RED) {
                    z->parent->color = u->color = RBColor::BLACK;
                    z->parent->parent->color = RBColor::RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->left) { z = z->parent; rotateRight(z); }
                    z->parent->color = RBColor::BLACK;
                    z->parent->parent->color = RBColor::RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root->color = RBColor::BLACK;
    }

    void destroyTree(RBNode* n) {
        if (!n || n == nil) return;
        destroyTree(n->left); destroyTree(n->right); delete n;
    }
};

//  4. ХЕШ-ТАБЛИЦА
struct HNode {
    std::string             key;
    std::vector<ZagsRecord> records;
    HNode* next = nullptr;
    explicit HNode(const std::string& k) : key(k) {}
};

class HashTable {
public:
    static constexpr std::size_t CAP = 1'000'003ULL;
    static constexpr std::size_t BASE = 31ULL;

    HashTable() : table(new HNode* [CAP]()), collisions(0) {
        for (std::size_t i = 0; i < CAP; ++i) table[i] = nullptr;
    }
    ~HashTable() {
        for (std::size_t i = 0; i < CAP; ++i) {
            HNode* cur = table[i];
            while (cur) { HNode* t = cur->next; delete cur; cur = t; }
        }
        delete[] table;
    }
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const ZagsRecord& rec) {
        std::size_t idx = hashStr(rec.groom_fio);
        HNode* cur = table[idx];
        while (cur) {
            if (cur->key == rec.groom_fio) { cur->records.push_back(rec); return; }
            cur = cur->next;
        }
        if (table[idx]) ++collisions;
        HNode* node = new HNode(rec.groom_fio);
        node->records.push_back(rec);
        node->next = table[idx];
        table[idx] = node;
    }

    std::vector<ZagsRecord> search(const std::string& key) const {
        HNode* cur = table[hashStr(key)];
        while (cur) {
            if (cur->key == key) return cur->records;
            cur = cur->next;
        }
        return {};
    }

    long long getCollisions() const { return collisions; }

private:
    HNode** table;
    long long collisions;

    std::size_t hashStr(const std::string& s) const {
        std::size_t h = 0, p = 1;
        for (unsigned char c : s) {
            h = (h + static_cast<std::size_t>(c) * p) % CAP;
            p = (p * BASE) % CAP;
        }
        return h;
    }
};

int main() {
    std::vector<ZagsRecord> fullData = loadCSV("zags_data.csv");
    if (fullData.empty()) {
        std::cerr << "Ошибка: не удалось загрузить zags_data.csv или файл пуст.\n";
        return 1;
    }
    std::cout << "Загружено " << fullData.size() << " записей.\n";

    const std::vector<int> sizes = {
        2000, 3000, 5000, 7500, 10000, 15000,
        20000, 50000, 100000, 200000, 300000
    };

    std::ofstream timesOut("search_times.csv");
    if (!timesOut.is_open()) {
        std::cerr << "Ошибка: не удалось открыть search_times.csv для записи.\n";
        return 1;
    }
    timesOut << "N,linear_ns,bst_ns,rbtree_ns,hash_ns,multimap_ns\n";

    std::ofstream collOut("hash_collisions.csv");
    if (!collOut.is_open()) {
        std::cerr << "Ошибка: не удалось открыть hash_collisions.csv для записи.\n";
        return 1;
    }
    collOut << "N,collisions\n";

    const std::string sep(90, '-');
    std::cout << "\n"
        << std::left
        << std::setw(8) << "N"
        << std::setw(14) << "linear_ns"
        << std::setw(14) << "bst_ns"
        << std::setw(14) << "rbtree_ns"
        << std::setw(14) << "hash_ns"
        << std::setw(16) << "multimap_ns"
        << std::setw(12) << "collisions"
        << "\n" << sep << "\n";

    std::mt19937 rng(2024);

    for (int N : sizes) {
        if (N > static_cast<int>(fullData.size())) {
            std::cerr << "Пропуск N=" << N << ": датасет содержит только "
                << fullData.size() << " записей.\n";
            continue;
        }

        std::vector<ZagsRecord> base(fullData.begin(), fullData.begin() + N);

        std::uniform_int_distribution<int> dist(0, N - 1);
        std::vector<std::string> keys(10);
        for (auto& k : keys) k = base[dist(rng)].groom_fio;

        BST       bst;
        RBTree    rbt;
        HashTable ht;
        std::multimap<std::string, ZagsRecord> mmap;
        for (const auto& rec : base) {
            bst.insert(rec);
            rbt.insert(rec);
            ht.insert(rec);
            mmap.insert({ rec.groom_fio, rec });
        }

        auto measure = [&](auto searchFn) -> long long {
            long long total = 0;
            for (const auto& key : keys) {
                auto t0 = std::chrono::high_resolution_clock::now();
                searchFn(key);
                auto t1 = std::chrono::high_resolution_clock::now();
                total += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            }
            return total / 10;
            };

        long long linAvg = measure([&](const std::string& k) { linearSearch(base, k); });
        long long bstAvg = measure([&](const std::string& k) { bst.search(k); });
        long long rbtAvg = measure([&](const std::string& k) { rbt.search(k); });
        long long htAvg = measure([&](const std::string& k) { ht.search(k); });
        long long mmAvg = measure([&](const std::string& k) {
            auto range = mmap.equal_range(k);
            std::vector<ZagsRecord> tmp;
            for (auto it = range.first; it != range.second; ++it)
                tmp.push_back(it->second);
            });
        long long cols = ht.getCollisions();

        timesOut << N << ','
            << linAvg << ',' << bstAvg << ','
            << rbtAvg << ',' << htAvg << ',' << mmAvg << '\n';
        collOut << N << ',' << cols << '\n';

        std::cout << std::left
            << std::setw(8) << N
            << std::setw(14) << linAvg
            << std::setw(14) << bstAvg
            << std::setw(14) << rbtAvg
            << std::setw(14) << htAvg
            << std::setw(16) << mmAvg
            << std::setw(12) << cols
            << '\n';
    }

    timesOut.close();
    collOut.close();
    std::cout << sep << "\nГотово. Результаты записаны в search_times.csv и hash_collisions.csv\n";
    return 0;
}