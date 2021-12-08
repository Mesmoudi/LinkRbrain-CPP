#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__ORGAN_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__ORGAN_HPP


#include <csignal>

#include "CLI/Arguments/Command.hpp"
#include "Types/Table.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    void organ(const CLI::Arguments::CommandResult& options) {
    }

    void organ_list(const CLI::Arguments::CommandResult& options) {
        Types::Table table;
        size_t count = 0;
        table.set_header({"Identifier", "Label", "Datasets"});
        for (const auto& organ_controller : _get_data_controller().get_organs()) {
            ++count;
            table.add_row({
                std::to_string(organ_controller->get_instance().get_id()),
                organ_controller->get_instance().get_label(),
                std::to_string(organ_controller->get_datasets().size())
            });
        }
        std::cout << count  << " organs are registered in '" << data_path << "':\n" << table;
    }

    void organ_add(const CLI::Arguments::CommandResult& options) {
        auto& data_controller = _get_data_controller();
        auto& organ_label = options.get("label");
        if (options.has("if-not-exists") && data_controller.has_organ(organ_label)) {
            auto& organ = data_controller.get_organ(options.get("label")).get_instance();
            std::cout << "Organ '" << organ.get_label() << "' already exists with identifier " << organ.get_id() << '\n';
        } else {
            auto& organ = data_controller.add_organ(options.get("label")).get_instance();
            std::cout << "Created organ '" << organ.get_label() << "' with identifier " << organ.get_id() << '\n';
        }
    }

    void organ_remove(const CLI::Arguments::CommandResult& options) {
        // retrieve organ & dataset controller
        auto& data_controller = _get_data_controller();
        auto& organ_controller = _get_organ_controller(options.get("id"));
        // initialize final message
        std::stringstream message;
        message << "Deleted organ '" << organ_controller.get_instance().get_label() << "' with identifier " << organ_controller.get_instance().get_id() << '\n';
        // delete
        data_controller.remove_organ(organ_controller);
        // the end!
        std::cout << message.str();
    }

} // LinkRbrain::Commands


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__ORGAN_HPP
