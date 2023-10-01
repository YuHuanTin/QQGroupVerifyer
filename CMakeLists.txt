cmake_minimum_required(VERSION 3.26)
project(QQGroupVerifyer)

set(CMAKE_CXX_STANDARD 23)


find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED locale)
find_package(cpr CONFIG REQUIRED)

link_libraries(cpr::cpr)
link_libraries(Boost::boost Boost::locale)
link_libraries(nlohmann_json::nlohmann_json)
link_libraries(fmt::fmt)




add_executable(QQGroupVerifyer
        main.cpp
        ExceptionHandler/ExceptionHandler.cpp
        ExceptionHandler/ExceptionHandler.h
        Misc/Misc.cpp
        Misc/Misc.h)
