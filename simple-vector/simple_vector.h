#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) : capacity_(capacity) {}

    size_t GetCapacity() {
        return capacity_;
    }

private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    // Констурктор копирования
    SimpleVector(const SimpleVector& other) : items_(other.capacity_), size_(other.size_) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    explicit SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.GetCapacity());
    }

    SimpleVector(SimpleVector&& other) {
        swap(other);
    }

    // Оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&items_ != &rhs.items_) {
            swap(SimpleVector<Type>(rhs));
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) throw std::out_of_range("Index is out of range");
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) throw std::out_of_range("Index is out of range");
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());

        size_t index = pos - cbegin();
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;
        return &items_[index];
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();

        if (capacity_ == 0) {
            ArrayPtr<Type> temp(1);
            items_.swap(temp);
            items_[0] = value;
            ++capacity_;
        }
        else {
            if (size_ < capacity_) {
                std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = value;
            }
            else {
                ArrayPtr<Type> temp(capacity_ * 2);
                std::copy(begin(), begin() + index, temp.Get());
                std::copy(begin() + index, end(), temp.Get() + index + 1);
                items_.swap(temp);
                items_[index] = value;
                capacity_ *= 2;
            }
        }

        ++size_;
        return &items_[index];
    }

    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();

        if (capacity_ == 0) {
            ArrayPtr<Type> temp(1);
            items_.swap(temp);
            items_[0] = std::move(value);
            ++capacity_;
        }
        else {
            if (size_ < capacity_) {
                std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = std::move(value);
            }
            else {
                ArrayPtr<Type> temp(capacity_ * 2);
                std::move(begin(), begin() + index, temp.Get());
                std::move(begin() + index, end(), temp.Get() + index + 1);
                items_.swap(temp);
                items_[index] = std::move(value);
                capacity_ *= 2;
            }
        }

        ++size_;
        return &items_[index];
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0 ? true : false;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {}
        else {
            if (capacity_ == 0) {
                ArrayPtr<Type> temp(1);
                items_.swap(temp);
                ++capacity_;
            }
            else {
                size_t new_capacity = capacity_ * 2;
                ArrayPtr<Type> temp(new_capacity);
                std::copy(begin(), end(), temp.Get());
                items_.swap(temp);
                capacity_ = new_capacity;
            }
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {}
        else {
            if (capacity_ == 0) {
                ArrayPtr<Type> temp(1);
                items_.swap(temp);
                ++capacity_;
            }
            else {
                size_t new_capacity = capacity_ * 2;
                ArrayPtr<Type> temp(new_capacity);
                std::move(begin(), end(), temp.Get());
                items_.swap(temp);
                capacity_ = new_capacity;
            }
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            std::copy(begin(), end(), temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_) {
            for (auto it = begin() + size_, it_end = begin() + new_size; it != it_end; ++it) {
                *it = std::move(Type{});
            }
            size_ = new_size;
            return;
        }

        if (new_size > capacity_ * 2) {
            ArrayPtr<Type> temp(new_size);
            std::move(begin(), end(), temp.Get());
            for (auto it = temp.Get() + size_, it_end = temp.Get() + new_size; it != it_end; ++it) {
                *it = std::move(Type{});
            }
            items_.swap(temp);
            size_ = new_size;
            capacity_ = new_size;
        }
        else {
            ArrayPtr<Type> temp(capacity_ * 2);
            std::move(begin(), end(), temp.Get());
            for (auto it = temp.Get() + size_, it_end = temp.Get() + new_size; it != it_end; ++it) {
                *it = std::move(Type{});
            }
            items_.swap(temp);
            size_ = new_size;
            capacity_ *= 2;
        }
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void swap(SimpleVector&& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs || lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}