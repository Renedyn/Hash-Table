#include <cstddef>
#include <cstdlib>
#include <utility>
#include <vector>
#include <iostream>
#include <cassert>
#include <exception>


template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    struct Element {
        std::pair<const KeyType, ValueType> key_value;
        int64_t cnt = -1;

        std::pair<const KeyType, ValueType>& get() {
            return key_value;
        }

        const std::pair<const KeyType, ValueType>& get() const {
            return key_value;
        }

        std::pair<KeyType&, ValueType&> ref() {
            return std::pair<KeyType&, ValueType&>(const_cast<KeyType&>(key_value.first), key_value.second);
        }

        Element& operator=(const Element& other) {
            ref().first = other.key_value.first;
            ref().second = other.key_value.second;
            return *this;
        }
    };

    typedef typename std::vector<Element>::iterator vec_iter;
    typedef typename std::vector<Element>::const_iterator const_vec_iter;

    
    class iterator { 
    public:
        explicit iterator(vec_iter pointer) : ptr_(pointer) {}

        iterator() : ptr_(nullptr) {}

        std::pair<const KeyType, ValueType>& operator*() {
            return ptr_->get();
        }

        std::pair<const KeyType, ValueType>* operator->() {
            return &(ptr_->get());
        }

        iterator operator++() {
            for (++ptr_; ; ++ptr_) {
                if (ptr_->cnt != -1) {
                    return *this;
                }
            }
        }

        iterator operator++(int) {
            iterator cp = *this;
            for (++ptr_; ; ++ptr_) {
                if (ptr_->cnt != -1) {
                    break;
                }
            }
            return cp;
        }

        bool operator==(const iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const iterator& other) const {
            return ptr_ != other.ptr_;
        }

    private:
        vec_iter ptr_;
        friend HashMap;
    };
    
    class const_iterator { 
    public:
        explicit const_iterator(const_vec_iter pointer) : ptr_(pointer) {}

        const_iterator() : ptr_(nullptr) {}

        const std::pair<const KeyType, ValueType>& operator*() const {
            return ptr_->get();
        }

        const std::pair<const KeyType, ValueType>* operator->() const {
            return &(ptr_->get());
        }

        const_iterator operator++() {
            for (++ptr_; ; ++ptr_) {
                if (ptr_->cnt != -1) {
                    return *this;
                }
            }
        }

        const_iterator operator++(int) {
            const_iterator cp = *this;
            for (++ptr_; ; ++ptr_) {
                if (ptr_->cnt != -1) {
                    break;
                }
            }
            return cp;
        }

        bool operator==(const const_iterator& other) const {
            return ptr_ == other.ptr_;
        }
        bool operator!=(const const_iterator& other) const {
            return ptr_ != other.ptr_;
        }

    private:
        const_vec_iter ptr_;
        friend HashMap;
    };

    iterator begin() {
        for (size_t it = 0; it < data_.size(); ++it) {
            if (data_[it].cnt >= 0) {
                return iterator(data_.begin() + it);
            }
        }
        return iterator(data_.end() - 1);
    }

    iterator end() {
        return iterator(data_.end() - 1);
    }
    
    const_iterator begin() const {
        for (size_t it = 0; it < data_.size(); ++it) {
            if (data_[it].cnt >= 0) {
                return const_iterator(data_.begin() + it);
            }
        }
        return const_iterator(data_.end() - 1);
    }

    const_iterator end() const {
        return const_iterator(data_.end() - 1);
    }
    

    explicit HashMap(const Hash &_hasher = Hash()) : hasher_(_hasher) {
        size_ = 0;
        data_.resize(1);
        data_[0].cnt = -2;
    }
    
    template<class It>
    HashMap(const It &beg, const It &ed, Hash _hasher = Hash()): hasher_(_hasher)  {
        size_ = 0;
        data_.resize(1);
        data_[0].cnt = -2;
        for (auto iter = beg; iter != ed; ++iter) {
            insert(*iter);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash _hasher = Hash()) : hasher_(_hasher)  {
        size_ = 0;
        data_.resize(1);
        data_[0].cnt = -2;
        //data_.resize(2LL << UpperPow2(list.size()));
        for (const std::pair<KeyType, ValueType> &element : list) {
            insert(element);
        }
    }

    iterator find(const KeyType &key) {
        if (data_.size() <= 1) {
            return end();
        }
        size_t hsh = hasher_(key) & (data_.size() - 2);
        int64_t welfare = 0;
        for (size_t i = hsh; ; i = (i + 1) & (data_.size() - 2)) {
            if (data_[i].cnt >= 0 && data_[i].get().first == key) {
                return iterator(data_.begin() + i);
            }
            if (welfare > data_[i].cnt) {
                return end();
            }
            ++welfare;
        }
    }

    const_iterator find(const KeyType &key) const {
        if (data_.size() <= 1) {
            return end();
        }
        size_t hsh = hasher_(key) & (data_.size() - 2);
        int64_t welfare = 0;
        for (size_t i = hsh; ; i = (i + 1) & (data_.size() - 2)) {
            if (data_[i].cnt >= 0 && data_[i].get().first == key) {
                return const_iterator(data_.begin() + i);
            }
            if (welfare > data_[i].cnt) {
                return end();
            }
            ++welfare;
        }
    }

    void insert(const std::pair<KeyType, ValueType> &element, bool internal_insert = false) {
        if (!internal_insert && find(element.first) != end()) {
            return;
        }
        if (data_.size() <= 2 || (size_ + .0) / (data_.size() - 1) >= max_loadfactor_) {
            if (data_.size() == 1) {
                data_.resize(2);
                data_[0].cnt = -1;
                data_[1].cnt = -2;
            } else {
                std::vector<Element> nw_;
                nw_.reserve((data_.size() - 1) * 2 + 1);
                nw_.resize((data_.size() - 1) * 2 + 1);
                std::swap(nw_, data_);
                data_.back().cnt = -2;
                size_ = 0;
                for (auto i : nw_) {
                    if (i.cnt >= 0) {
                        insert(i.get(), true);
                    }
                }
            }
        }
        ++size_;

        size_t hsh = hasher_(element.first) & (data_.size() - 2);
        int64_t welfare = 0;
        std::pair<KeyType, ValueType> to_insert = element;
        for (size_t i = hsh; ; i = (i + 1) & (data_.size() - 2)) {
            if (data_[i].cnt == -1) {
                std::swap(data_[i].ref().first, to_insert.first);
                std::swap(data_[i].ref().second, to_insert.second);
                data_[i].cnt = welfare;
                break;
            } else if (data_[i].cnt < welfare) {
                std::swap(data_[i].ref().first, to_insert.first);
                std::swap(data_[i].ref().second, to_insert.second);
                std::swap(data_[i].cnt, welfare);
            }
            ++welfare;
        }
    }

    void erase(const KeyType &key) { 
        auto pos_it = find(key);
        if (pos_it == end()) {
            return;
        }

        size_t pos = pos_it.ptr_ - data_.begin();

        if (data_[pos].cnt >= 0 && data_[pos].get().first == key) {
            data_[pos].cnt = -1;
            for (; data_[(pos + 1) & (data_.size() - 2)].cnt > 0; pos = (pos + 1) & (data_.size() - 2)) {
                size_t nxt = (pos + 1) & (data_.size() - 2);
                --data_[nxt].cnt;
                std::swap(data_[pos].ref().first, data_[nxt].ref().first);
                std::swap(data_[pos].ref().second, data_[nxt].ref().second);
                std::swap(data_[pos].cnt, data_[nxt].cnt);
            }
        }
        
        --size_;

        if (size_ > 0 && (size_ + .0) / (data_.size() - 1) <= min_loadfactor_) {
            std::vector<Element> nw_;
            nw_.reserve((data_.size() - 1) / 2 + 1);
            nw_.resize((data_.size() - 1) / 2 + 1);
            std::swap(nw_, data_);
            data_.back().cnt = -2;
            size_ = 0;
            for (auto i : nw_) {
                if (i.cnt >= 0) {
                    insert(i.get(), true);
                }
            }
        }
    }

    size_t size() const { 
        return size_;
    }

    bool empty() const { 
        return !size();
    }

    ValueType& operator[](const KeyType &key) {
        auto it = find(key);
        if (it == end()) {
            insert({key, ValueType()});
            return find(key)->second;
        } else {
            return it->second;
        }
    }

    const ValueType& at(const KeyType &key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("key does not exist, bruh");
        }
        return it->second;
    }

    void clear() { 
        data_.clear();
        size_ = 0;
        data_.resize(1);
        data_[0].cnt = -2;
    }   

    Hash hash_function() const {
        return hasher_;
    }
    

private:
    Hash hasher_;
    size_t size_;
    std::vector<Element> data_;
    constexpr static double min_loadfactor_ = 0.20;
    constexpr static double max_loadfactor_ = 0.80;
};
