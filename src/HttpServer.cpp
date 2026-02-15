#include "HttpServer.h"

// I log directly to stdout here to keep the server lightweight
// and dependency-free.
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <utility>

#include "WeatherClient.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

// I inject all dependencies so the server does not own application state.
HttpServer::HttpServer(const std::string& addr, int p, Database& database, AuthService& authService)
    : address(addr), port(p), db(database), auth(authService) {}

static std::string getBearerToken(const http::request<http::string_body>& req) {
    auto it = req.find(http::field::authorization);
    if (it == req.end()) return "";

    std::string value(it->value().data(), it->value().size());
    const std::string prefix = "Bearer ";
    if (value.rfind(prefix, 0) != 0) return "";

    return value.substr(prefix.size());
}

void HttpServer::run() {
    boost::asio::io_context ioc{1};
    tcp::acceptor acceptor{
        ioc,
        { boost::asio::ip::make_address(address),
          static_cast<unsigned short>(port) }
    };

    std::cout << "Server running at http://" << address << ":" << port << "\n";

    for (;;) {
        tcp::socket socket{ioc};
        acceptor.accept(socket);

        boost::beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_headers, "Authorization, Content-Type");
        res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");

        // HARD STOP for CORS preflight
        if (req.method() == http::verb::options) {
            res.result(http::status::ok);
            res.body() = "";
            res.prepare_payload();
            http::write(socket, res);
            socket.shutdown(tcp::socket::shutdown_send);
            continue;
        }

        try {
            if (req.method() == http::verb::get && req.target() == "/health") {
                res.body() = R"({"status":"ok"})";
            }

            else if (req.method() == http::verb::post && req.target() == "/auth/register") {
                auto body = nlohmann::json::parse(req.body());
                bool ok = auth.registerUser(
                    body["username"].get<std::string>(),
                    body["password"].get<std::string>()
                );
                res.body() = nlohmann::json{{"success", ok}}.dump();
            }

            else if (req.method() == http::verb::post && req.target() == "/auth/login") {
                auto body = nlohmann::json::parse(req.body());
                int userId = auth.loginUser(
                    body["username"].get<std::string>(),
                    body["password"].get<std::string>()
                );

                if (userId < 0) {
                    res.result(http::status::unauthorized);
                    res.body() = R"({"error":"invalid credentials"})";
                } else {
                    res.body() = nlohmann::json{
                        {"token", sessions.createSession(userId, body["username"])},
                        {"username", body["username"]}
                    }.dump();
                }
            }

            else if (req.method() == http::verb::post && req.target() == "/weather/current") {
                Session session;
                if (!sessions.validateToken(getBearerToken(req), session)) {
                    res.result(http::status::unauthorized);
                    res.body() = R"({"error":"unauthorized"})";
                } else {
                    auto body = nlohmann::json::parse(req.body());
                    WeatherClient weather;
                    std::string summary = weather.getWeather(body["city"]);
                    db.logQuery(session.userId, body["city"], summary);
                    res.body() = nlohmann::json{{"summary", summary}}.dump();
                }
            }

            else if (req.method() == http::verb::get && req.target() == "/history") {
                Session session;
                if (!sessions.validateToken(getBearerToken(req), session)) {
                    res.result(http::status::unauthorized);
                    res.body() = R"({"error":"unauthorized"})";
                } else {
                    auto rows = db.getHistory(session.userId);
                    std::vector<nlohmann::json> out;
                    for (const auto& r : rows) {
                        out.push_back({
                            {"timestamp", r.timestamp},
                            {"city", r.city},
                            {"summary", r.summary}
                        });
                    }
                    res.body() = nlohmann::json(out).dump();
                }
            }

            else {
                res.result(http::status::not_found);
                res.body() = R"({"error":"not found"})";
            }
        }
        catch (const std::exception& e) {
            res.result(http::status::bad_request);
            res.body() = nlohmann::json{{"error", e.what()}}.dump();
        }

        res.prepare_payload();
        http::write(socket, res);
        socket.shutdown(tcp::socket::shutdown_send);
    }
}
