#ifndef CPPP____INCLUDE____TYPES__DATETIME_HPP
#define CPPP____INCLUDE____TYPES__DATETIME_HPP


#include <time.h>
#include <stdint.h>
#include <memory.h>
#include <sys/time.h>
#include <iomanip>

#include "Exceptions/Exception.hpp"


namespace Types {

    #pragma pack(push, 1)

    struct DateTime {
    public:

        DateTime() {
            memset(this, 0, sizeof(DateTime));
        }
        DateTime(const DateTime& source) {
            memcpy(this, &source, sizeof(DateTime));
        }
        template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
        DateTime(const T& timestamp) {
            set_timestamp(timestamp);
        }
        DateTime(const int64_t timestamp, const uint32_t microsecond) {
            set_timestamp(timestamp);
            set_microsecond(microsecond);
        }
        DateTime(const int32_t year, const uint32_t month, const uint32_t day, const uint32_t hour=0, const uint32_t minute=0, const uint32_t second=0, const uint32_t microsecond=0) :
            _year(year),
            _month(month),
            _day(day),
            _hour(hour),
            _minute(minute),
            _second(second),
            _microsecond(microsecond) {}
        DateTime(const std::string& string_timestamp) {
            int32_t year = 0;
            uint32_t month = 0;
            uint32_t day = 0;
            uint32_t hour = 0;
            uint32_t minute = 0;
            uint32_t second = 0;
            uint32_t microsecond = 0;
            int i = sscanf(string_timestamp.c_str(), "%d-%u-%u %u:%u:%u.%u",
                &year, &month, &day,
                &hour, &minute, &second, &microsecond
            );
            _year = year;
            _month = month;
            _day = day;
            _hour = hour;
            _minute = minute;
            _second = second;
            _microsecond = microsecond;
        }

        static DateTime now() {
            double t;
            timeval tv;
            gettimeofday(&tv, NULL);
            return DateTime(tv.tv_sec, tv.tv_usec);
        }

        void set_year(const int32_t year) {
            _year = year;
        }
        void set_month(const uint32_t month) {
            _month = month;
        }
        void set_day(const uint32_t day) {
            _day = day;
        }
        void set_hour(const uint32_t hour) {
            _hour = hour;
        }
        void set_minute(const uint32_t minute) {
            _minute = minute;
        }
        void set_second(const uint32_t second) {
            _second = second;
        }
        void set_microsecond(const uint32_t microsecond) {
            _microsecond = microsecond;
        }

        const int32_t get_year() const {
            return _year;
        }
        const uint32_t get_month() const {
            return _month;
        }
        const uint32_t get_day() const {
            return _day;
        }
        const uint32_t get_hour() const {
            return _hour;
        }
        const uint32_t get_minute() const {
            return _minute;
        }
        const uint32_t get_second() const {
            return _second;
        }
        const uint32_t get_microsecond() const {
            return _microsecond;
        }

        //

        const double get_timestamp() const {
            uint32_t year;
            uint32_t result;
            static const int cumdays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
            year = _year + _month / 12;
            result = (year - 1970) * 365 + cumdays[(_month - 1) % 12];
            result += (year - 1968) / 4;
            result -= (year - 1900) / 100;
            result += (year - 1600) / 400;
            if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) && ((_month - 1) % 12) < 2) {
                result--;
            }
            result += _day - 1;
            result *= 24;
            result += _hour;
            result *= 60;
            result += _minute;
            result *= 60;
            result += _second;
            return result + 1e-6 * _microsecond;
        }
        void set_timestamp(const int32_t& timestamp) {
            set_timestamp((int64_t) timestamp);
        }
        void set_timestamp(const int64_t& timestamp) {
            time_t t = timestamp;
            struct tm timeinfo;
            gmtime_r(&t, &timeinfo);
            _year = 1900 + timeinfo.tm_year;
            _month = 1 + timeinfo.tm_mon;
            _day = timeinfo.tm_mday;
            _hour = timeinfo.tm_hour;
            _minute = timeinfo.tm_min;
            _second = timeinfo.tm_sec;
            _microsecond = 0;
        }
        void set_timestamp(const double& timestamp) {
            const int64_t timestamp_integer = timestamp;
            set_timestamp(timestamp_integer);
            _microsecond = 1e6 * (timestamp - timestamp_integer);
        }

        //

        operator std::string() const {
            char result[32];
            snprintf(result, sizeof(result), "%d-%02d-%02dT%02d-%02d-%02d.%06d", _year, _month, _day, _hour, _minute, _second, _microsecond);
            return result;
        }

        const std::string get_date_string() const {
            char result[12];
            snprintf(result, sizeof(result), "%d-%02d-%02d", _year, _month, _day);
            return result;
        }

        //

        inline const bool operator == (const DateTime& other) const {
            return _hash == other._hash;
        }
        inline const bool operator != (const DateTime& other) const {
            return _hash != other._hash;
        }

        //

        inline const DateTime& operator = (const DateTime& source) {
            memcpy(this, &source, sizeof(DateTime));
            return *this;
        }

    private:

        union {
            struct {
                int32_t _year : 18;
                uint32_t _month : 4;
                uint32_t _day : 5;
                uint32_t _hour : 5;
                uint32_t _minute : 6;
                uint32_t _second : 6;
                uint32_t _microsecond : 20;
            };
            uint64_t _hash;
        };

    };

    #pragma pack(pop)

    std::ostream& operator<<(std::ostream& os, const DateTime& datetime) {
        return (os
            << std::setfill('0')
            << std::setw(4) << datetime.get_year() << '-'
            << std::setw(2) << datetime.get_month() << '-'
            << std::setw(2) << datetime.get_day() << 'T'
            << std::setw(2) << datetime.get_hour() << ':'
            << std::setw(2) << datetime.get_minute() << ':'
            << std::setw(2) << datetime.get_second() << '.'
            << std::setw(6) << datetime.get_microsecond()
            << std::setfill(' ')
        );
    }

} // Types


namespace std {
    const string to_string(const Types::DateTime& datetime) {
        stringstream buffer;
        buffer << datetime;
        return buffer.str();
    }
} // std


#endif // CPPP____INCLUDE____TYPES__DATETIME_HPP
