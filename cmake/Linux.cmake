message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
set(CMAKE_CXX_FLAGS "-DUNIX -D_REENTRANT -DFMI_COMPRESSION -DBOOST -DBOOST_IOSTREAMS_NO_LIB")
add_compile_options(-std=c++11 -fPIC -MD -Wall -W -Wno-unused-parameter)
