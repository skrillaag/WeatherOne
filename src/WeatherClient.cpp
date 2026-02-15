#include "WeatherClient.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <cstdlib>
#include <sstream>
#include <stdexcept>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

// I use a minimal string-based extractor because I only need
// a few top-level values and want to avoid a full JSON dependency here.
static std::string extract(const std::string& src, const std::string& key) {
    auto pos = src.find(key);
    if (pos == std::string::npos) return "N/A";
    pos = src.find(':', pos);
    if (pos == std::string::npos) return "N/A";
    auto end = src.find_first_of(",}", pos + 1);
    if (end == std::string::npos) end = src.size();

    std::string raw = src.substr(pos + 1, end - pos - 1);
    size_t start = raw.find_first_not_of(" \"\t");
    size_t finish = raw.find_last_not_of(" \"\t");
    if (start == std::string::npos || finish == std::string::npos) return "N/A";
    return raw.substr(start, finish - start + 1);
}

// I read the API key from the environment so secrets never live in code.
std::string WeatherClient::getApiKey() const {
    const char* key = std::getenv("WEATHERAPI_KEY");
    return key ? std::string(key) : std::string();
}

std::string WeatherClient::getWeather(const std::string& city) {
    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        return "WEATHERAPI_KEY not set";
    }

    try {
        const std::string host = "api.weatherapi.com";
        const std::string port = "443";
        const std::string target = "/v1/current.json?key=" + apiKey + "&q=" + city;

        boost::asio::io_context ioc;
        boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};

        // I rely on system trust stores to validate HTTPS certificates.
        ctx.set_default_verify_paths();

        boost::asio::ssl::stream<tcp::socket> stream{ioc, ctx};

        tcp::resolver resolver{ioc};
        auto const results = resolver.resolve(host, port);
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());

        // I explicitly perform the TLS handshake before sending the request.
        stream.handshake(boost::asio::ssl::stream_base::client);

        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "WeatherApp");

        http::write(stream, req);

        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        boost::system::error_code ec;
        stream.shutdown(ec);

        // I format a short, human-readable summary instead of returning raw JSON.
        std::string body = res.body();
        std::ostringstream out;
        out << "Weather in " << city
            << " | Temp " << extract(body, "\"temp_c\"")
            << " C | Wind " << extract(body, "\"wind_kph\"")
            << " kph";
        return out.str();
    }
    catch (const std::exception& ex) {
        // I surface failures as text so callers can display them directly.
        return std::string("Error: ") + ex.what();
    }
}
