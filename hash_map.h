//
// Created by Andrew Polikarpov on 28.01.2023.
//

#ifndef HASHMAP_HASH_MAP_H
#define HASHMAP_HASH_MAP_H

#endif //HASHMAP_HASH_MAP_H

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:

    class Element {
    public:
        Element() : nxt_(nullptr), prev_(nullptr),
                    elem_(std::make_pair(KeyType(), ValueType())), counter_(0) {};

        explicit Element(Element *begin) : nxt_(begin), prev_(nullptr),
                                           elem_(std::make_pair(KeyType(), ValueType())),
                                           counter_(0) {
            if (begin != nullptr) {
                begin->prev_ = this;
            }
        };

        Element(std::pair<const KeyType, ValueType> elem, Element *begin) : nxt_(begin),
                                                                            prev_(nullptr),
                                                                            elem_(elem),
                                                                            counter_(0) {
            if (begin != nullptr) {
                begin->prev_ = this;
            }
        };

        ~Element() {
            if (prev_ != nullptr) {
                prev_->nxt_ = nxt_;
            }
            if (nxt_ != nullptr) {
                nxt_->prev_ = prev_;
            }
        }

        Element *nxt_;
        Element *prev_;
        std::pair<const KeyType, ValueType> elem_;
        size_t counter_;
    };

    class iterator {
    public:
        iterator() : ptr_(nullptr) {};

        iterator(iterator &other) : ptr_(other.ptr_) {};

        explicit iterator(Element *elem) : ptr_(elem) {};

        std::pair<const KeyType, ValueType> &operator*() const {
            return ptr_->elem_;
        }

        std::pair<const KeyType, ValueType> *operator->() const {
            return &ptr_->elem_;
        }

        iterator &operator=(iterator other) {
            ptr_ = other.ptr_;
            return *this;
        }

        iterator &operator++() {
            ptr_ = ptr_->nxt_;
            return *this;
        }

        iterator operator++(int) {
            iterator cur(*this);
            ptr_ = ptr_->nxt_;
            return cur;
        }

        bool operator==(const iterator &other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const iterator &other) const {
            return ptr_ != other.ptr_;
        }

        Element *ptr_;
    };

    class const_iterator {
    public:
        const_iterator() : ptr_(nullptr) {};

        const_iterator(const_iterator &other) : ptr_(other.ptr_) {};

        explicit const_iterator(Element *elem) : ptr_(elem) {};

        const std::pair<const KeyType, ValueType> &operator*() const {
            return ptr_->elem_;
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return &ptr_->elem_;
        }

        const_iterator &operator=(const_iterator other) {
            ptr_ = other.ptr_;
            return *this;
        }

        const_iterator &operator++() {
            ptr_ = ptr_->nxt_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator cur(*this);
            ptr_ = ptr_->nxt_;
            return cur;
        }

        bool operator==(const const_iterator &other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const const_iterator &other) const {
            return ptr_ != other.ptr_;
        }

        Element *ptr_;
    };

    explicit HashMap(const Hash hash = Hash()) : hash_(hash) {
        size_ = 0;
        capacity_ = load_factor_;
        data_.assign(capacity_, nullptr);
        end_ = nullptr;
        begin_ = end_;
    }

    template<class Iter>
    HashMap(Iter begin, Iter end, const Hash hash = Hash()) : hash_(hash) {
        size_ = 0;
        capacity_ = load_factor_;
        data_.assign(capacity_, nullptr);
        end_ = nullptr;
        begin_ = end_;
        for (Iter cur = begin; cur != end; ++cur) {
            insert(*cur);
        }
    }

    HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>> &init, const Hash hash = Hash()) : hash_(
            hash) {
        size_ = 0;
        capacity_ = load_factor_;
        data_.assign(capacity_, nullptr);
        end_ = nullptr;
        begin_ = end_;
        for (const auto &cur: init) {
            insert(cur);
        }
    }

    ~HashMap() {
        while (begin_ != end_) {
            Element *nxt = begin_->nxt_;
            delete begin_;
            begin_ = nxt;
        }
        begin_ = nullptr;
        end_ = nullptr;
    }

    HashMap(HashMap &other) {
        std::vector<std::pair<KeyType, ValueType>> temp;
        for (auto cur: other) {
            temp.push_back(cur);
        }
        size_ = 0;
        capacity_ = load_factor_;
        data_.assign(capacity_, nullptr);
        end_ = nullptr;
        begin_ = end_;
        hash_ = other.hash_;
        for (const auto &cur: temp) {
            insert(cur);
        }
    }

    HashMap &operator=(HashMap &other) {
        std::vector<std::pair<KeyType, ValueType>> temp;
        for (auto cur: other) {
            temp.push_back(cur);
        }
        clear();
        capacity_ = load_factor_;
        data_.assign(capacity_, nullptr);
        end_ = nullptr;
        begin_ = end_;
        hash_ = other.hash_;
        for (const auto &cur: temp) {
            insert(cur);
        }
        return *this;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return hash_;
    }

    void insert(std::pair<const KeyType, ValueType> elem) {
        if (find(elem.first) != end())
            return;
        ++size_;
        auto *cur = new Element(elem, begin_);
        begin_ = cur;
        size_t pos = hash_(elem.first) % capacity_;
        while (data_[pos] != nullptr) {
            if (cur->counter_ > data_[pos]->counter_) {
                std::swap(cur, data_[pos]);
            }
            cur->counter_++;
            pos++;
            if (pos >= capacity_) {
                pos -= capacity_;
            }
        }
        data_[pos] = cur;
        if (size_ * load_factor_ > capacity_) {
            rebuild();
        }
    }

    void erase(const KeyType key) {
        size_t pos = hash_(key) % capacity_;
        bool found = false;
        while (data_[pos] != nullptr) {
            if (data_[pos]->elem_.first == key) {
                found = true;
                break;
            }
            pos++;
            if (pos >= capacity_) {
                pos -= capacity_;
            }
        }
        if (!found)
            return;
        size_--;
        if (begin_ == data_[pos]) {
            begin_ = begin_->nxt_;
        }
        delete data_[pos];
        data_[pos] = nullptr;
        pos++;
        if (pos >= capacity_) {
            pos -= capacity_;
        }
        while (data_[pos] != nullptr && data_[pos]->counter_ != 0) {
            size_t prev = pos;
            if (prev == 0)
                prev += capacity_;
            prev--;
            data_[pos]->counter_--;
            std::swap(data_[prev], data_[pos]);
            pos++;
            if (pos >= capacity_) {
                pos -= capacity_;
            }
        }
    }

    iterator find(const KeyType key) {
        size_t pos = hash_(key) % capacity_;
        while (data_[pos] != nullptr) {
            if (data_[pos]->elem_.first == key) {
                return iterator(data_[pos]);
            }
            pos++;
            if (pos >= capacity_) {
                pos -= capacity_;
            }
        }
        return end();
    }

    const_iterator find(const KeyType key) const {
        size_t pos = hash_(key) % capacity_;
        while (data_[pos] != nullptr) {
            if (data_[pos]->elem_.first == key) {
                return const_iterator(data_[pos]);
            }
            pos++;
            if (pos >= capacity_) {
                pos -= capacity_;
            }
        }
        return end();
    }

    ValueType &operator[](const KeyType key) {
        iterator cur = find(key);
        if (cur == end()) {
            insert(std::make_pair(key, ValueType()));
            return find(key)->second;
        }
        return cur->second;
    }

    const ValueType &at(const KeyType key) const {
        const_iterator cur = find(key);
        if (cur == end())
            throw std::out_of_range("");
        return cur->second;
    }

    void clear() {
        while (begin_ != end_) {
            Element *nxt = begin_->nxt_;
            erase(begin_->elem_.first);
            begin_ = nxt;
        }
    }

    iterator begin() {
        return iterator(begin_);
    }

    const_iterator begin() const {
        return const_iterator(begin_);
    }

    iterator end() {
        return iterator(end_);
    }

    const_iterator end() const {
        return const_iterator(end_);
    }

private:
    void rebuild() {
        std::vector<std::pair<KeyType, ValueType>> temp;
        for (auto cur: *this) {
            temp.push_back(cur);
        }
        clear();
        capacity_ *= 2;
        data_.assign(capacity_, nullptr);
        for (const auto &cur: temp) {
            insert(cur);
        }
    }

    size_t size_;
    size_t capacity_;
    Element *begin_;
    Element *end_;
    std::vector<Element *> data_;
    Hash hash_;
    static const size_t load_factor_ = 3;
};