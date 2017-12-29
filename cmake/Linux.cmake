message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")

option(DISABLE_GDAL "Disable GDAL" OFF)

set(CMAKE_CXX_FLAGS "-DUNIX -D_REENTRANT -DFMI_COMPRESSION -DBOOST -DBOOST_IOSTREAMS_NO_LIB")

if(DISABLE_GDAL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLED_GDAL")
endif()

add_compile_options(-std=c++11 -fPIC -MD -Wall -W -Wno-unused-parameter)
