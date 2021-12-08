#include "ORM/Database.hpp"
#include "ORM/macros.hpp"
#include "ORM/types.hpp"


class Group {
public:
    std::string label;
};

ORM_Model("datasets", Dataset,
    ORM_Field(id, ORM::Int32);
    ORM_Field(name, ORM::Text);
    ORM_Field(label, ORM::Text);
    // ORM_Field(metadata, ORM::JSON);
    // ORM_Field(groups, ORM::Blob<std::vector<Group>>);
)

ORM_Model("tests", Test,
    ORM_Field(id, int);
    ORM_Field(name, std::string);
)


static const ORM::Converter* converter;


#include <iostream>

void show(const ORM::ModelBase& instance) {

    std::cout
        << "model `" << instance.get_name()
        << "` with status `" << ORM::get_status_name(instance.get_status())
        << "`" << '\n';
    for (const ORM::FieldBase* field : instance.get_fields()) {
        std::cout
            << " - "
            << "model `" << field->get_model().get_name()
            << "`, offset " << field->get_offset()
            << ": field `" << field->get_name()
            << "`, status `" << ORM::get_status_name(field->get_status())
            << "`, type `" << field->get_info().type.name
            << "`, value `"
            << (field->get_isnull() ? "NULL" : converter->to_text(*field))
            << "`\n";
    }
}


int main(int argc, char const *argv[]) {

    // std::cout << Types::Variant::get_type_name(ORM::variant_type_of<
    //     ORM::Text
    // >) << '\n';
    // exit(0);
    ORM::Database db(ORM::Postgres, "host=localhost user=linkrbrain_user password=linkrbrain_password_2019 dbname=linkrbrain2019");
    ORM::Connection& connection = db.get_connection();
    converter = & connection.get_converter();

    db.link<Dataset>();
    // db.link<Test>();

    // Test test;
    // show(test);
    // std::cout << '\n';
    // //
    // Dataset dataset;
    // dataset.name = std::string("nom du test");
    // dataset.label = std::string("un label trop long");
    // show(dataset);
    // connection.save(dataset);
    // show(dataset);
    // dataset.label = "nouveau label beaucoup trop long";
    // show(dataset);
    // connection.save(dataset);
    // show(dataset);
    // // test.name.set_value("nom du test");
    //
    // auto d = connection.get<Dataset>(1);
    // show(d);
    // d.name = "renamed dataset";
    // show(d);
    // connection.reload(d);
    // show(d);
    // d.name = "renamed dataset";
    // show(d);
    // connection.save(d);
    // show(d);
    // connection.reload(d);
    // show(d);

    //

    std::cout << '\n';
    int counter = 0;
    auto results = connection.get_all<Dataset>();
    for (const Dataset& result : results) {
    // for (const Dataset& result : connection.get_all<Dataset>()) {
        std::cout << '\n';
        show(result);
        // if (++counter > 4) {
        //     break;
        // }
    }
    std::cout << '\n';

    return 0;
}
