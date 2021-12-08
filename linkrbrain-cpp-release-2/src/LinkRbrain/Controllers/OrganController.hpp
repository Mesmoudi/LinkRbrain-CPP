#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__ORGANCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__ORGANCONTROLLER_HPP


#include "Exceptions/GenericExceptions.hpp"

#include "LinkRbrain/Models/Organ.hpp"
#include "./DatasetController.hpp"


namespace LinkRbrain::Controllers {

    template <typename T>
    class OrganController : public Logging::Loggable {
    public:

        OrganController(const size_t id, const std::filesystem::path& path, const std::string& label) {
            _organ.reset(
                new Models::Organ<T>(id, label)
            );
            save(path);
        }
        OrganController(const std::filesystem::path& path) {
            load(path);
        }

        Models::Organ<T>& get_instance() {
            if (_organ.get() == NULL) {
                except("No organ instance");
            }
            return *_organ;
        }
        const Models::Organ<T>& get_instance() const {
            if (_organ.get() == NULL) {
                except("No organ instance");
            }
            return *_organ;
        }

        void load(const std::filesystem::path& path) {
            _path = path;
            // load organ instance
            std::ifstream data_buffer(path / "data");
            _organ.reset(new Models::Organ<T>(data_buffer));
            get_logger().notice("Loaded organ data from folder ", path / "data");
            // load dataset controllers
            _dataset_controllers.clear();
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                const std::filesystem::path dataset_path = entry.path();
                get_logger().notice("Loading dataset from ", dataset_path);
                if (entry.is_directory()) {
                    try {
                        _dataset_controllers.push_back(
                            std::make_shared<DatasetController<T>>(dataset_path)
                        );
                    } catch (std::exception& e) {
                        get_logger().warning("Cannot load dataset from ", dataset_path, ": ", e.what());
                    }
                }
            }
            get_logger().message("Loaded organ controller from folder ", path);
        }
        void save(const std::filesystem::path& path) {
            // make directory
            std::filesystem::create_directories(path);
            // save organ instance
            if (_organ.get() == NULL) {
                except("No organ instance");
            }
            std::ofstream buffer(path / "data");
            Conversion::Binary::serialize(buffer, *_organ);
            // save datasets
            for (auto& dataset_controller : _dataset_controllers) {
                dataset_controller->save();
            }
            // conclude
            _path = path;
            get_logger().message("Saved organ controller to folder", path);
        }
        void remove() {
            _dataset_controllers.clear();
            std::filesystem::remove_all(_path);
            get_logger().warning("Removed organ located at", _path);
        }

        DatasetController<T>& get_dataset(const std::string& dataset_label) {
            for (auto& dataset_controller : _dataset_controllers) {
                if (dataset_controller->get_instance().get_label() == dataset_label) {
                    return * dataset_controller;
                }
            }
            throw Exceptions::NotFoundException("Cannot find dataset with label: " + dataset_label, {
                {"model", "Dataset"},
                {"label", dataset_label}
            });
        }
        DatasetController<T>& get_dataset(const size_t& dataset_id) {
            for (auto& dataset_controller : _dataset_controllers) {
                if (dataset_controller->get_instance().get_id() == dataset_id) {
                    return * dataset_controller;
                }
            }
            throw Exceptions::NotFoundException("Cannot find dataset with identifier: " + std::to_string(dataset_id), {
                {"model", "Dataset"},
                {"identifier", dataset_id}
            });
        }
        DatasetController<T>& get_or_add_dataset(const std::string& dataset_label) {
            for (DatasetController<T>& dataset_controller : _dataset_controllers) {
                if (dataset_controller.get_instance().get_label() == dataset_label) {
                    return dataset_controller;
                }
            }
            return add_dataset(dataset_label);
        }
        DatasetController<T>& add_dataset(const size_t dataset_id, const std::string& dataset_label) {
            // build path
            const std::filesystem::path dataset_path
                = _path / (std::to_string(dataset_id) + "_" + dataset_label + "_" + (std::string) Types::DateTime::now());
            // instanciate corresponding controller
            const size_t index = _dataset_controllers.size();
            _dataset_controllers.push_back(
                std::make_shared<DatasetController<T>>(_organ->get_id(), dataset_id, dataset_path, dataset_label)
            );
            return * _dataset_controllers[index];
        }
        void remove_dataset(DatasetController<T>& dataset_controller) {
            // delete files
            dataset_controller.remove();
            // remove from organ controller collection
            for (auto it=_dataset_controllers.begin(); it!=_dataset_controllers.end(); ++it) {
                if (it->get() == &dataset_controller) {
                    _dataset_controllers.erase(it);
                    break;
                }
            }
        }

        std::vector<std::shared_ptr<DatasetController<T>>>& get_datasets() {
            return _dataset_controllers;
        }
        const std::vector<std::shared_ptr<DatasetController<T>>>& get_datasets() const {
            return _dataset_controllers;
        }

        void serialize(Types::Variant& destination, const bool with_datasets=true) const {
            destination = {
                {"id", _organ->get_id()},
                {"label", _organ->get_label()},
                {"metadata", _organ->get_metadata()}
            };
            if (with_datasets) {
                const size_t n = _dataset_controllers.size();
                destination["datasets"].set_vector();
                std::vector<Types::Variant>& datasets = destination["datasets"].get_vector();
                datasets.resize(n);
                for (size_t i=0; i<n; ++i) {
                    _dataset_controllers[i]->serialize(datasets[i], false);
                }
            }
        }

    protected:

        virtual const std::string get_logger_name() {
            if (_organ) {
                return "OrganController[" + std::to_string(_organ->get_id()) + "|" + _organ->get_label() + "]";
            }
            return "OrganController";
        }

    private:

        std::filesystem::path _path;
        std::shared_ptr<Models::Organ<T>> _organ;
        std::vector<std::shared_ptr<DatasetController<T>>> _dataset_controllers;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__ORGANCONTROLLER_HPP
