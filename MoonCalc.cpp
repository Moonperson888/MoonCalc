#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>

bool relaunchInWindowsTerminal() {
    if (std::getenv("WT_SESSION")) return false;

    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string cmd = std::string("wt.exe powershell -NoExit -Command \"& '") + path + "'\"";
    return std::system(cmd.c_str()) == 0;
}
#endif

class Parser {
public:
    explicit Parser(const std::string& text) : s(text), pos(0) {}

    double parse() {
        double value = expression();
        skipSpaces();
        if (pos != s.size())
            throw std::runtime_error(std::string("unexpected character '") + s[pos] + "'");
        return value;
    }

private:
    std::string s;
    std::size_t pos;

    void skipSpaces() {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos])))
            ++pos;
    }

    bool match(char c) {
        skipSpaces();
        if (pos < s.size() && s[pos] == c) {
            ++pos;
            return true;
        }
        return false;
    }

    double expression() {
        double value = term();
        while (true) {
            if (match('+'))      value += term();
            else if (match('-')) value -= term();
            else break;
        }
        return value;
    }

    double term() {
        double value = factor();
        while (true) {
            if (match('*')) {
                value *= factor();
            } else if (match('/')) {
                double divisor = factor();
                if (divisor == 0.0)
                    throw std::runtime_error("division by zero");
                value /= divisor;
            } else {
                break;
            }
        }
        return value;
    }

    double factor() {
        if (match('+')) return factor();
        if (match('-')) return -factor();

        double base = primary();
        if (match('^')) return std::pow(base, factor());
        return base;
    }

    double primary() {
        if (match('(')) {
            double value = expression();
            if (!match(')'))
                throw std::runtime_error("missing ')'");
            return value;
        }

        skipSpaces();
        std::size_t start = pos;
        while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])) || s[pos] == '.'))
            ++pos;

        if (start == pos)
            throw std::runtime_error("expected a number");

        std::string token = s.substr(start, pos - start);
        std::size_t used = 0;
        double value = 0.0;
        try {
            value = std::stod(token, &used);
        } catch (const std::exception&) {
            throw std::runtime_error("invalid number '" + token + "'");
        }
        if (used != token.size())
            throw std::runtime_error("invalid number '" + token + "'");
        return value;
    }
};

int main() {
#ifdef _WIN32
    if (relaunchInWindowsTerminal()) return 0;
#endif

    std::cout << "Calculator: + - * / ^ and parentheses.\n"
              << "Type 'quit' to exit.\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "quit" || line == "exit")
            break;

        if (line.find_first_not_of(" \t\r") == std::string::npos)
            continue;

        try {
            Parser parser(line);
            std::cout << "= " << std::setprecision(12) << parser.parse() << '\n';
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << '\n';
        }
    }
    return 0;
}

