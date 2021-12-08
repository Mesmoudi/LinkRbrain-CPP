#ifndef LINKRBRAIN2019__SRC__DB__FIELDTYPE_HPP
#define LINKRBRAIN2019__SRC__DB__FIELDTYPE_HPP


namespace DB {


    enum FieldType {
        Unrecognized,
        UInt64,
        String,
        Variant,
        DateTime,
        Boolean,
    };


    template <typename T> const FieldType get_field_type() { return FieldType::Unrecognized; }
    template <> const FieldType get_field_type<uint64_t>() { return FieldType::UInt64; }
    template <> const FieldType get_field_type<std::string>() { return FieldType::String; }
    template <> const FieldType get_field_type<Types::Variant>() { return FieldType::Variant; }
    template <> const FieldType get_field_type<Types::DateTime>() { return FieldType::DateTime; }
    template <> const FieldType get_field_type<bool>() { return FieldType::Boolean; }


} // DB


#endif // LINKRBRAIN2019__SRC__DB__FIELDTYPE_HPP
