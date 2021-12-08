#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__TOKENSCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__TOKENSCONTROLLER_HPP


#include "DB/ORM/Controller.hpp"
#include "Logging/Loggable.hpp"
#include "Generators/Random.hpp"
#include "Network/Server/HTTP/Request.hpp"

#include "../Models/User.hpp"


namespace LinkRbrain::Controllers {

    class TokensController : public Logging::Loggable {
    public:

        const std::string& get_token(const Models::User& user) {
            // is it already in cache?
            auto it = _username_to_token.find(user.username);
            if (it != _username_to_token.end()) {
                return it->second;
            }
            // nope. then let's make a new one
            const std::string token = compute_token();
            _token_to_user.insert({token, user});
            return _username_to_token.insert({user.username, token}).first->second;
        }
        const Models::User& get_user(const std::string& token) {
            auto it = _token_to_user.find(token);
            if (it != _token_to_user.end()) {
                return it->second;
            }
            throw Exceptions::UnauthorizedException("Could not find user for this token", {
                {"token", token},
                {"problem", "nouser"}
            });
        }
        const Models::User& get_user(const Network::Server::HTTP::Request& request) {
            auto it = request.headers.find("Authorization");
            if (it == request.headers.end()) {
                throw Exceptions::UnauthorizedException("Missing header 'Authorization'", {
                    {"header", "Authorization"},
                    {"problem", "missing"}
                });
            }
            if (it->second.find("Bearer ") != 0) {
                throw Exceptions::UnauthorizedException("Header 'Authorization' should start with 'Bearer '", {
                    {"position", it->second.find("Bearer ")},
                    {"header", "Authorization"},
                    {"problem", "missingbearer"}
                });
            }
            const std::string token = it->second.substr(7);
            return get_user(token);
        }

    protected:

        const std::string compute_token(const size_t min_length=32, const size_t max_length=64) {
            static const std::string vowels = "aeiou";
            static const std::string consonants = "zrtypsdfghjklmwvbn";
            const size_t length = max_length ? Generators::Random::generate_number(min_length, max_length) : min_length;
            std::string token;
            do {
                token.clear();
                bool is_vowel = Generators::Random::generate<bool>();
                for (size_t i=0; i<length; ++i) {
                    if (i && i != length-1 && token[i-1] != '-' && Generators::Random::generate_number(4) == 0) {
                        token += '-';
                        is_vowel = Generators::Random::generate<bool>();
                    } else {
                        token += Generators::Random::pick(is_vowel ? vowels : consonants);
                        is_vowel = !is_vowel;
                    }
                }
            } while (_token_to_user.find(token) != _token_to_user.end());
            return token;
        }

        virtual const std::string get_logger_name() {
            return "TokensController";
        }

    private:

        std::unordered_map<std::string, Models::User> _token_to_user;
        std::unordered_map<std::string, std::string> _username_to_token;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__TOKENSCONTROLLER_HPP
