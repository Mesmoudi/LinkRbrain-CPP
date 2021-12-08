#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__ROUTINGPROCESSOR_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__ROUTINGPROCESSOR_HPP


#include "./Processor.hpp"
#include "./Redirection.hpp"

#include <vector>


namespace Network::Server::HTTP {


    struct RoutingProcessor : public Processor {
    public:

        void add_redirection(
            const std::string& pattern,
            const std::string& replacement,
            Redirection::Type type,
            const bool is_last)
        {
            _redirections.push_back({
                .pattern = std::regex(pattern),
                .replacement = replacement,
                .type = type,
                .is_last = is_last
            });
        }

        virtual const bool process(Connection& connection) {
            if (_redirections.size() == 0) {
                return false;
            }
            std::string url = connection.request.url;
            std::string old_url;
            while (true) {
                bool nochange = true;
                old_url = url;
                for (const Redirection& redirection : _redirections) {
                    if (!std::regex_match(url, redirection.pattern)) {
                        continue;
                    }
                    url = std::regex_replace(
                        url,
                        redirection.pattern,
                        redirection.replacement
                    );
                    switch (redirection.type) {
                        case Redirection::Permanent:
                        case Redirection::Temporary:
                            connection.response.code = redirection.type;
                            connection.response.headers["Location"] = url;
                            return true;
                        case Redirection::Invisible:
                            connection.request.url = url;
                            if (redirection.is_last) {
                                return false;
                            }
                    }
                    nochange = false;
                }
                if (nochange) {
                    return false;
                }
            }
        }

    private:

        std::vector<Redirection> _redirections;
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__ROUTINGPROCESSOR_HPP
