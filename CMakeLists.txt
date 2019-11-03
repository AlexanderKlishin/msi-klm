cmake_minimum_required(VERSION 3.10)
project(msi-klm)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

add_executable(msi-klm main.c)
target_link_libraries(msi-klm hidapi-libusb)

set(CPACK_PACKAGE_NAME msi-klm)
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT "alexander.klishin@gmail.com")

include(CPack)
