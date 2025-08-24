#ifndef ASYNCHTTPPROXY_SESSION_H
#define ASYNCHTTPPROXY_SESSION_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>

struct HostPort {
    std::string host;
    std::string port = "80";

    bool operator==(const HostPort& hostPort) const {
        if (host != hostPort.host)
            return false;
        return port == hostPort.port;
    }

    bool operator!=(const HostPort& hostPort) const {
        return not (*this == hostPort);
    }
};

class session {
    boost::asio::io_context& context_;
    boost::beast::tcp_stream client_stream_;
    boost::beast::tcp_stream remote_stream_;
    boost::beast::http::request_parser<boost::beast::http::empty_body> request_parser_;
    boost::beast::http::response_parser<boost::beast::http::buffer_body> response_parser_;
    HostPort cached_host_port;
    boost::beast::flat_buffer buf_;


public:
    session(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& client_socket) :
         context_(io_context), client_stream_(std::move(client_socket)),
         remote_stream_(context_.get_executor())
    {}

    boost::asio::awaitable<void> process();
    boost::asio::awaitable<std::optional<boost::beast::http::request_header<>>> async_connect_from_client();
    boost::asio::awaitable<bool> connect_to_remote(boost::beast::http::request_header<> &&target);
    boost::asio::awaitable<bool> reade_header_frome_remote();
    boost::asio::awaitable<bool> pipe();
};

#endif  // ASYNCHTTPPROXY_SESSION_H
