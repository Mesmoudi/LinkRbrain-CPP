#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATACONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATACONTROLLER_HPP


#include <set>
#include <filesystem>

#include "Exceptions/Exception.hpp"
#include "Exceptions/GenericExceptions.hpp"
#include "Logging/Loggable.hpp"

#include "./OrganController.hpp"
#include "./DatasetController.hpp"
#include "LinkRbrain/Models/Organ.hpp"

#include "Types/DateTime.hpp"


namespace LinkRbrain::Controllers {

    template <typename T>
    class DataController : public Logging::Loggable {
    public:

        DataController(const std::filesystem::path& path) :
            _max_organ_id(0),
            _max_dataset_id(0)
        {
            load(path);
        }

        void load(const std::filesystem::path& path) {
            _path = path;
            _organ_controllers.clear();
            std::filesystem::create_directories(path);
            get_logger().debug("Start loading data controller from", path);
            if (! std::filesystem::is_directory(path)) {
                except("Not a directory:", path);
            }
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                const std::filesystem::path organ_path = entry.path();
                if (entry.is_directory()) {
                    try {
                        _organ_controllers.push_back(
                            std::make_shared<OrganController<T>>(organ_path)
                        );
                        auto& organ_controller = * _organ_controllers.back();
                        const size_t organ_id = organ_controller.get_instance().get_id();
                        _max_organ_id = std::max(_max_organ_id, organ_id);
                        get_logger().debug("organ_id =", organ_id);
                        for (auto& dataset_controller : organ_controller.get_datasets()) {
                            _dataset_controllers.push_back(dataset_controller);
                            _max_dataset_id = std::max(_max_dataset_id, dataset_controller->get_instance().get_id());
                        }
                    } catch (std::exception& e) {
                        get_logger().warning("Cannot load organ from ", organ_path, ": ", e.what());
                    }
                }
            }
            get_logger().message("Loaded data controller from folder ", path);
        }

        OrganController<T>& get_organ(const std::string& organ_label) {
            for (auto& organ_controller : _organ_controllers) {
                if (organ_controller->get_instance().get_label() == organ_label) {
                    return *organ_controller;
                }
            }
            throw Exceptions::NotFoundException("Cannot find organ: " + organ_label, {
                {"model", "Organ"},
                {"label", organ_label}
            });
        }
        OrganController<T>& get_or_add_organ(const std::string& organ_label) {
            for (auto& organ_controller : _organ_controllers) {
                if (organ_controller->get_instance().get_label() == organ_label) {
                    return *organ_controller;
                }
            }
            return add_organ(organ_label);
        }
        OrganController<T>& add_organ(const std::string& organ_label) {
            const size_t organ_id = ++_max_organ_id;
            // build path
            const std::filesystem::path organ_path
                = _path / (std::to_string(organ_id) + "_" + organ_label + "_" + (std::string) Types::DateTime::now());
            // instanciate corresponding controller
            const size_t index = _organ_controllers.size();
            _organ_controllers.push_back(
                std::make_shared<OrganController<T>>(organ_id, organ_path, organ_label)
            );
            return * _organ_controllers[index];
        }

        const bool has_organ(const std::string& organ_label) const {
            for (auto& organ_controller : _organ_controllers) {
                if (organ_controller->get_instance().get_label() == organ_label) {
                    return true;
                }
            }
            return false;
        }
        OrganController<T>& get_organ(const size_t organ_id) {
            for (auto& organ_controller : _organ_controllers) {
                if (organ_controller->get_instance().get_id() == organ_id) {
                    return *organ_controller;
                }
            }
            throw Exceptions::NotFoundException("Cannot find organ with given identifier: " + std::to_string(organ_id), {
                {"resource", "organs"},
                {"identifier", organ_id}
            });
        }
        const std::vector<std::shared_ptr<OrganController<T>>> get_organs() const {
            return _organ_controllers;
        }

        void remove_organ(OrganController<T>& organ_controller) {
            // delete files
            organ_controller.remove();
            // remove from data controller collection
            for (auto it=_organ_controllers.begin(); it!=_organ_controllers.end(); ++it) {
                if (it->get() == &organ_controller) {
                    _organ_controllers.erase(it);
                    break;
                }
            }
        }

        DatasetController<T>& add_dataset(OrganController<T>& organ_controller, const std::string& dataset_label) {
            organ_controller.add_dataset(++_max_dataset_id, dataset_label);
            _dataset_controllers.push_back(organ_controller.get_datasets().back());
            return * _dataset_controllers.back();
        }
        DatasetController<T>& add_dataset(const size_t organ_id, const std::string& dataset_label) {
            return add_dataset(get_organ(organ_id), dataset_label);
        }
        DatasetController<T>& get_dataset(const size_t dataset_id) {
            for (const auto& dataset_controller : _dataset_controllers) {
                if (dataset_controller->get_instance().get_id() == dataset_id) {
                    return *dataset_controller;
                }
            }
            throw Exceptions::NotFoundException("Cannot find dataset with given identifier", {
                {"resource", "datasets"},
                {"identifier", dataset_id}
            });
        }
        const std::vector<std::shared_ptr<DatasetController<T>>> get_datasets() const {
            return _dataset_controllers;
        }

    protected:

        virtual const std::string get_logger_name() {
            return "DataController";
        }

    private:

        std::filesystem::path _path;
        std::vector<std::shared_ptr<OrganController<T>>> _organ_controllers;
        std::vector<std::shared_ptr<DatasetController<T>>> _dataset_controllers;
        size_t _max_organ_id;
        size_t _max_dataset_id;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATACONTROLLER_HPP
