#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN_HPP


#include "CLI/Arguments/Command.hpp"

#include "LinkRbrain/Controllers/DBController.hpp"
#include "LinkRbrain/Controllers/DataController.hpp"




namespace LinkRbrain::Commands {

    typedef double T;
    static std::string data_path;
    static bool debug = false;

    LinkRbrain::Controllers::DataController<T>& _get_data_controller() {
        static std::shared_ptr<LinkRbrain::Controllers::DataController<T>> data_controller;
        if (!data_controller) {
            data_controller.reset(
                new LinkRbrain::Controllers::DataController<T>(data_path)
            );
        }
        return *data_controller;
    }

    LinkRbrain::Controllers::OrganController<T>& _get_organ_controller(const std::string& name_or_identifer) {
        if (std::all_of(name_or_identifer.begin(), name_or_identifer.end(), ::isdigit)) {
            const size_t identifier = std::stoul(name_or_identifer);
            return _get_data_controller().get_organ(identifier);
        } else {
            return _get_data_controller().get_organ(name_or_identifer);
        }
    }

    LinkRbrain::Controllers::DatasetController<T>& _get_dataset_controller(const std::string& organ, const std::string& dataset) {
        // fetch organ
        LinkRbrain::Controllers::OrganController<T>* organ_controller;
        if (std::all_of(organ.begin(), organ.end(), ::isdigit)) {
            const size_t organ_id = std::stoul(organ);
            organ_controller = & _get_data_controller().get_organ(organ_id);
        } else {
            organ_controller = & _get_data_controller().get_organ(organ);
        }
        // fetch dataset
        if (std::all_of(dataset.begin(), dataset.end(), ::isdigit)) {
            const size_t dataset_id = std::stoul(dataset);
            return organ_controller->get_dataset(dataset_id);
        } else {
            return organ_controller->get_dataset(dataset);
        }
    }
    LinkRbrain::Controllers::DatasetController<T>& _get_dataset_controller(LinkRbrain::Controllers::OrganController<T>& organ_controller, const std::string& dataset) {
        if (std::all_of(dataset.begin(), dataset.end(), ::isdigit)) {
            const size_t dataset_id = std::stoul(dataset);
            return organ_controller.get_dataset(dataset_id);
        } else {
            return organ_controller.get_dataset(dataset);
        }
    }


    static std::shared_ptr<Controllers::DBController> _db_controller;
    void _init_db_controller(const CLI::Arguments::CommandResult& options) {
        _db_controller.reset(new Controllers::DBController(
            DB::get_type_from_string(options.get("db-type")),
            options.get("db-connection")));
    }
    Controllers::DBController& _get_db_controller() {
        if (!_db_controller) {
            throw Exceptions::NotFoundException("DB controller should be initialized first");
        }
        return *_db_controller;
    }

    void linkrbrain(const CLI::Arguments::CommandResult& options) {
        // log level
        int log_level = Logging::Level::None;
        // debugging
        if (options.has("debug")) {
            debug = true;
            options.view();
            std::cout << std::string(64, '-') << "\n\n";
            log_level = Logging::Level::Detail;
        } else {
            // logging level
            const std::string log_level_name = options.get("log-level");
            log_level = Logging::get_level_by_name(log_level_name);
            if (log_level == -1) {
                throw Exceptions::BadDataException("Invalid log level: " + log_level_name, {});
            }
            Logging::set_level((Logging::Level) log_level);
            // logging outputs
            std::filesystem::create_directories("var/log/linkrbrain");
            Logging::add_output(Logging::Output::StandardError).set_color(true);
            Logging::add_output("var/log/linkrbrain").set_color(false);
        }
        // data location
        data_path = options.get("data");
    }


} // LinkRbrain::Commands


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN_HPP
