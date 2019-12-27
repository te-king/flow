//
// Created by Thomas King on 26/12/19.
//

#pragma once

#include <sstream>
#include <functional>
#include <optional>



// -- Workaround for lack of optional reference in cpp17

namespace std {

    template<typename _T>
    class optional<_T const &> : public std::optional<_T const *> {

    public:

        using std::optional<_T const *>::optional;

        optional(_T const &value) : optional<_T const *>(&value) {}


        _T const &value() const {
            return *std::optional<_T const *>::value();
        }

    };

}


template<typename _T>
class flow : private std::function<std::optional<_T>()> {

public:


    // -- Use std::function constructor and invoke operator

    using std::function<std::optional<_T>()>::function;

    using std::function<std::optional<_T>()>::operator();


    // -- For Each Loops

    void for_each(std::function<void(_T)> const &fn) const {
        auto it = *this;
        while (auto &&item = it()) fn(item.value());
    }


    void for_each_indexed(std::function<void(_T, size_t)> const &fn) const {
        auto it = *this;
        size_t counter = 0;
        while (auto &&item = it()) fn(item.value(), counter++);
    }


    // -- Flow Filtering

    flow<_T> filter(std::function<bool(_T)> const &pred) const {

        auto it = *this;

        return [=]() mutable -> std::optional<_T> {
            while (auto &&item = it())
                if (pred(item.value())) return item.value();
            return std::nullopt;
        };

    }


    flow<_T> filter_not(std::function<bool(_T)> const &pred) const {

        auto it = *this;

        return [=]() mutable -> std::optional<_T> {
            while (auto &&item = it())
                if (!pred(item.value())) return item.value();
            return std::nullopt;
        };

    }


    // -- Flow Mapping


    template<typename _M, typename _ = _T, std::enable_if_t<std::is_class_v<_>, int> = 0>
    flow<_M> map(_M _::*member) const {

        auto it = *this;

        return [=]() mutable -> std::optional<_M> {
            if (auto &&item = it()) return item.value().*member;
            return std::nullopt;
        };

    }


    template<typename _T1>
    flow<_T1> map(std::function<_T1(_T)> const &mapper) const {

        auto it = *this;

        return [=]() mutable -> std::optional<_T1> {
            if (auto &&item = it()) return mapper(item.value());
            return std::nullopt;
        };

    }


    template<typename _T1>
    flow<_T1> map_indexed(std::function<_T1(_T, size_t)> const &mapper) const {

        auto it = *this;
        size_t counter = 0;

        return [=]() mutable -> std::optional<_T1> {
            if (auto &&item = it()) return mapper(item.value(), counter++);
            return std::nullopt;
        };

    }


    template<typename _T1>
    flow<_T1> map_to() const {

        auto it = *this;

        return [=]() mutable -> std::optional<_T1> {
            if (auto &&item = it()) return _T1(item.value());
            return std::nullopt;
        };

    }


    // -- Constraints

    flow<_T> take(size_t count) const {

        auto it = *this;
        size_t ctr = 0;

        return [=]() mutable -> std::optional<_T> {
            if (ctr++ >= count) return std::nullopt;
            return it();
        };

    }


    flow<_T> skip(size_t count) const {

        auto it = *this;
        size_t ctr = 0;

        return [=]() mutable -> std::optional<_T> {
            while (ctr++ < count && it());
            return it();
        };

    }


    // -- Attachments

    flow<_T> then(flow<_T> const &other) const {

        auto it0 = *this;
        auto it1 = other;
        bool second = false;

        return [=]() mutable -> std::optional<_T> {
            if (!second) {
                if (auto &&item = it0()) return item;
                else second = true;
            }
            return it1();
        };

    }


    flow<_T> interleave(flow<_T> const &other) const {

        auto it0 = *this;
        auto it1 = other;
        bool second = false;

        return [=]() mutable -> std::optional<_T> {
            if ((second = !second)) return it0();
            return it1();
        };

    }


    // -- Utilities

    size_t count() const {
        auto it = *this;
        size_t counter = 0;
        while (it()) ++counter;
        return counter;
    }

    bool any(std::function<bool(_T)> const &pred) const {
        auto it = *this;
        while (auto &&item = it())
            if (pred(item.value())) return true;
        return false;
    }

    bool all(std::function<bool(_T)> const &pred) const {
        auto it = *this;
        while (auto &&item = it())
            if (!pred(item.value())) return false;
        return true;
    }


    // -- Container generation

    using _TV = std::remove_cv_t<std::remove_reference_t<_T>>;


    std::vector<_TV> to_vector() const {
        std::vector<_TV> result;
        auto it = *this;
        while (auto &&item = it()) result.push_back(item.value());
        return result;
    }


    std::set<_TV> to_set() const {
        std::set<_TV> result;
        auto it = *this;
        while (auto &&item = it()) result.insert(item.value());
        return result;
    }


    std::string join_to_string(char const *sep = ", ") const {
        std::stringstream ss;
        auto it = *this;
        if (auto &&item = it()) ss << item.value();
        while (auto &&item = it()) ss << sep << item.value();
        return ss.str();
    }

};


// Wraps an stl container
template<typename _C>
flow<typename _C::value_type const &> from(_C const &container) {

    auto begin = container.cbegin(),
            end = container.cend();

    return [=]() mutable -> std::optional<typename _C::value_type const &> {
        if (begin == end) return std::nullopt;
        return *(begin++);
    };

}

// Represents a range between two numbers
flow<size_t> range(size_t first, size_t last) {

    auto ctr = first;

    return [=]() mutable -> std::optional<size_t> {
        if (ctr > last) return std::nullopt;
        return ctr++;
    };

}
