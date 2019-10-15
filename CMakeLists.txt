if(WIN32)
    cmake_minimum_required(VERSION 3.14.4)
    project(Newbase)

    set(CMAKE_MFC_FLAG 0)

    file( GLOB SOURCE_FILES newbase/* )

    add_library( 
        ${PROJECT_NAME} STATIC
        ${SOURCE_FILES} 
        )

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
else()
  cmake_minimum_required(VERSION 3.7.1)
  project(Newbase)

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

# uninstall target
  if(NOT TARGET uninstall)
      configure_file(
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
          "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
          IMMEDIATE @ONLY
      )

      add_custom_target(
          uninstall
          COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
      )
  endif()

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(
      ${PROJECT_NAME}
      ${Fmt_LIBRARIES}
      ${GDAL_LIBRARY}
      ${Boost_LIBRARIES}
    )
    include(cmake/package.cmake OPTIONAL)
  endif()

  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if (Fmt_FOUND)
      target_link_libraries(${PROJECT_NAME} fmt::fmt-header-only)
    endif()
  endif()

endif()


