CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
project(edwards CXX)

find_package(asio)
find_package(Boost COMPONENTS system thread)
find_package(fmt)
find_package(units)

add_library(libedwards
            src/internal/dialog.cpp
            src/error.cpp
            src/multidrop_network.cpp)

target_compile_features(libedwards PRIVATE cxx_std_17)

# libedwards uses the coroutines TS internally
target_compile_options(libedwards
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/await>
        PRIVATE $<$<CXX_COMPILER_ID:Clang>:-fcoroutines-ts>)

target_include_directories(libedwards PUBLIC include)
target_link_libraries(libedwards
        PRIVATE fmt::fmt
        PUBLIC Boost::boost
        PUBLIC Boost::system
        PUBLIC Boost::thread)