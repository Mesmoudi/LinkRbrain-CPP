#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__DATASET_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__DATASET_HPP


#include <stdint.h>

#include <fstream>
#include <limits>
#include <memory>
#include <vector>
#include <string>

#include "./Group.hpp"

#include "Exceptions/GenericExceptions.hpp"
#include "Conversion/Binary.hpp"
#include "Types/Entity.hpp"
#include "Types/NumberNature.hpp"
#include "Logging/Loggable.hpp"


namespace LinkRbrain::Models {


    template <typename T>
    class Dataset : public Types::Entity, public Logging::Loggable {
    public:

        Dataset(const size_t organ_id=0, const size_t id=0, const std::string& label="") :
            Entity(id, label),
            _organ_id(organ_id) {}
        Dataset(std::istream& buffer) {
            Conversion::Binary::parse(buffer, *this);
        }
        Dataset(const std::filesystem::path& path) {
            std::ifstream buffer(path);
            Conversion::Binary::parse(buffer, *this);
        }

        const size_t& get_organ_id() const {
            return _organ_id;
        }
        size_t& get_organ_id() {
            return _organ_id;
        }
        std::vector<Group<T>>& get_groups() {
            return _groups;
        }
        const std::vector<Group<T>>& get_groups() const {
            return _groups;
        }
        Group<T>& get_group(const size_t identifier) {
            for (Group<T>& group : _groups) {
                if (group.get_id() == identifier) {
                    return group;
                }
            }
            throw Exceptions::NotFoundException("Cannot find group with this identifier", {
                {"model", "Group"},
                {"key", "id"},
                {"id", identifier}
            });
        }
        const Group<T>& get_group(const size_t identifier) const {
            for (Group<T>& group : _groups) {
                if (group.get_id() == identifier) {
                    return group;
                }
            }
            throw Exceptions::NotFoundException("Cannot find group with this identifier", {
                {"model", "Group"},
                {"key", "id"},
                {"id", identifier}
            });
        }
        Group<T>& get_group(const std::string& group_label, const bool exact=true) {
            for (Group<T>& group : _groups) {
                if (exact && group.get_label() == group_label) {
                        return group;
                } else if (!exact && group.get_label().find(group_label) != std::string::npos) {
                    return group;
                }
            }
            throw Exceptions::NotFoundException("Cannot find group with this label: " + group_label, {
                {"model", "Group"},
                {"key", "label"},
                {"label", group_label}
            });
        }

        Group<T>& add_group(const std::string& group_label) {
            _groups.push_back(group_label);
            return _groups.back();
        }
        Group<T>& add_group(const Group<T>& group) {
            _groups.push_back(group);
            return _groups.back();
        }

        //

        const Types::PointExtrema<T> compute_extrema() const {
            Types::PointExtrema<T> extrema;
            for (const Group<T>& group : get_groups()) {
                for (const Types::Point<T>& point : group.get_points()) {
                    extrema.integrate(point);
                }
            }
            return extrema;
        }
        const size_t compute_hash() const {
            static const size_t shift = 8 * sizeof(size_t) / 2;
            size_t hash = 0;
            for (const auto& group : _groups) {
                hash ^= group.compute_hash();
                hash = (hash << shift) | (hash >> shift);
            }
            return hash;
        }

        std::vector<std::reference_wrapper<Group<T>>> search_groups(const std::string& search_string, const bool exact=false) {
            std::vector<std::reference_wrapper<Group<T>>> result;
            for (Group<T>& group : _groups) {
                const std::string& group_label = group.get_label();
                if ((!exact && group_label.find(search_string) != std::string::npos) || group_label == search_string) {
                    result.push_back(group);
                }
            }
            return result;
        }

        virtual const std::string get_logger_name() {
            return "Dataset[" + std::to_string(get_id()) + "|" + get_label() + "]";
        }

    private:

        size_t _organ_id;
        std::vector<Group<T>> _groups;

    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Dataset<T>& dataset) {
        os << "<Dataset label=" << dataset.get_label() << " groups=";
        for (const Group<T>& group : dataset.get_groups()) {
            os << "\n    " << group;
        }
        return os << ">";
    }


} // LinkRbrain::Models


namespace Conversion::Binary {

    template <typename T>
    void dataset_parse(std::istream& buffer, LinkRbrain::Models::Dataset<T>& destination) {
        parse(buffer, (Types::Entity&) destination);
        parse(buffer, destination.get_organ_id());
        Types::NumberNature number_nature = {.type=Types::NumberNature::Other};
        straight_parse(buffer, number_nature);
        if (number_nature != Types::NumberNatureOf<T>) {
            throw Exceptions::BadDataException("dataset_parse: number nature is different in template parameter (" + Types::NumberNatureOf<T>.get_full_name() + ") than in loaded file (" + number_nature.get_full_name() + ").");
        }
        buffer.ignore(256);
        parse(buffer, destination.get_groups());
    }

    template <>
    void parse<LinkRbrain::Models::Dataset<float>>(std::istream& buffer, LinkRbrain::Models::Dataset<float>& destination) {
        dataset_parse(buffer, destination);
    }
    template <>
    void parse<LinkRbrain::Models::Dataset<double>>(std::istream& buffer, LinkRbrain::Models::Dataset<double>& destination) {
        dataset_parse(buffer, destination);
    }
    template <>
    void parse<LinkRbrain::Models::Dataset<long double>>(std::istream& buffer, LinkRbrain::Models::Dataset<long double>& destination) {
        dataset_parse(buffer, destination);
    }

    template <typename T>
    void dataset_serialize(std::ostream& buffer, const LinkRbrain::Models::Dataset<T>& source) {
        serialize(buffer, (const Types::Entity&) source);
        serialize(buffer, source.get_organ_id());
        straight_serialize(buffer, Types::NumberNatureOf<T>);
        buffer.seekp(256, std::ios_base::cur);
        serialize(buffer, source.get_groups().size());
        for (const auto& group : source.get_groups()) {
            serialize(buffer, group);
        }
    }

    template <>
    void serialize<LinkRbrain::Models::Dataset<float>>(std::ostream& buffer, const LinkRbrain::Models::Dataset<float>& source) {
        dataset_serialize(buffer, source);
    }
    template <>
    void serialize<LinkRbrain::Models::Dataset<double>>(std::ostream& buffer, const LinkRbrain::Models::Dataset<double>& source) {
        dataset_serialize(buffer, source);
    }
    template <>
    void serialize<LinkRbrain::Models::Dataset<long double>>(std::ostream& buffer, const LinkRbrain::Models::Dataset<long double>& source) {
        dataset_serialize(buffer, source);
    }

} // Conversion::Binary


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__DATASET_HPP
