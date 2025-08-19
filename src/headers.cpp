#include "headers.h"

#include <algorithm>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

using Callback = std::function<void(std::string_view, std::string_view)>;


void iterHeaders(std::string_view header, Callback&& callback) {
    if(header.empty())
        throw std::runtime_error("Empty header");

    auto lines = header | std::views::split('\r') |
                 std::views::transform([](auto &&part) { return part | std::views::split('\n'); }) | std::views::join |
                 std::views::filter([](auto &&line) { return !line.empty(); }) | std::views::drop(1);

    std::ranges::for_each(lines, [&](auto&& line_view){
        std::string_view line = std::string_view(line_view.begin(), line_view.size());

        const size_t colon = line.find(':');
        if (colon == std::string_view::npos) {
            throw std::runtime_error("Invalid header format");
        }
        std::string_view field_name = line.substr(0, colon);
        constexpr std::size_t offset_to_data = 2;
        std::string_view field_value = line.substr(colon + offset_to_data);
        callback(field_name, field_value);
    });

}

std::pair<std::string, std::string> findHostPort(std::string_view req) {
    try {
        std::string host, port;
        iterHeaders(req, [&host, &port](std::string_view field_name, std::string_view field_value) {
            if (field_name == "Host") {
                size_t colon_pos = field_value.find(':');
                if (colon_pos != std::string_view::npos) {
                    host = std::string(field_value.substr(0, colon_pos));
                    port = std::string(field_value.substr(colon_pos + 1));
                } else {
                    host = std::string(field_value);
                    port = "80";
                }
            }
        });
        return {host, port};
    } catch (std::exception& e) {
        throw;
    }

}

std::optional<size_t> findContentLength(std::string_view rsp) {
    try {
        std::optional<std::size_t> result;
        iterHeaders(rsp, [&result](std::string_view field_name, std::string_view field_value) {
            if(field_name == "Content-Length") {
                std::size_t value{0};
                auto res = std::from_chars(field_value.data(), field_value.data() + field_value.size(), value);
                if(res.ec == std::errc()) {
                    result = value;
                } else {
                    result = std::nullopt;
                }
            }
        });
        return result;
    } catch (std::exception& e) {
        throw;
    }
}
