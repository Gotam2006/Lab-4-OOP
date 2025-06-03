  #include <iostream>
#include <stdexcept>
#include <algorithm>
#include <iterator>

// Абстрактний клас-трансформер (динамічний поліморфізм)
template<typename T>
struct Transformer {
    virtual T operator()(const T&) const = 0;
    virtual ~Transformer() = default;
};

template<typename T>
class String {
private:
    T* data;
    size_t length;

    void copyFrom(const T* src, size_t len) {
        data = new T[len];
        std::copy(src, src + len, data);
        length = len;
    }

public:
    // --- Конструктори ---
    String() : data(nullptr), length(0) {}

    String(const String& other) {
        copyFrom(other.data, other.length);
    }

    String(String&& other) noexcept : data(other.data), length(other.length) {
        other.data = nullptr;
        other.length = 0;
    }

    String(size_t count, T ch) {
        data = new T[count];
        std::fill(data, data + count, ch);
        length = count;
    }

    String(const T* str) {
        size_t len = 0;
        while (str[len] != T()) len++;
        copyFrom(str, len);
    }

    String(const T* begin, const T* end) {
        if (begin > end) throw std::invalid_argument("Begin > End");
        copyFrom(begin, end - begin);
    }

    template<typename U>
    String(const String<U>& other) {
        data = new T[other.size()];
        std::transform(other.begin(), other.end(), data, [](const U& ch) {
            return static_cast<T>(ch);
        });
        length = other.size();
    }

    // --- Деструктор ---
    ~String() {
        delete[] data;
    }

    // --- Оператори присвоєння ---
    String& operator=(const String& other) {
        if (this != &other) {
            delete[] data;
            copyFrom(other.data, other.length);
        }
        return *this;
    }

    String& operator=(String&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            length = other.length;
            other.data = nullptr;
            other.length = 0;
        }
        return *this;
    }

    // --- Методи ---
    size_t size() const { return length; }
    bool empty() const { return length == 0; }

    void clear() {
        delete[] data;
        data = nullptr;
        length = 0;
    }

    T& operator[](size_t index) {
        if (index >= length) throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= length) throw std::out_of_range("Index out of range");
        return data[index];
    }

    // Підрядок
    String substring(size_t start, size_t len) const {
        if (start > length) throw std::out_of_range("Start out of range");
        size_t actual_len = std::min(len, length - start);
        return String(data + start, data + start + actual_len);
    }

    // --- Оператори конкатенації ---
    String operator+(const String& rhs) const {
        String result;
        result.length = length + rhs.length;
        result.data = new T[result.length];
        std::copy(data, data + length, result.data);
        std::copy(rhs.data, rhs.data + rhs.length, result.data + length);
        return result;
    }

    String& operator+=(const T& ch) {
        T* new_data = new T[length + 1];
        std::copy(data, data + length, new_data);
        new_data[length] = ch;
        delete[] data;
        data = new_data;
        ++length;
        return *this;
    }

    // --- Повторення рядка ---
    friend String operator*(const String& str, int n) {
        if (n <= 0) return String();
        String result;
        result.length = str.length * n;
        result.data = new T[result.length];
        for (int i = 0; i < n; ++i)
            std::copy(str.data, str.data + str.length, result.data + i * str.length);
        return result;
    }

    friend String operator*(int n, const String& str) {
        return str * n;
    }

    // --- Порівняння ---
    bool operator==(const String& rhs) const {
        if (length != rhs.length) return false;
        return std::equal(data, data + length, rhs.data);
    }

    bool operator!=(const String& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const String& rhs) const {
        return std::lexicographical_compare(data, data + length, rhs.data, rhs.data + rhs.length);
    }

    bool operator<=(const String& rhs) const {
        return !(rhs < *this);
    }

    bool operator>(const String& rhs) const {
        return rhs < *this;
    }

    bool operator>=(const String& rhs) const {
        return !(*this < rhs);
    }

    // --- Трансформації (динамічний поліморфізм) ---
    void apply(const Transformer<T>& transformer) {
        for (size_t i = 0; i < length; ++i) {
            data[i] = transformer(data[i]);
        }
    }

    // --- Трансформації (статичний поліморфізм) ---
    template <typename Trans>
    void modify(const Trans& t) {
        for (size_t i = 0; i < length; ++i) {
            data[i] = t(data[i]);
        }
    }

    // --- Ітератори для зручності ---
    T* begin() { return data; }
    T* end() { return data + length; }
    const T* begin() const { return data; }
    const T* end() const { return data + length; }

    // --- Вивід у потік ---
    friend std::ostream& operator<<(std::ostream& os, const String& str) {
        for (size_t i = 0; i < str.length; ++i)
            os << str.data[i];
        return os;
    }
};

// --- Тестування ---
struct ToUpper : Transformer<char> {
    char operator()(const char& c) const override {
        return std::toupper(static_cast<unsigned char>(c));
    }
};

int main() {
    String<char> s1("hello");
    String<char> s2(" world");
    String<char> s3 = s1 + s2;
    std::cout << "s3: " << s3 << "\n"; // hello world

    s3 += '!';
    std::cout << "s3 + '!': " << s3 << "\n"; // hello world!

    String<char> s4 = s3 * 2;
    std::cout << "s3 * 2: " << s4 << "\n"; // hello world!hello world!

    s4.apply(ToUpper());
    std::cout << "Uppercase: " << s4 << "\n"; // HELLO WORLD!HELLO WORLD!

    String<char> sub = s4.substring(6, 5);
    std::cout << "Substring: " << sub << "\n"; // WORLD

    return 0;
}
               