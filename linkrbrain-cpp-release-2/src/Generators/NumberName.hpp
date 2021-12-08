#ifndef LINKRBRAIN2019__SRC__GENERATORS__NUMBERNAME_HPP
#define LINKRBRAIN2019__SRC__GENERATORS__NUMBERNAME_HPP


#include <stdint.h>
#include <string>


namespace Generators {


    class NumberName {
    public:

        static const std::string get_english_name(int64_t number) {
            static std::string units[] = {
                "one",
                "two",
                "three",
                "four",
                "five",
                "six",
                "seven",
                "eight",
                "nine",
                "ten",
                "eleven",
                "twelve",
                "thirteen",
                "fourteen",
                "fifteen",
                "sixteen",
                "seventeen",
                "eighteen",
                "nineteen",
            };
            static std::string tens[] = {
                "twenty",
                "thirty",
                "fourty",
                "fifty",
                "sixty",
                "seventy",
                "eighty",
                "ninety",
            };
            static std::string scale[] = {
                "thousand",
                "million",
                "billion",
                "trillion",
                "quadrillion",
                "quintillion",
                "sextillion",
                "septillion",
            };
            std::string name;
            const bool is_negative = (number < 0);
            if (number == 0) {
                return "zero";
            } else if (is_negative) {
                number = -number;
            }
            int scale_index = -1;
            while (number) {
                std::string last3_name;
                uint8_t last_digit[] = {
                    number % 10,
                    (number / 10) % 10,
                    (number % 1000) / 100,
                };
                //
                if (last_digit[2]) {
                    last3_name += units[last_digit[2] - 1] + " hundred";
                }
                if (last_digit[1] > 1) {
                    if (last_digit[2]) {
                        last3_name += " ";
                    }
                    last3_name += tens[last_digit[1] - 2];
                    if (last_digit[0]) {
                        last3_name += "-";
                    }
                }
                if (last_digit[0] || last_digit[1] == 1) {
                    if (last_digit[1] == 1) {
                        if (last3_name.size()) {
                            last3_name += " ";
                        }
                        last3_name += units[last_digit[0] + 9];
                    } else {
                        if (last_digit[2] and !last_digit[1]) {
                            last3_name += " and ";
                        }
                        last3_name += units[last_digit[0] - 1];
                    }
                }
                //
                if (scale_index == -1) {
                    name = last3_name;
                } else if (last3_name.size()) {
                    const std::string plural = (last3_name == "one") ? "" : "s";
                    std::string suffix = scale[scale_index] + plural;
                    if (suffix.size() && name.size()) {
                        suffix += " ";
                    }
                    suffix += name;
                    if (suffix.size()) {
                        suffix = " " + suffix;
                    }
                    name = last3_name + suffix;
                }
                ++scale_index;
                number /= 1000;
            }
            if (is_negative) {
                return "minus " + name;
            }
            return name;
        }

    };


} // Generators


#endif // LINKRBRAIN2019__SRC__GENERATORS__NUMBERNAME_HPP
