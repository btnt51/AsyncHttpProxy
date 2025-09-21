cmake_minimum_required(VERSION 3.31.6)
project(AsyncHttpProxy VERSION 1.0.0 LANGUAGES CXX)

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
find_package(GTest REQUIRED)
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/src/*.hpp")
file(GLOB SOURCES_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")


# Создаём статическую библиотеку
add_subdirectory(http_lib)

add_executable(${PROJECT_NAME}  ${SOURCES_FILES} ${HEADER_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE AsyncHttpLib ${Boost_LIBRARIES})
#
# Тесты
#

# Рекурсивно ищем все используемые в тестах .cpp файлы
file(GLOB TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(${PROJECT_NAME}_tests "${TEST_SRC_FILES}")

target_link_libraries(${PROJECT_NAME}_tests PRIVATE AsyncHttpLib GTest::GTest GTest::Main)

# Включаем тестирование
enable_testing()
add_test(NAME AsyncHttpProxy_Tests COMMAND ${PROJECT_NAME}_tests)
