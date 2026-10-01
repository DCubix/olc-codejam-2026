#pragma once

#include <cassert>
#include <cstddef>

template <typename T, size_t N>
class Array {
public:
    Array() = default;
    ~Array() = default;

    Array(const Array&) = default;
    Array& operator=(const Array&) = default;

    Array(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        for (size_t i = 0; i < other.m_count; i++) {
            m_data[i] = std::move(other.m_data[i]);
        }
        m_count = other.m_count;
        other.m_count = 0;
    }

    Array& operator=(Array&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            for (size_t i = 0; i < other.m_count; i++) {
                m_data[i] = std::move(other.m_data[i]);
            }
            m_count = other.m_count;
            other.m_count = 0;
        }
        return *this;
    }

    T& operator[](size_t i) { return m_data[i]; }
    const T& operator[](size_t i) const { return m_data[i]; }
    bool operator==(const Array& rhs) const {
        return m_count == rhs.m_count &&
               std::equal(m_data, m_data + m_count, rhs.m_data);
    }
    size_t Size() const { return m_count; }

    bool IsEmpty() const { return m_count == 0; }
    bool IsFull() const { return m_count == N; }

    T* Data() { return m_data; }
    const T* Data() const { return m_data; }

    size_t Add(T el) {
        assert(m_count < N && "out of bounds");
        m_data[m_count++] = std::move(el);;
        return m_count-1;
    }

    // returns a reference to the next free slot instead of copying/moving a value in,
    // so T can be constructed in place at its final, stable address
    T& AddInPlace() {
        assert(m_count < N && "out of bounds");
        return m_data[m_count++];
    }

    void Remove(size_t index) {
        assert(m_count > 0 && index < m_count && "out of bounds");
        m_data[index] = std::move(m_data[m_count-1]);
        m_data[m_count-1] = T{};
        m_count--;
    }

    void Insert(T el, size_t at) {
        assert(at <= m_count && m_count < N && "out of bounds");
        for (size_t i = m_count; i > at; --i)
            m_data[i] = std::move(m_data[i - 1]);
        m_data[at] = std::move(el);
        m_count++;
    }

    T Pop() {
        assert(m_count > 0 && "empty");
        m_count--;
        return std::move(m_data[m_count]);
    }

    T Shift() {
        assert(m_count > 0 && "empty");
        T front = std::move(m_data[0]);
        for (size_t i = 0; i < m_count - 1; ++i)
            m_data[i] = std::move(m_data[i + 1]);
        m_count--;
        m_data[m_count] = T{};
        return front;
    }

    void Clear() {
        for (size_t i = 0; i < m_count; i++) m_data[i] = T{};
        m_count = 0;
    }

    T* begin() { return m_data; }
    T* end() { return m_data + m_count; }
    const T* begin() const { return m_data; }
    const T* end() const { return m_data + m_count; }

private:
    size_t m_count{ 0 };
    T m_data[N]{};
};
