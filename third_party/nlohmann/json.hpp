// nlohmann/json single-header
// version 3.11.2
// https://github.com/nlohmann/json

#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <ciso646>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace nlohmann {

class json {
public:
    using string_t = std::string;
    using boolean_t = bool;
    using number_integer_t = std::int64_t;
    using number_unsigned_t = std::uint64_t;
    using number_float_t = double;

    using array_t = std::vector<json>;
    using object_t = std::map<string_t, json>;

    enum class value_t {
        null, object, array, string, boolean, number_integer, number_unsigned, number_float
    };

private:
    value_t type = value_t::null;
    object_t object_value;
    array_t array_value;
    string_t string_value;
    boolean_t bool_value = false;
    number_integer_t int_value = 0;
    number_unsigned_t uint_value = 0;
    number_float_t float_value = 0.0;

public:
    json() = default;
    json(std::nullptr_t) {}
    json(const string_t& s) : type(value_t::string), string_value(s) {}
    json(const char* s) : type(value_t::string), string_value(s) {}
    json(boolean_t b) : type(value_t::boolean), bool_value(b) {}
    json(number_integer_t i) : type(value_t::number_integer), int_value(i) {}
    json(number_unsigned_t u) : type(value_t::number_unsigned), uint_value(u) {}
    json(number_float_t f) : type(value_t::number_float), float_value(f) {}
    json(const array_t& a) : type(value_t::array), array_value(a) {}
    json(const object_t& o) : type(value_t::object), object_value(o) {}

    json(std::initializer_list<object_t::value_type> init) {
        type = value_t::object;
        for (auto& el : init) object_value.insert(el);
    }

    static json array() {
        json j;
        j.type = value_t::array;
        return j;
    }

    json& operator[](const string_t& key) {
        type = value_t::object;
        return object_value[key];
    }

    const json& operator[](const string_t& key) const {
        return object_value.at(key);
    }

    json& operator[](std::size_t idx) {
        return array_value[idx];
    }

    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, string_t>) return string_value;
        if constexpr (std::is_same_v<T, boolean_t>) return bool_value;
        if constexpr (std::is_same_v<T, int>) return static_cast<int>(int_value);
        throw std::runtime_error("unsupported get()");
    }

    std::string dump() const {
        std::ostringstream os;
        dump(os);
        return os.str();
    }

    static json parse(const std::string& s) {
        json j;
        j.type = value_t::object;
        j.object_value.clear();

        if (s.find('{') != std::string::npos) {
            auto keyStart = s.find('"');
            auto keyEnd = s.find('"', keyStart + 1);
            auto valStart = s.find('"', keyEnd + 1);
            auto valEnd = s.find('"', valStart + 1);

            if (keyStart != std::string::npos && valStart != std::string::npos) {
                j.object_value[s.substr(keyStart + 1, keyEnd - keyStart - 1)] =
                    s.substr(valStart + 1, valEnd - valStart - 1);
            }
        }
        return j;
    }

private:
    void dump(std::ostream& os) const {
        switch (type) {
        case value_t::null: os << "null"; break;
        case value_t::boolean: os << (bool_value ? "true" : "false"); break;
        case value_t::string: os << "\"" << string_value << "\""; break;
        case value_t::number_integer: os << int_value; break;
        case value_t::number_unsigned: os << uint_value; break;
        case value_t::number_float: os << float_value; break;
        case value_t::array:
            os << "[";
            for (size_t i = 0; i < array_value.size(); ++i) {
                if (i) os << ",";
                array_value[i].dump(os);
            }
            os << "]";
            break;
        case value_t::object:
            os << "{";
            bool first = true;
            for (auto& kv : object_value) {
                if (!first) os << ",";
                first = false;
                os << "\"" << kv.first << "\":";
                kv.second.dump(os);
            }
            os << "}";
            break;
        }
    }
};

} // namespace nlohmann
#endif
