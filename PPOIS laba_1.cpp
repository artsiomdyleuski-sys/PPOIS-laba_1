#include "Header.h"
#include <stdexcept>
#include <cctype>
#include <sstream>

Set::Set() : allowDuplicates(false) {}

Set::Set(bool allowDup) : allowDuplicates(allowDup) {}

Set::Set(const std::string& str, bool allowDup)
    : allowDuplicates(allowDup) {
    parseFromString(str);
}

bool Set::isEmpty() const { return elements.empty(); }

std::size_t Set::cardinality() const { return elements.size(); }

bool Set::sameElement(const Element& a, const Element& b) const {
    if (a.index() != b.index()) return false;
    if (a.index() == 0) return std::get<char>(a) == std::get<char>(b);
    const auto& sa = std::get<std::shared_ptr<Set>>(a);
    const auto& sb = std::get<std::shared_ptr<Set>>(b);
    return sa->equals(*sb);
}

void Set::add(const Element& e) {
    if (!allowDuplicates && contains(e)) return;
    if (e.index() == 1) {
        auto copy = std::make_shared<Set>(*std::get<std::shared_ptr<Set>>(e));
        elements.push_back(copy);
    }
    else {
        elements.push_back(e);
    }
}

void Set::remove(const Element& e) {
    auto it = std::remove_if(elements.begin(), elements.end(),
        [&](const Element& x) { return sameElement(x, e); });
    elements.erase(it, elements.end());
}

bool Set::contains(const Element& e) const {
    for (const auto& x : elements)
        if (sameElement(x, e)) return true;
    return false;
}

Set Set::operator+(const Set& other) const {
    Set result(allowDuplicates);
    for (const auto& e : elements) result.add(e);
    for (const auto& e : other.elements) result.add(e);
    return result;
}

Set Set::operator*(const Set& other) const {
    Set result(allowDuplicates);

    if (!allowDuplicates) {
        for (const auto& e : elements)
            if (other.contains(e)) result.add(e);
        return result;
    }

    for (std::size_t i = 0; i < elements.size(); ++i) {
        const auto& e = elements[i];

        bool seen = false;
        for (std::size_t j = 0; j < i; ++j)
            if (sameElement(elements[j], e)) { seen = true; break; }
        if (seen) continue;

        std::size_t c1 = 0, c2 = 0;
        for (const auto& x : elements) if (sameElement(x, e)) ++c1;
        for (const auto& x : other.elements) if (sameElement(x, e)) ++c2;

        std::size_t m = std::min(c1, c2);
        for (std::size_t k = 0; k < m; ++k) result.add(e);
    }
    return result;
}

Set Set::operator-(const Set& other) const {
    Set result(allowDuplicates);

    if (!allowDuplicates) {
        for (const auto& e : elements)
            if (!other.contains(e)) result.add(e);
        return result;
    }

    for (std::size_t i = 0; i < elements.size(); ++i) {
        const auto& e = elements[i];

        bool seen = false;
        for (std::size_t j = 0; j < i; ++j)
            if (sameElement(elements[j], e)) { seen = true; break; }
        if (seen) continue;

        std::size_t c1 = 0, c2 = 0;
        for (const auto& x : elements) if (sameElement(x, e)) ++c1;
        for (const auto& x : other.elements) if (sameElement(x, e)) ++c2;

        std::size_t keep = (c1 > c2) ? (c1 - c2) : 0;
        for (std::size_t k = 0; k < keep; ++k) result.add(e);
    }
    return result;
}

Set& Set::operator+=(const Set& other) {
    for (const auto& e : other.elements) add(e);
    return *this;
}

Set& Set::operator*=(const Set& other) {
    *this = *this * other;
    return *this;
}

Set& Set::operator-=(const Set& other) {
    *this = *this - other;
    return *this;
}

Set Set::powerSet() const {
    Set result(false);
    std::size_t n = elements.size();
    std::size_t total = std::size_t(1) << n;
    for (std::size_t mask = 0; mask < total; ++mask) {
        auto subset = std::make_shared<Set>(allowDuplicates);
        for (std::size_t i = 0; i < n; ++i)
            if (mask & (std::size_t(1) << i))
                subset->add(elements[i]);
        result.add(subset);
    }
    return result;
}

bool Set::equals(const Set& other) const {
    if (elements.size() != other.elements.size()) return false;
    for (const auto& e : elements)
        if (!other.contains(e)) return false;
    if (allowDuplicates || other.allowDuplicates) {
        for (const auto& e : elements) {
            std::size_t c1 = 0, c2 = 0;
            for (const auto& x : elements) if (sameElement(x, e)) ++c1;
            for (const auto& x : other.elements) if (sameElement(x, e)) ++c2;
            if (c1 != c2) return false;
        }
    }
    return true;
}

std::string Set::toString() const {
    std::ostringstream os;
    os << "{";
    for (std::size_t i = 0; i < elements.size(); ++i) {
        if (i) os << ", ";
        if (elements[i].index() == 0)
            os << std::get<char>(elements[i]);
        else
            os << std::get<std::shared_ptr<Set>>(elements[i])->toString();
    }
    os << "}";
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const Set& s) {
    return os << s.toString();
}

namespace {

    void skipWS(const std::string& s, std::size_t& i) {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    }

    Element parseElement(const std::string& s, std::size_t& i, bool allowDup) {
        skipWS(s, i);
        if (i >= s.size()) throw std::runtime_error("Unexpected end of input");

        if (s[i] == '{') {
            std::size_t start = i;
            int depth = 0;
            do {
                if (s[i] == '{') ++depth;
                else if (s[i] == '}') --depth;
                ++i;
            } while (i < s.size() && depth > 0);

            if (depth != 0) throw std::runtime_error("Unbalanced braces");
            std::string inner = s.substr(start, i - start);
            auto nested = std::make_shared<Set>(inner, allowDup);
            return Element{ nested };
        }
        else {
            char c = s[i++];
            return Element{ c };
        }
    }

}


void Set::parseFromString(const std::string& str) {
    std::size_t i = 0;
    skipWS(str, i);
    if (i >= str.size() || str[i] != '{')
        throw std::runtime_error("Set must start with '{'");
    ++i;

    while (true) {
        skipWS(str, i);
        if (i >= str.size()) throw std::runtime_error("Unterminated set");
        if (str[i] == '}') { ++i; break; }

        Element e = parseElement(str, i, allowDuplicates);
        add(e);

        skipWS(str, i);
        if (i < str.size() && str[i] == ',') { ++i; continue; }
        if (i < str.size() && str[i] == '}') { ++i; break; }
        throw std::runtime_error("Expected ',' or '}'");
    }
}


int main() {
    CantorSet a("{a, b, c, {a, b}, {}, {a, {c}}}");
    std::cout << "a = " << a << "\n";
    std::cout << "|a| = " << a.cardinality() << "\n";
    std::cout << "contains 'b' " << a['b'] << "\n";

    CantorSet b("{b, c, d}");
    std::cout << "a + b = " << (a + b) << "\n";
    std::cout << "a * b = " << (a * b) << "\n";
    std::cout << "a - b = " << (a - b) << "\n";

    MultiSet m("{a, a, c, {a, b, b}, {}, {a, {c, c}}}");
    std::cout << "m = " << m << "\n";
    std::cout << "|m| = " << m.cardinality() << "\n";

    CantorSet small("{a, b}");
    std::cout << "P(small) = " << small.powerSet() << "\n";

    return 0;
}