#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <utility>

#include "../src/headers.h"

TEST(iterHeaders, Empty) {
    std::string_view empty{};
    EXPECT_THROW({
        iterHeaders(empty, [](std::string_view, std::string_view){});
    }, std::runtime_error);
}

TEST(iterHeaders, SkipRequestLine) {
    const std::string hdr =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n";

    int calls = 0;
    std::string last_name, last_value;

    ASSERT_NO_THROW({
        iterHeaders(hdr, [&](std::string_view name, std::string_view value){
            ++calls;
            last_name  = std::string(name);
            last_value = std::string(value);
        });
    });

    EXPECT_EQ(calls, 1);
    EXPECT_EQ(last_name,  "Host");
    EXPECT_EQ(last_value, "example.com");
}

TEST(iterHeaders, SingleHeader) {
    const std::string hdr =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n";

    std::vector<std::pair<std::string,std::string>> got;
    ASSERT_NO_THROW({
        iterHeaders(hdr, [&](std::string_view n, std::string_view v){
            got.emplace_back(n, v);
        });
    });

    ASSERT_EQ(got.size(), 1u);
    EXPECT_EQ(got[0].first,  "Content-Type");
    EXPECT_EQ(got[0].second, "text/plain");
}

TEST(iterHeaders, MultipleHeaders) {
    const std::string hdr =
        "HTTP/1.1 200 OK\r\n"
        "Server: test\r\n"
        "Content-Length: 42\r\n"
        "Connection: keep-alive\r\n";

    std::vector<std::pair<std::string,std::string>> got;
    iterHeaders(hdr, [&](std::string_view n, std::string_view v){
        got.emplace_back(n, v);
    });

    ASSERT_EQ(got.size(), 3u);
    EXPECT_EQ(got[0].first,  "Server");
    EXPECT_EQ(got[0].second, "test");
    EXPECT_EQ(got[1].first,  "Content-Length");
    EXPECT_EQ(got[1].second, "42");
    EXPECT_EQ(got[2].first,  "Connection");
    EXPECT_EQ(got[2].second, "keep-alive");
}

TEST(iterHeaders, MultipleSameHeaders) {
    const std::string hdr =
        "HTTP/1.1 200 OK\r\n"
        "Cookie: a=1\r\n"
        "Cookie: b=2\r\n";

    std::vector<std::string> values;
    iterHeaders(hdr, [&](std::string_view n, std::string_view v){
        if (n == "Cookie") values.emplace_back(v);
    });

    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], "a=1");
    EXPECT_EQ(values[1], "b=2");
}

TEST(findHostPort, Simple) {
    const std::string req =
        "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:8080\r\n"
        "User-Agent: x\r\n";

    auto [host, port] = findHostPort(req);
    EXPECT_EQ(host, "127.0.0.1");
    EXPECT_EQ(port, "8080");
}

TEST(findHostPort, NoHost) {
    const std::string req =
        "GET / HTTP/1.1\r\n"
        "User-Agent: x\r\n";

    auto [host, port] = findHostPort(req);
    EXPECT_TRUE(host.empty());
    EXPECT_TRUE(port.empty());
}

TEST(findContentLength, Simple) {
    const std::string rsp =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12345\r\n"
        "Connection: close\r\n";

    auto len = findContentLength(rsp);
    ASSERT_TRUE(len.has_value());
    EXPECT_EQ(*len, static_cast<size_t>(12345));
}

TEST(findContentLength, NoContentLength) {
    const std::string rsp =
        "HTTP/1.1 204 No Content\r\n"
        "Server: test\r\n";

    auto len = findContentLength(rsp);
    EXPECT_FALSE(len.has_value());
}
