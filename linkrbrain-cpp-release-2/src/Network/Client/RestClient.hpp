#ifndef LINKRBRAIN2019__SRC__NETWORK__RESTCLIENT_HPP
#define LINKRBRAIN2019__SRC__NETWORK__RESTCLIENT_HPP



#include <string>

#include <curl/curl.h>

#include "Exceptions/Exception.hpp"
#include "Types/Variant.hpp"
#include "Buffering/Format.hpp"
#include "Buffering/Writing/StringWriter.hpp"
#include "Buffering/Reading/StringReader.hpp"
#include "./RestResponse.hpp"
#include "Logging/Loggable.hpp"


namespace Network::Client {

    class RestClient : public Logging::Loggable {
    public:

        inline RestClient(const std::string& base_url="", const Buffering::Format& format=Buffering::JSON) :
            _base_url(base_url),
            _format(format),
            _headers(NULL),
            _headers_nopayload(NULL),
            _timeout(5)
        {
            switch (_format) {
                case Buffering::JSON:
                    _headers = curl_slist_append(_headers, "Content-Type: application/json");
                    _headers = curl_slist_append(_headers, "Accept: application/json");
                    _headers_nopayload = curl_slist_append(_headers_nopayload, "Accept: application/json");
                    break;
                default:
                    except("Unsupported format for RestClient:", Buffering::get_format_name(_format));
            }
        }
        inline ~RestClient() {
            curl_slist_free_all(_headers);
            curl_slist_free_all(_headers_nopayload);
        }

        inline void set_timeout(const int timeout) {
            _timeout = timeout;
        }

        inline const RestResponse request(const std::string& method, const std::string& url, const Types::Variant& data=Types::Variant::Undefined) const {
            const std::string full_url = _base_url + url;
            CURL *curl = curl_easy_init();
            if (!curl) {
                except("Could not initialize cURL");
            }
            //
            curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "cpptrading");
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, _timeout);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);
            //
            Buffering::Writing::StringWriter request_body_writer(_format);
            request_body_writer << data;
            if (method == "GET" || method == "DELETE") {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers_nopayload);
            } else {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body_writer.get_string().c_str());
            }
            //
            std::string response_body;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
            //
            long http_code = 0;
            CURLcode result = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (result != CURLE_OK) {
                except("Could not perform cURL", method, full_url, curl_easy_strerror(result));
            }
            //
            try {
                Buffering::Reading::StringReader response_body_reader(response_body, _format);
                Types::Variant response_data;
                response_body_reader >> response_data;
                return {
                    .method = method,
                    .url = full_url,
                    .code = http_code,
                    .data = response_data
                };
            } catch (const std::exception& exception) {
                // std::cerr << "COULD NOT PARSE JSON IN REST RESPONSE: " << response_body << '\n';
                return {
                    .method = method,
                    .url = full_url,
                    .code = http_code,
                    .data = exception.what()
                };
            }
        }

        inline const RestResponse get(std::string url) const {
            return request("GET", url);
        }
        inline const RestResponse post(std::string url, const Types::Variant& data) const {
            return request("POST", url, data);
        }
        inline const RestResponse patch(std::string url, const Types::Variant& data) const {
            return request("PATCH", url, data);
        }
        inline const RestResponse del(std::string url) const {
            return request("DELETE", url);
        }
        inline const RestResponse put(std::string url, const Types::Variant& data) const {
            return request("PUT", url, data);
        }

    private:

        static const bool initialize() {
            return (curl_global_init(CURL_GLOBAL_ALL) == 0);
        }

        static int writer(const char *data, const size_t size, const size_t nmemb, std::string* _buffer) {
            std::string& buffer = * _buffer;
            buffer.append(
                data,
                size * nmemb
            );
            return size * nmemb;
        }

        static bool _is_curl_initialized;
        std::string _base_url;
        Buffering::Format _format;
        struct curl_slist* _headers;
        struct curl_slist* _headers_nopayload;
        int _timeout;

    };

    bool RestClient::_is_curl_initialized = RestClient::initialize();

} // Network::Client


#endif // LINKRBRAIN2019__SRC__NETWORK__RESTCLIENT_HPP
