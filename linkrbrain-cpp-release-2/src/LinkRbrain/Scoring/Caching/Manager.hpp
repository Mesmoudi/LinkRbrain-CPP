#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MANAGER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MANAGER_HPP


#include "./MemoryScorerCache.hpp"
#include "./FileScorerCache.hpp"
#include "./MappedFileScorerCache.hpp"
#include "Conversion/Binary.hpp"

#include <fstream>
#include <filesystem>
#include <algorithm>


namespace LinkRbrain::Scoring::Caching {

    enum Type {
        Memory = 1,
        File = 2,
        MappedFile = 3,
    };

    struct Manager {

        template <typename T, typename T2>
        static ScorerCache<T>* make(const LinkRbrain::Scoring::Caching::Type& caching_type, const std::vector<Group<T2>>& groups, const std::filesystem::path& path=".") {
            switch (caching_type) {
                case Memory:
                    return new LinkRbrain::Scoring::Caching::MemoryScorerCache<T>(groups);
                case File:
                    return new LinkRbrain::Scoring::Caching::FileScorerCache<T>(groups, path);
                case MappedFile:
                    return new LinkRbrain::Scoring::Caching::MappedFileScorerCache<T>(groups, path);
                default:
                    except("Not implemented");
            }
        }

        template <typename T>
        static void save(ScorerCache<T>* cache, const std::filesystem::path& path) {
            // initialize formatter
            std::ofstream buffer(path.native());
            // write cache type name
            std::string type_name = cache->get_type_name();
            type_name.resize(64, '\0');
            buffer.write(type_name.data(), 64);
            // write cache
            to_binary(buffer, *cache);
        }

        template <typename T>
        static std::shared_ptr<ScorerCache<T>> load(const std::filesystem::path& path) {
            // initialize parser
            std::ifstream buffer(path.native());
            // read cache type name
            std::string type_name(64, '\0');
            buffer.read(& type_name[0], 64);
            type_name.erase(std::find(type_name.begin(), type_name.end(), '\0'), type_name.end());
            // initialize cache
            std::shared_ptr<ScorerCache<T>> cache;
            if (type_name == "MemoryScorerCache") {
                cache.reset(new MemoryScorerCache<T>());
            } else if (type_name == "FileScorerCache") {
                cache.reset(new FileScorerCache<T>());
            } else if (type_name == "MappedFileScorerCache") {
                cache.reset(new MappedFileScorerCache<T>());
            } else {
                except("Unrecognized cache type: " + type_name);
            }
            // read cache
            cache(buffer, *cache);
            return cache;
        }

    };

} // LinkRbrain::Scoring::Caching


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MANAGER_HPP
