get_filename_component(INSTALLED_PREFIX ${Newbase_DIR} DIRECTORY)
get_filename_component(INSTALLED_PREFIX ${INSTALLED_PREFIX} DIRECTORY)
get_filename_component(INSTALLED_PREFIX ${INSTALLED_PREFIX} DIRECTORY)
set(Newbase_INCLUDE_DIRS ${INSTALLED_PREFIX}/include)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # If static and shared library is available, one can select
    # which one to use. By default use shared one.
    if(NOT ${PROJECT_NAME} MATCHES "Newbase")
        option(Newbase_USE_STATIC_LIBS "Newbase use static libs" OFF)
    endif()

    if(Newbase_USE_STATIC_LIBS)
        set(Newbase_LIBRARIES  ${INSTALLED_PREFIX}/lib/libNewbase.a)
    else()
        set(Newbase_LIBRARIES  ${INSTALLED_PREFIX}/lib/libNewbase.so)
    endif()

    if(NOT EXISTS ${Newbase_LIBRARIES})
        message(STATUS "Library ${Newbase_LIBRARIES} not found.")
    endif()
else()
    set(Newbase_LIBRARIES  ${INSTALLED_PREFIX}/lib/Newbase.lib)
endif()
