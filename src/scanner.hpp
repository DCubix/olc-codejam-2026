#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <functional>
#include <sstream>

template <typename Char = char>
using Delimiter = std::function<bool(Char)>;

template <typename Char = char>
class Scanner {
public:
    using str = std::basic_string<Char>;

    Scanner() = default;
    Scanner(const str& input) {
        m_input = std::vector<Char>(input.begin(), input.end());
    }

    Char Peek() const { if (m_input.empty()) return '\0'; return m_input[0]; }
    Char Read() {
        if (m_input.empty()) return '\0';
        Char tmp = m_input[0];
        m_input.erase(m_input.begin());
        return tmp;
    }

    bool IsEOF() const { return m_input.empty(); }

    void SkipWhitespace() {
        while (!IsEOF() && ::isspace(Peek())) Read();
    }

    template <typename T>
    T Token(const Delimiter<Char>& delim = TokenBoundary) {
        std::vector<Char> buf;
        SkipWhitespace();
        while (!IsEOF() && !delim(Peek())) buf.push_back(Read());
        str raw(buf.begin(), buf.end());
        std::basic_istringstream<Char> ss(raw);
        T value; ss >> value;
        return value;
    }

    inline static bool WhiteSpace(Char c) { return ::isspace(c); }
    // Numeric fields in the .stk format can be immediately followed by a
    // hierarchy marker with no separating space (e.g. "N 0 0 0 0<"), so the
    // default delimiter must stop there too, or '<'/'>' gets silently
    // swallowed into the number and the marker is never seen.
    inline static bool TokenBoundary(Char c) { return ::isspace(c) || c == '>' || c == '<'; }
private:
    std::vector<Char> m_input;
};
