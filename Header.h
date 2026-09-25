#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <algorithm>

class Set;

using Element = std::variant<char, std::shared_ptr<Set>>;

class Set {
protected:
    std::vector<Element> elements;
    bool allowDuplicates;

public:
    Set();
    explicit Set(bool allowDup);
    explicit Set(const std::string& str, bool allowDup = false);

    bool isEmpty() const;
    std::size_t cardinality() const;

    virtual void add(const Element& e);
    virtual void remove(const Element& e);

    bool contains(const Element& e) const;
    bool operator[](const Element& e) const { return contains(e); }
    bool operator[](char c) const { return contains(Element{ c }); }

    Set operator+(const Set& other) const;
    Set operator*(const Set& other) const;
    Set operator-(const Set& other) const;

    Set& operator+=(const Set& other);
    Set& operator*=(const Set& other);
    Set& operator-=(const Set& other);

    Set powerSet() const;

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Set& s);

    bool equals(const Set& other) const;
    bool operator==(const Set& other) const { return equals(other); }
    bool operator!=(const Set& other) const { return !equals(other); }

protected:
    bool sameElement(const Element& a, const Element& b) const;
    void parseFromString(const std::string& str);
};

class CantorSet : public Set {
public:
    CantorSet() : Set(false) {}
    explicit CantorSet(const std::string& str) : Set(str, false) {}
};

class MultiSet : public Set {
public:
    MultiSet() : Set(true) {}
    explicit MultiSet(const std::string& str) : Set(str, true) {}
};