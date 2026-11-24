#include "calculator.hpp"

#include <charconv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace myos {
namespace {
class Equation {
public:
    explicit Equation(std::string_view text) : text_(text) {}

    double evaluate() {
        const double result = sum();
        skip_spaces();
        if (!text_.empty()) throw std::runtime_error("Unexpected text in equation.");
        return result;
    }

private:
    std::string_view text_;

    void skip_spaces() {
        const auto start = text_.find_first_not_of(" \t\r");
        text_.remove_prefix(start == std::string_view::npos ? text_.size() : start);
    }

    bool consume(std::string_view token) {
        skip_spaces();
        if (text_.substr(0, token.size()) != token) return false;
        text_.remove_prefix(token.size());
        return true;
    }

    static double finite(double value) {
        if (!std::isfinite(value)) throw std::runtime_error("Result is outside the real numeric range.");
        return value;
    }

    double sum() {
        double value = product();
        for (;;) {
            if (consume("+")) value = finite(value + product());
            else if (consume("-")) value = finite(value - product());
            else return value;
        }
    }

    double product() {
        double value = factor();
        for (;;) {
            if (consume("*")) {
                value = finite(value * factor());
                continue;
            }

            enum class Operation { Divide, FloorDivide, Remainder };
            Operation operation;
            if (consume("//")) operation = Operation::FloorDivide;
            else if (consume("/")) operation = Operation::Divide;
            else if (consume("%")) operation = Operation::Remainder;
            else return value;

            const double divisor = factor();
            if (divisor == 0) throw std::runtime_error("Cannot divide by zero.");
            if (operation == Operation::Divide) {
                value = finite(value / divisor);
                continue;
            }

            double remainder = std::fmod(value, divisor);
            double quotient = (value - remainder) / divisor;
            if (remainder != 0 && std::signbit(remainder) != std::signbit(divisor)) {
                remainder += divisor;
                quotient -= 1;
            }
            value = operation == Operation::FloorDivide ? finite(std::round(quotient)) : remainder;
        }
    }

    double factor() {
        if (consume("+")) return factor();
        if (consume("-")) return -factor();
        const double base = primary();
        if (!consume("**")) return base;
        return finite(std::pow(base, factor()));
    }

    double primary() {
        if (consume("(")) {
            const double value = sum();
            if (!consume(")")) throw std::runtime_error("Missing closing parenthesis.");
            return value;
        }
        skip_spaces();
        if (text_.empty() || (text_.front() != '.' &&
                (text_.front() < '0' || text_.front() > '9'))) {
            throw std::runtime_error("Expected a number or opening parenthesis.");
        }
        double value = 0;
        const auto result = std::from_chars(text_.data(), text_.data() + text_.size(), value);
        if (result.ec == std::errc::result_out_of_range) {
            throw std::runtime_error("Number is outside the supported range.");
        }
        if (result.ec != std::errc{}) throw std::runtime_error("Invalid number.");
        text_.remove_prefix(static_cast<std::size_t>(result.ptr - text_.data()));
        return value;
    }
};
}

void run_calculator(std::istream& input, std::ostream& output) {
    output << "Enter an equation\n" << std::flush;
    std::string equation;
    if (!std::getline(input, equation)) return;
    if (equation.size() > 512) {
        output << "Calculator: equation is too long.\n";
        return;
    }
    try {
        const double result = Equation(equation).evaluate();
        std::ostringstream formatted;
        formatted << std::setprecision(std::numeric_limits<double>::digits10) << result;
        output << "the answer is " << formatted.str() << '\n';
    } catch (const std::runtime_error& error) {
        output << "Calculator: " << error.what() << '\n';
    }
}
}
