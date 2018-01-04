cmake_minimum_required(VERSION 3.7.1)
project(Newbase)

if(WIN32)
    set(CMAKE_MFC_FLAG 0)
endif()

include(cmake/${CMAKE_SYSTEM_NAME}.cmake OPTIONAL)

file( GLOB SOURCE_FILES newbase/* )

add_library(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
    )

if(NOT Boost_INCLUDE_DIRS)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    find_package(Boost 1.60.0 QUIET COMPONENTS filesystem regex thread)
    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
    endif()
endif()

find_package(Newbase QUIET)
if(Newbase_FOUND)
    include(${Newbase_CONFIG})
    file(GLOB INSTALLED_FILES
        ${Newbase_DIR}
	${Newbase_INSTALLED_INCLUDE_DIRS}
	${Newbase_INSTALLED_LIBRARIES})
    add_custom_target(uninstall rm -rf ${INSTALLED_FILES})
    message(STATUS "Already installed Newbase files: ${INSTALLED_FILES}")
endif()

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/newbase
    PUBLIC ${BOOST_INCLUDE_DIR} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/newbase
    ${PROJECT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR} 
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

install(DIRECTORY newbase/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/newbase
        FILES_MATCHING PATTERN "*.h")
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY cmake/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/newbase
        FILES_MATCHING PATTERN "*onfig.cmake")
