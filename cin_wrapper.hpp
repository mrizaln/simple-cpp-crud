#ifndef CIN_WRAPPER_HPP_UVHDMPJL
#define CIN_WRAPPER_HPP_UVHDMPJL

#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// CinWrapper, self-recovering `std::cin` wrapper, which ignores all errors and recovers from them.
// fail status is saved as bool that store last operation status, check it using `previousFail()` method.
// the wrapper main purpose is to detect `std::cin.eof()` and call a callback function if it's set.
class CinWrapper
{
private:
    std::istream&         m_cin = std::cin;
    std::function<void()> m_onEof;
    bool                  m_previousFail;

public:
    CinWrapper(std::function<void()> onEof)
        : m_onEof{ std::move(onEof) }
    {
    }

    bool previousFail() const { return m_previousFail; }

    std::istream& getline(std::string& str)
    {
        std::getline(m_cin, str);
        m_previousFail = !checkStatusAndRecover();

        return m_cin;
    }

    template <
        typename T,
        typename = std::enable_if_t<std::disjunction_v<
            std::is_same<T, int>,
            std::is_same<T, long>,
            std::is_same<T, long long>,
            std::is_same<T, unsigned long>,
            std::is_same<T, unsigned long long>>>>
    std::optional<T> getFromLine()
    {
        std::string line;
        std::getline(m_cin, line);
        m_previousFail = !checkStatusAndRecover();

        if (m_previousFail) {
            return std::nullopt;
        }

        try {
            // clang-format off
            if      constexpr (std::is_same_v<T, int>)                { return std::stoi  (line); }
            else if constexpr (std::is_same_v<T, long>)               { return std::stol  (line); }
            else if constexpr (std::is_same_v<T, long long>)          { return std::stoll (line); }
            else if constexpr (std::is_same_v<T, unsigned long>)      { return std::stoul (line); }
            else if constexpr (std::is_same_v<T, unsigned long long>) { return std::stoull(line); }
            // clang-format on
        } catch (...) {
            // also set fail flag
            m_previousFail = true;
            return std::nullopt;
        }
    }

private:
    // returns true if std::cin is in good state, false otherwise
    bool checkStatusAndRecover()
    {
        // very unlikely to happen
        if (std::cin.bad()) {
            throw std::runtime_error{ "Unable to read value from std::cin. Bad bit is set, can't recover!" };
        }

        // if eof is reached, std::cin can't be recovered using only standard methods
        if (std::cin.eof()) {
            if (m_onEof) {
                m_onEof();
            }
            return false;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            return false;
        }

        return true;
    }
};

#endif /* end of include guard: CIN_WRAPPER_HPP_UVHDMPJL */
