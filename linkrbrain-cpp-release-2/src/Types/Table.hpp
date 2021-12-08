#ifndef LINKRBRAIN2019__SRC__TYPES__TABLE_HPP
#define LINKRBRAIN2019__SRC__TYPES__TABLE_HPP


#include <vector>
#include <string>
#include <iomanip>

#include "Exceptions/GenericExceptions.hpp"


namespace Types {

    static const std::string empty_string = "";


    class Table {
    public:

        Table() : _column_count(0) {}
        Table(const std::initializer_list<std::string>& header) : _column_count(0) {
            set_header(header);
        }
        Table(const std::vector<std::string>& header) : _column_count(0) {
            set_header(header);
        }
        template <typename ... Args>
        Table(const Args& ... args) : _column_count(0) {
            set_header(args ...);
        }

        void set_header(const std::initializer_list<std::string>& header) {
            _header = header;
            update_count_and_width(header);
        }
        void set_header(const std::vector<std::string>& header) {
            _header = header;
            update_count_and_width(header);
        }
        template <typename ... Args>
        void set_header(const Args& ... args) {
            set_header(
                std::vector<std::string>({
                    to_string(args) ...
                })
            );
        }

        void add_row(const std::vector<std::string>& row) {
            _rows.push_back(row);
            update_count_and_width(row);
        }
        template <typename ... Args>
        void add_row(const Args& ... args) {
            add_row(
                std::vector<std::string>({
                    to_string(args) ...
                })
            );
        }

        const bool has_header() const {
            return _header.size();
        }
        const std::vector<std::string>& get_header() const {
            return _header;
        }

        const bool has_rows() const {
            return _rows.size();
        }
        const std::vector<std::vector<std::string>>& get_rows() const {
            return _rows;
        }
        const size_t get_rows_count() const {
            return _rows.size();
        }
        const std::vector<std::string>& get_row(const size_t i) const {
            if (i >= _rows.size()) {
                throw Exceptions::BadDataException("Row index is way too big: " + std::to_string(i));
            }
            return _rows[i];
        }

        const size_t& get_column_count() const {
            return _column_count;
        }
        const std::vector<size_t>& get_column_widths() const {
            return _column_widths;
        }

        void to_text(std::ostream& buffer) const {
            show_line(buffer, "┌", "┬", "┐");
            if (has_header()) {
                show_row(buffer, _header);
            }
            if (has_header() && has_rows()) {
                show_line(buffer, "├", "┼", "┤");
            }
            for (const std::vector<std::string>& row : _rows) {
                show_row(buffer, row);
            }
            show_line(buffer, "└", "┴", "┘");
        }

    private:

        template <typename T>
        static const std::string to_string(const T& value) {
            if constexpr (std::is_convertible<T, std::string>::value) {
                return value;
            } else {
                return std::to_string(value);
            }
        }
        template <size_t N>
        static const std::string to_string(const char (&data)[N]) {
            return {data, (N&&!data[N-1]) ? (N-1) : N};
        }

        void update_count_and_width(const std::vector<std::string>& row) {
            // update column count when new row has more
            const size_t column_count = row.size();
            if (column_count > _column_count) {
                _column_count = column_count;
                _column_widths.resize(column_count);
            }
            // update column widths
            for (size_t column_index = 0; column_index < column_count; column_index++) {
                const size_t column_width = row[column_index].size();
                if (column_width > _column_widths[column_index]) {
                    _column_widths[column_index] = column_width;
                }
            }
        }

        void show_line(std::ostream& buffer, const std::string& begin, const std::string& separator, const std::string& end) const {
            buffer << begin;
            bool first = true;
            for (const size_t& column_width : get_column_widths()) {
                if (first) {
                    first = false;
                } else {
                    buffer << separator;
                }
                for (size_t i = 0; i < column_width; i++) {
                    buffer << "─";
                }
            }
            buffer << end << '\n';
        }
        void show_row(std::ostream& buffer, const std::vector<std::string>& row) const {
            buffer << "│";
            bool first = true;
            for (size_t column_index = 0; column_index < _column_count; column_index++) {
                if (first) {
                    first = false;
                } else {
                    buffer << "│";
                }
                buffer << std::left << std::setw(_column_widths[column_index]);
                if (column_index < row.size()) {
                    buffer << row[column_index];
                } else {
                    buffer << empty_string;
                }
            }
            buffer << "│\n";
        }

        std::vector<std::string> _header;
        std::vector<std::vector<std::string>> _rows;
        size_t _column_count;
        std::vector<size_t> _column_widths;

    };

    std::ostream& operator << (std::ostream& buffer, const Table& table) {
        table.to_text(buffer);
        return buffer;
    }


} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__TABLE_HPP
