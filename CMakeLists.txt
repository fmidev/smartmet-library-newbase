cmake_minimum_required(VERSION 3.14.4)
project(Newbase)

if(WIN32)
    set(CMAKE_MFC_FLAG 0)
endif()

file( GLOB SOURCE_FILES newbase/* )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/newbase
    PUBLIC ${BOOST_INCLUDE_DIR} 
    PUBLIC ${FMT_INCLUDE_DIR}
    PUBLIC ${GDAL_INCLUDE_DIR} 
    PUBLIC ${Gis_INCLUDE_DIRS}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/newbase
    ${PROJECT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR} 
    ${FMT_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR} 
    ${Gis_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )


