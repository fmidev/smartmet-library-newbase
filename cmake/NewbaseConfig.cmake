get_filename_component(INSTALLED_PREFIX ${Newbase_DIR} DIRECTORY)
get_filename_component(INSTALLED_PREFIX ${INSTALLED_PREFIX} DIRECTORY)
get_filename_component(INSTALLED_PREFIX ${INSTALLED_PREFIX} DIRECTORY)
set(Newbase_INCLUDE_DIRS ${INSTALLED_PREFIX}/include)
set(Newbase_INSTALLED_INCLUDE_DIRS ${INSTALLED_PREFIX}/include/newbase)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(Newbase_INSTALLED_LIBRARIES ${INSTALLED_PREFIX}/lib/libNewbase.a)
    set(Newbase_LIBRARIES  ${INSTALLED_PREFIX}/lib/libNewbase.a)
else()
    set(Newbase_INSTALLED_LIBRARIES ${INSTALLED_PREFIX}/lib/NewBase.lib)
    set(Newbase_LIBRARIES  ${INSTALLED_PREFIX}/lib/Newbase.lib)
endif()
