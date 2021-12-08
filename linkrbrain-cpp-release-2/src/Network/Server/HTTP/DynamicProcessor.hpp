#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__DYNAMICPROCESSOR_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__DYNAMICPROCESSOR_HPP


#include "./Processor.hpp"
#include "./BaseResource.hpp"
#include "./ParametrizedResource.hpp"

#include <vector>
#include <typeinfo>


namespace Network::Server::HTTP {


    struct DynamicProcessor : public Processor {
    public:

        DynamicProcessor() :
            _parameter_pointer(NULL),
            _parameter_type_hash(0) {}

        virtual const bool process(Connection& connection) {
            for (auto& resource : _resources) {
                if (resource->match(connection)) {
                    resource->process(connection);
                    return true;
                }
            }
            return false;
        }


        void add_resource(BaseResource* resource) {
            _resources.push_back(
                std::shared_ptr<BaseResource>(resource)
            );
        }
        // BaseResource
        template <typename Resource, std::enable_if_t<std::is_base_of<BaseResource, Resource>::value, int> = 0, std::enable_if_t<!std::is_same<typename Resource::ParameterType, void>::value, int> = 0>
        void add_resource(const std::string& url_pattern) {
            if (_parameter_pointer == NULL) {
                except("You should define a resource parameter first");
            }
            typedef typename Resource::ParameterType ParameterType;
            if (typeid(ParameterType).hash_code() != _parameter_type_hash) {
                except("Resource parameter type do not match");                
            }
            add_resource(
                new Resource(url_pattern, * (ParameterType *) _parameter_pointer)
            );
        }
        // ParametrizedResource
        template <typename Resource, std::enable_if_t<std::is_base_of<BaseResource, Resource>::value, int> = 0, std::enable_if_t<std::is_same<typename Resource::ParameterType, void>::value, int> = 0>
        void add_resource(const std::string& url_pattern) {
            add_resource(
                new Resource(url_pattern)
            );
        }

        template <typename T>
        void set_resource_parameter(T& parameter) {
            _parameter_pointer = &parameter;
            _parameter_type_hash = typeid(parameter).hash_code();
        }

    private:

        std::vector<std::shared_ptr<BaseResource>> _resources;
        void* _parameter_pointer;
        size_t _parameter_type_hash;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__DYNAMICPROCESSOR_HPP
