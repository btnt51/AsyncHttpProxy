#include "session.h"
#include "headers.h"
#include <iostream>

boost::asio::awaitable<void> session::process() {
    for (;;) {
        auto target = co_await async_connect_from_client();
        if (not target)
            co_return;
        if (not co_await connect_to_remote(std::move(target.value()))) {
            co_return;
        }
        if (not co_await pipe()) {
            co_return;
        }
    }

}

boost::asio::awaitable<std::optional<boost::beast::http::request_header<>>> session::async_connect_from_client() {
    boost::system::error_code ec;
    co_await boost::beast::http::async_read_header(
        client_stream_, buf_, request_parser_,
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
        std::println(std::cerr, "Error occured while async reading: {}", ec.what());
        co_return std::nullopt;
    }
    boost::beast::error_code parser_error_code;
    if(parser_error_code.failed()) {
        std::println(std::cerr, "Error occured while parsing header: {}", parser_error_code.what());
        co_return std::nullopt;
    }
    auto message = request_parser_.get();
    co_return message.base();
}

boost::asio::awaitable<bool> session::connect_to_remote(boost::beast::http::request_header<> &&header) {
    auto host_value = header[boost::beast::http::field::host];
    if (host_value.empty()) {
        std::println(std::cerr, "Request header does not contains");
        co_return false;
    }
    auto new_host_port = findHostPort(host_value);
    if(not remote_stream_.socket().is_open() or cached_host_port not_eq new_host_port) {
        cached_host_port = new_host_port;
        boost::asio::ip::tcp::resolver resolver(remote_stream_.socket().get_executor());
        auto iter = co_await resolver.async_resolve(cached_host_port.host, cached_host_port.port, boost::asio::use_awaitable);

        auto [error_code_connet, _] = co_await boost::asio::async_connect(remote_stream_.socket(), iter, as_tuple(boost::asio::use_awaitable));
        if(error_code_connet) {
            std::println(std::cerr, "Error occured while parsing header: {}", error_code_connet.what());
            co_return false;
        }
    }
    boost::beast::http::request<boost::beast::http::empty_body> req;
    req.base() = std::move(header);

    boost::beast::http::serializer<true, boost::beast::http::empty_body> sr{req};
    sr.split(true);

    auto [error_code_write, _] = co_await boost::beast::http::async_write(remote_stream_, sr,as_tuple(boost::asio::use_awaitable));
    if(error_code_write) {
        std::println(std::cerr, "Error occured while sending header: {}", error_code_write.what());
        co_return false;
    }
//    auto [error_code_write, _] = co_await boost::asio::async_write(remote_socket_, boost::asio::buffer(header.begin(), header.end()), as_tuple(boost::asio::use_awaitable));
    co_return true;
}

boost::asio::awaitable<bool> session::reade_header_frome_remote() {

    co_return true;
}

boost::asio::awaitable<bool> session::pipe() {

    co_return true;
}