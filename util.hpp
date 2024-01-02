#ifndef UTIL_HPP_4LFLPWKG
#define UTIL_HPP_4LFLPWKG

#include <array>
#include <cstring>
#include <string>

using IntBinType = std::array<char, sizeof(int)>;

inline IntBinType intToBin(int v)
{
    IntBinType buf;
    std::memcpy(&buf.front(), &v, buf.size());
    return buf;
};

inline int binToInt(const IntBinType& b)
{
    int v;
    std::memcpy(&v, &b.front(), b.size());
    return v;
}

inline IntBinType strToBin(const std::string& string)
{
    IntBinType buf;
    std::memcpy(&buf.front(), &string.front(), buf.size());
    return buf;
}

inline std::string insertSize(const std::string& string)
{
    auto        size_s = intToBin(static_cast<int>(string.size()));
    std::string newString{ size_s.begin(), size_s.end() };
    newString += string;
    return newString;
}

#endif /* end of include guard: UTIL_HPP_4LFLPWKG */
