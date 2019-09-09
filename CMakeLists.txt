cmake_minimum_required(VERSION 3.14)
project(ImageProcessor6)

set(CMAKE_INCLUDE_CURERENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)

include(FindPkgConfig)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Charts)
find_package(Qt5Pdfium)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale system)
#find_package(OpenCV REQUIRED)

#pkg_check_modules(PKGS REQUIRED)

#include_directories(${PKGS_INCLUDE_DIRS})
#link_directories(${PKGS_LIBRARY_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(ImageProcessor6 main.cpp application.h matrix.h array.h image.h event.h application.cpp stream.h math.h images.h tesseract.h lab.h utils.h translate.cpp translate.h config.h)

target_link_libraries(ImageProcessor6 pthread)
target_link_libraries(ImageProcessor6 Qt5::Core)
target_link_libraries(ImageProcessor6 Qt5::Widgets)
target_link_libraries(ImageProcessor6 Qt5::Charts)
target_link_libraries(ImageProcessor6 Qt5::Pdfium)
#target_link_libraries(ImageProcessor6 ${PKGS_LIBRARY_DIRS})
#target_link_libraries(ImageProcessor6 ${OpenCV_LIBS})
target_link_libraries(ImageProcessor6 ${Boost_LIBRARIES})
target_link_libraries(ImageProcessor6 ${OPENSSL_LIBRARIES})