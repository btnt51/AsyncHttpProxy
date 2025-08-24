#include "headers.h"

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast.hpp>

#include <string_view>
#include <iostream>
#include <print>

using boost::asio::io_service;
using boost::asio::co_spawn;
using boost::asio::async_read_until;
using boost::asio::async_read;
using boost::asio::awaitable;
using boost::asio::use_awaitable;
using boost::system::error_code;
using boost::asio::buffer;
using boost::asio::dynamic_buffer;
using boost::asio::transfer_at_least;
using boost::asio::ip::tcp;
using boost::asio::detached;
using boost::asio::streambuf;


constexpr std::string_view delimiter = "\r\n\r\n";

awaitable<void> connect_to_remote_host(std::string_view header_response, tcp::socket &remote_host) {
    std::string host, port;
    try {
//        std::tie(host, port) = findHostPort(header_response);
    } catch (std::exception& e) {
        std::println("Error occured while parsing header: {}", e.what());
        co_return;
    }
    tcp::resolver resolver(remote_host.get_executor());
    auto iter = co_await resolver.async_resolve(host, port, use_awaitable);
    if (not remote_host.is_open())
        co_await async_connect(remote_host, iter, use_awaitable);

    co_await async_write(remote_host, boost::asio::buffer(header_response.data(), header_response.size()), use_awaitable);
}

awaitable<std::optional<size_t>> read_header(tcp::socket &remote_host, tcp::socket& client_socket) {
    streambuf buf;
    std::size_t header_size = co_await async_read_until(remote_host, buf, delimiter, use_awaitable);
    auto bufs = buf.data();
    std::string content(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + buf.size());
    std::string header_response = content.substr(0, header_size);
    std::optional<std::size_t> cl;
    try {
        cl = findContentLength(header_response);
    } catch (std::exception& e) {
        std::println("Error occured while parsing header: {}", e.what());
        co_return std::nullopt;
    }
    co_await async_write(client_socket, buffer(buf.data(), buf.size()), use_awaitable);
    if(cl and header_response.size() < buf.size()) {
        cl.value() -= buf.size()-header_response.size();
    }
    co_return cl;
}



awaitable<void> pipe(tcp::socket &client_socket, tcp::socket &remote_host, std::size_t remaining) {
    std::size_t current_read{0};
    while(current_read < remaining) {
        std::string content_from_remote;
        content_from_remote.resize(remaining - current_read);
        std::size_t readed_data = co_await async_read(remote_host, buffer(content_from_remote), use_awaitable);
        std::string_view content2{content_from_remote.data(), readed_data};
        current_read += readed_data;
        std::println( "{}", content2);
        co_await async_write(client_socket, buffer(content_from_remote, readed_data), use_awaitable);
    }
}

awaitable<void> session(tcp::socket client_socket, io_service& io_service) {
    tcp::socket remote_host(io_service);
    for (;;) {
        streambuf buf;
        std::size_t header_size = co_await async_read_until(client_socket, buf, delimiter, use_awaitable);
        auto bufs = buf.data();
        std::string content(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + buf.size());
        std::string header_request = content.substr(0, header_size);

        co_await connect_to_remote_host(header_request, remote_host);
        auto remaining = co_await read_header(remote_host, client_socket);
        if (remaining) {
            co_await pipe(client_socket, remote_host, remaining.value());
        }
    }
}
#include "session.h"
awaitable<void> session2(tcp::socket client_socket, io_service& io_service) {
//    tcp::socket remote_host(io_service);
//    for (;;) {
//        streambuf buf;
//        std::size_t header_size = co_await async_read_until(client_socket, buf, delimiter, use_awaitable);
//        auto bufs = buf.data();
//        std::string content(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + buf.size());
//        std::string header_request = content.substr(0, header_size);
//
//        co_await connect_to_remote_host(header_request, remote_host);
//        auto remaining = co_await read_header(remote_host, client_socket);
//        if (remaining) {
//            co_await pipe(client_socket, remote_host, remaining.value());
//        }
//    }
    class session session(io_service, std::move(client_socket));
    co_await session.process();
}


class Server
{
public:
    Server(io_service& io_service, short port)
    : io_service_(io_service)
    , acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        co_spawn(io_service_, do_accept(), detached);
    }

private:
    awaitable<void> do_accept()
    {
      while(true) {
          auto socket = co_await acceptor_.async_accept(use_awaitable);
          co_spawn(io_service_.get_executor(), session2(std::move(socket), io_service_), detached);
      }
    }

    io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: proxy_server";
      std::cerr << " <listen_port>\n";
      return 1;
    }
    io_service io_service(1);
    Server server(io_service, std::atoi(argv[1]));
    io_service.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
