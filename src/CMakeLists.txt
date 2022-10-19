project(libuid_wrapper C)

add_library(uid_wrapper SHARED uid_wrapper.c)

target_compile_options(uid_wrapper
                       PRIVATE
                           ${DEFAULT_C_COMPILE_FLAGS}
                           -D_GNU_SOURCE)

target_include_directories(uid_wrapper
                           PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(uid_wrapper ${UIDWRAP_REQUIRED_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(
  uid_wrapper
    PROPERTIES
      VERSION
        ${LIBRARY_VERSION}
      SOVERSION
        ${LIBRARY_SOVERSION}
)

install(TARGETS uid_wrapper
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(UID_WRAPPER_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}uid_wrapper${CMAKE_SHARED_LIBRARY_SUFFIX}" PARENT_SCOPE)
