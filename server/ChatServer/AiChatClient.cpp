#include "AiChatClient.h"
#include "ConfigMgr.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <json/json.h>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <algorithm>

AiReply AiChatClient::Chat(const std::string& system_prompt,
    const std::vector<std::pair<std::string, std::string>>& messages) const {
    auto section = ConfigMgr::Inst()["AI"];
    if (section["Enabled"] == "false" || section["Enabled"] == "0") return {false, "", "AI service is disabled"};
    const std::string host = section["Host"].empty() ? "127.0.0.1" : section["Host"];
    const std::string port = section["Port"].empty() ? "8080" : section["Port"];
    std::string target = section["Path"].empty() ? "/v1/chat/completions" : section["Path"];
    if (target.front() != '/') target.insert(target.begin(), '/');
    const std::string model = section["Model"].empty() ? "qwen" : section["Model"];
    const int timeout_ms = std::max(1000, std::min(120000, section["TimeoutMs"].empty() ? 30000 : std::atoi(section["TimeoutMs"].c_str())));
    const int max_tokens = std::max(1, std::min(8192, section["MaxTokens"].empty() ? 512 : std::atoi(section["MaxTokens"].c_str())));
    const double temperature = std::max(0.0, std::min(2.0, section["Temperature"].empty() ? 0.7 : std::atof(section["Temperature"].c_str())));
    Json::Value body; body["model"] = model; body["temperature"] = temperature; body["max_tokens"] = max_tokens; body["stream"] = false;
    if (!system_prompt.empty()) { Json::Value item; item["role"] = "system"; item["content"] = system_prompt; body["messages"].append(item); }
    for (const auto& message : messages) { Json::Value item; item["role"] = message.first; item["content"] = message.second; body["messages"].append(item); }
    Json::StreamWriterBuilder writer; writer["indentation"] = "";
    try {
        namespace beast = boost::beast; namespace http = beast::http; namespace net = boost::asio; using tcp = net::ip::tcp;
        net::io_context io; tcp::resolver resolver(io); beast::tcp_stream stream(io);
        net::steady_timer timer(io);
        timer.expires_after(std::chrono::milliseconds(timeout_ms));
        bool timed_out = false;
        boost::system::error_code operation_error;
        unsigned int response_status = 0;
        std::string response_body;
        http::request<http::string_body> request(http::verb::post, target, 11);
        request.set(http::field::host, host); request.set(http::field::user_agent, "LinkChat-AI/1.1"); request.set(http::field::content_type, "application/json");
        request.body() = Json::writeString(writer, body); request.prepare_payload();
        http::response<http::string_body> response;
        beast::flat_buffer buffer;
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                timed_out = true;
                resolver.cancel();
                boost::system::error_code ignored;
                stream.socket().cancel(ignored);
                stream.socket().close(ignored);
            }
        });
        resolver.async_resolve(host, port, [&](const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
            if (ec) { operation_error = ec; timer.cancel(); return; }
            stream.async_connect(endpoints, [&](const boost::system::error_code& connect_error, const tcp::endpoint&) {
                if (connect_error) { operation_error = connect_error; timer.cancel(); return; }
                http::async_write(stream, request, [&](const boost::system::error_code& write_error, std::size_t) {
                    if (write_error) { operation_error = write_error; timer.cancel(); return; }
                    http::async_read(stream, buffer, response, [&](const boost::system::error_code& read_error, std::size_t) {
                        if (read_error) { operation_error = read_error; timer.cancel(); return; }
                        response_status = response.result_int();
                        response_body = response.body();
                        timer.cancel();
                    });
                });
            });
        });
        io.run();
        if (timed_out) return {false, "", "llama.cpp request timed out"};
        if (operation_error) return {false, "", operation_error.message()};
        if (response_status != static_cast<unsigned int>(http::status::ok)) return {false, "", "llama.cpp returned HTTP " + std::to_string(response_status)};
        Json::Value root; Json::CharReaderBuilder reader; std::string errors; std::istringstream input(response_body);
        if (!Json::parseFromStream(reader, input, &root, &errors)) return {false, "", "invalid llama.cpp JSON response: " + errors};
        if (!root["choices"].isArray() || root["choices"].empty() ||
            !root["choices"][0]["message"]["content"].isString())
            return {false, "", "AI response missing choices[0].message.content"};
        const std::string content = root["choices"][0]["message"]["content"].asString();
        if (content.empty()) return {false, "", "llama.cpp returned an empty response"};
        return {true, content, ""};
    } catch (const std::exception& ex) { return {false, "", ex.what()}; }
}
