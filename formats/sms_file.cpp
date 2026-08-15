#include "sms_file.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace linear_algebra {

namespace {

Givaro::Integer parse_integer(const std::string& text) {
    if (text.empty()) throw std::runtime_error("empty number in SMS");
    const std::size_t digits_from = (text[0] == '-' || text[0] == '+') ? 1 : 0;
    if (digits_from == text.size()) throw std::runtime_error("'" + text + "' has no digits");
    for (std::size_t index = digits_from; index < text.size(); ++index) {
        if (text[index] < '0' || text[index] > '9') {
            throw std::runtime_error("'" + text + "' is not an integer");
        }
    }
    return Givaro::Integer(text.c_str());
}

Givaro::Rational parse_value(const std::string& text) {
    const std::size_t slash = text.find('/');
    if (slash == std::string::npos) {
        return Givaro::Rational(parse_integer(text), Givaro::Integer(1));
    }
    const Givaro::Integer denominator = parse_integer(text.substr(slash + 1));
    if (denominator == 0) throw std::runtime_error("zero denominator in '" + text + "'");
    return Givaro::Rational(parse_integer(text.substr(0, slash)), denominator);
}

}  // namespace

RationalMatrix read_sms(std::istream& input) {
    std::size_t rows = 0;
    std::size_t columns = 0;
    std::string type;
    if (!(input >> rows >> columns >> type)) {
        throw std::runtime_error("SMS needs a header: rows columns type");
    }
    if (type != "M" && type != "R" && type != "I") {
        throw std::runtime_error("SMS type must be M, R or I, not '" + type + "'");
    }

    RationalMatrix matrix(rows, columns);
    for (;;) {
        long long row = 0;
        long long column = 0;
        std::string value;
        if (!(input >> row >> column >> value)) {
            throw std::runtime_error("SMS ended before its 0 0 0 terminator");
        }
        if (row == 0 && column == 0) break;  // the terminator; its value is ignored

        if (row < 1 || column < 1 || static_cast<std::size_t>(row) > rows ||
            static_cast<std::size_t>(column) > columns) {
            throw std::runtime_error("SMS entry " + std::to_string(row) + " " +
                                     std::to_string(column) + " is outside the declared shape");
        }
        // SMS counts from one.
        matrix(static_cast<std::size_t>(row) - 1, static_cast<std::size_t>(column) - 1) =
            parse_value(value);
    }
    return matrix;
}

RationalMatrix read_sms_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open SMS file: " + path);
    return read_sms(input);
}

void write_sms(std::ostream& output, const RationalMatrix& matrix) {
    const RationalField field;

    bool every_entry_is_an_integer = true;
    for (std::size_t entry = 0; entry < matrix.entry_count(); ++entry) {
        if (matrix.data()[entry].deno() != 1) {
            every_entry_is_an_integer = false;
            break;
        }
    }

    output << matrix.rows() << " " << matrix.columns() << " "
           << (every_entry_is_an_integer ? "M" : "R") << "\n";
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            const Givaro::Rational& value = matrix(row, column);
            if (field.isZero(value)) continue;
            output << (row + 1) << " " << (column + 1) << " " << value.nume();
            if (value.deno() != 1) output << "/" << value.deno();
            output << "\n";
        }
    }
    output << "0 0 0\n";
}

}  // namespace linear_algebra
