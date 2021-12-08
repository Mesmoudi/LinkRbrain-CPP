#ifndef LINKRBRAIN2019__SRC__READING__READER_HPP
#define LINKRBRAIN2019__SRC__READING__READER_HPP


#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace Reading {

    class CSVReader {
    public:

        CSVReader(const std::filesystem::path& path, const size_t skip_lines=0) :
            _path(path),
            _f(fopen(_path.c_str(), "r")),
            _line_index(0),
            _is_finished(false),
            _skip_lines(skip_lines)
        {
            if (_f == NULL) {
                _is_finished = true;
                throw Exceptions::NotFoundException("File not found: " + path.native(), {});
            } else {
                _is_finished = false;
            }
        }
        virtual ~CSVReader() {
            if (_f != NULL) {
                fclose(_f);
            }
        }

        const bool parse_line(std::vector<std::string>& result) {
            result.clear();
            if (_is_finished) {
                return false;
            }
            std::string column;
            bool quoting = false;
            char c = '\0';
            while ((c = fgetc(_f)) != EOF) {
                if (c == '\n' && !quoting) {
                    ++_line_index;
                    if (_line_index > _skip_lines) {
                        result.push_back(column);
                        return true;
                    }
                }
                if (_line_index < _skip_lines) {
                    continue;
                }
                switch (c) {
                    case ',':
                        if (!quoting) {
                            result.push_back(column);
                            column = "";
                        }
                        break;
                    case '\"':
                        if (quoting) {
                            quoting = false;
                        } else {
                            quoting = true;
                        }
                        break;
                    default:
                        column += c;
                        break;
                }
            }
            _is_finished = true;
            return false;
        }

    private:
        const std::filesystem::path _path;
        FILE* _f;
        size_t _line_index;
        size_t _skip_lines;
        bool _is_finished;

    };

}


#endif // LINKRBRAIN2019__SRC__READING__READER_HPP
