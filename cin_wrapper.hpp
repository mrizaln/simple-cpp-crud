#ifndef CIN_WRAPPER_HPP_UVHDMPJL
#define CIN_WRAPPER_HPP_UVHDMPJL

#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

class CinWrapper
{
private:
    std::istream&         m_cin = std::cin;
    std::function<void()> m_onEof;

public:
    CinWrapper(std::function<void()> onEof)
        : m_onEof{ std::move(onEof) }
    {
    }

    template <typename T>
    CinWrapper& operator>>(T& value)
    {
        if (!(std::cin >> value)) {
            if (std::cin.eof()) {
                if (m_onEof) {
                    m_onEof();
                }
            } else if (std::cin.fail()) {
                std::cin.clear();
            } else {
                std::cout << "Failed to read value or error occurred." << std::endl;
                throw std::runtime_error{ "Unable to read value from std::cin" };
            }
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return *this;
    }

    std::istream& getline(std::string& str)
    {
        std::getline(m_cin, str);
        return m_cin;
    }

    std::istream& operator*() { return m_cin; }
    std::istream* operator->() { return &m_cin; }
};

#endif /* end of include guard: CIN_WRAPPER_HPP_UVHDMPJL */
