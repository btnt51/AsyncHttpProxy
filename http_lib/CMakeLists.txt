cmake_minimum_required(VERSION 3.31.6)
project(AsyncHttpLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Все предупреждения - это ошибки
    add_compile_options(-Wfatal-errors -Wall -Werror)

    # Не больше одной ошибки за раз
    add_compile_options(-fmax-errors=1)
endif()

#set(CMAKE_PREFIX_PATH /usr/local/asio)

# Ищем необходимые библиотеки
find_package(Boost REQUIRED COMPONENTS system chrono thread filesystem)

file(GLOB HEADER_FILES "src/*.hpp")
file(GLOB SOURCES_FILES "src/*.cpp")


# Создаём статическую библиотеку
add_library(${PROJECT_NAME} STATIC
        ${SOURCES_FILES}
        ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})