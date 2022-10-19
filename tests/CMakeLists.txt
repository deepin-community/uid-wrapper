project(tests C)

function(ADD_CMOCKA_TEST_ENVIRONMENT _TEST_NAME)
    if (CMAKE_BUILD_TYPE)
        string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
        if (CMAKE_BUILD_TYPE_LOWER STREQUAL "addresssanitizer")
            find_library(ASAN_LIBRARY
                         NAMES asan)
            if (NOT ASAN_LIBRARY)
                foreach(version RANGE 10 1)
                    if (NOT ASAN_LIBRARY)
                        find_library(ASAN_LIBRARY libasan.so.${version})
                    endif()
                endforeach()
            endif()
        endif()
    endif()

    if (ASAN_LIBRARY)
        list(APPEND PRELOAD_LIBRARIES ${ASAN_LIBRARY})
    endif()
    list(APPEND PRELOAD_LIBRARIES ${UID_WRAPPER_LOCATION})

    if (OSX)
        set(TORTURE_ENVIRONMENT "DYLD_FORCE_FLAT_NAMESPACE=1;DYLD_INSERT_LIBRARIES=${UID_WRAPPER_LOCATION}")
    else ()
        string(REPLACE ";" ":" _TMP_ENV "${PRELOAD_LIBRARIES}")
        set(TORTURE_ENVIRONMENT "LD_PRELOAD=${_TMP_ENV}")
    endif()

    list(APPEND TORTURE_ENVIRONMENT UID_WRAPPER=1)

    foreach(_arg ${ARGN})
        list(APPEND TORTURE_ENVIRONMENT ${_arg})
    endforeach()

    set_property(TEST
                    ${_TEST_NAME}
                PROPERTY
                    ENVIRONMENT "${TORTURE_ENVIRONMENT}")
endfunction()

set(TESTSUITE_LIBRARIES ${UWRAP_REQUIRED_LIBRARIES} ${CMOCKA_LIBRARY})

if (BSD)
    add_definitions(-DBSD)
endif (BSD)

add_executable(mock_exec_uid mock_exec_uid.c)
target_compile_options(mock_exec_uid
                       PRIVATE
                           ${DEFAULT_C_COMPILE_FLAGS}
                           -D_GNU_SOURCE)
target_include_directories(mock_exec_uid
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})

set(UWRAP_UID_TESTS
     test_setuid
     test_seteuid)

if (HAVE_SETREUID)
    list(APPEND UWRAP_UID_TESTS test_setreuid)
endif (HAVE_SETREUID)

if (HAVE_SETRESUID)
    list(APPEND UWRAP_UID_TESTS test_setresuid test_setuid_euid1 test_setuid_euid2)
    if (HAVE_GETRESUID)
        list(APPEND UWRAP_UID_TESTS test_getresuid test_fork_exec)
    endif (HAVE_GETRESUID)
endif (HAVE_SETRESUID)

set(UWRAP_SYSCALL_UID_TESTS
    test_syscall_setuid
    test_syscall_setreuid
    test_syscall_setresuid)

set(UWRAP_GID_TESTS
    test_setgid
    test_setegid)

if (HAVE_SETREGID)
    list(APPEND UWRAP_GID_TESTS test_setregid)
endif (HAVE_SETREGID)

if (HAVE_SETRESGID)
    list(APPEND UWRAP_GID_TESTS test_setresgid)

    if (HAVE_GETRESGID)
        list(APPEND UWRAP_GID_TESTS test_setresgid_unprivileged)
    endif (HAVE_GETRESGID)
endif (HAVE_SETRESGID)

set(UWRAP_TESTS
    ${UWRAP_UID_TESTS}
    ${UWRAP_GID_TESTS}
    test_setgroups
    test_syscall
    ${UWRAP_SYSCALL_UID_TESTS}
    test_syscall_gid)

if (HAVE_LINUX_32BIT_SYSCALLS)
    set(UWRAP_TESTS
        ${UWRAP_TESTS}
        test_syscall_setuid32
        test_syscall_setreuid32
        test_syscall_setresuid32
        test_syscall_setgid32
        test_syscall_setregid32
        test_syscall_setresgid32
        test_syscall_setgroups32)
endif (HAVE_LINUX_32BIT_SYSCALLS)

foreach(_UWRAP_TEST ${UWRAP_TESTS})
    add_cmocka_test(${_UWRAP_TEST}
                    SOURCES ${_UWRAP_TEST}.c
                    COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                    LINK_LIBRARIES ${TESTSUITE_LIBRARIES}
                    LINK_OPTIONS ${DEFAULT_LINK_FLAGS})

    target_include_directories(${_UWRAP_TEST}
                               PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
    add_cmocka_test_environment(${_UWRAP_TEST} UID_WRAPPER_ROOT=1)
endforeach()

# helper library implementing uid_wrapper_enabled()
add_library(uwrap_enabled SHARED uwrap_enabled.c)
target_compile_options(uwrap_enabled
                       PRIVATE
                           ${DEFAULT_C_COMPILE_FLAGS})

add_cmocka_test(test_uwrap_enabled
                SOURCES test_uwrap_enabled.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                LINK_LIBRARIES ${TESTSUITE_LIBRARIES} uwrap_enabled
                LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
target_include_directories(test_uwrap_enabled
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
add_cmocka_test_environment(test_uwrap_enabled)

if (NOT CMAKE_BUILD_TYPE_LOWER MATCHES "sanitizer" AND
    NOT CMAKE_CROSSCOMPILING)
    add_cmocka_test(test_uwrap_disabled
                    SOURCES test_uwrap_disabled.c
                    COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                    LINK_LIBRARIES ${TESTSUITE_LIBRARIES}
                    LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
    target_include_directories(test_uwrap_disabled
                               PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
    set_property(
        TEST
            test_uwrap_disabled
        PROPERTY
            ENVIRONMENT LD_PRELOAD=${UID_WRAPPER_LOCATION})
endif ()

add_cmocka_test(test_thread_setuid
                SOURCES test_thread_setuid.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                LINK_LIBRARIES ${TESTSUITE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
                LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
target_include_directories(test_thread_setuid
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
add_cmocka_test_environment(test_thread_setuid UID_WRAPPER_ROOT=1 CMOCKA_TEST_ABORT=1)

add_cmocka_test(test_thread_setreuid
                SOURCES test_thread_setreuid.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                LINK_LIBRARIES ${TESTSUITE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
                LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
target_include_directories(test_thread_setreuid
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
add_cmocka_test_environment(test_thread_setreuid UID_WRAPPER_ROOT=1 CMOCKA_TEST_ABORT=1)

add_cmocka_test(test_thread_sync_setreuid
                SOURCES test_thread_sync_setreuid.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                LINK_LIBRARIES ${TESTSUITE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
                LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
target_include_directories(test_thread_sync_setreuid
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
add_cmocka_test_environment(test_thread_sync_setreuid UID_WRAPPER_ROOT=1 CMOCKA_TEST_ABORT=1)

add_cmocka_test(test_glibc_thread_support
                SOURCES test_glibc_thread_support.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} -D_GNU_SOURCE
                LINK_LIBRARIES ${TESTSUITE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
                LINK_OPTIONS ${DEFAULT_LINK_FLAGS})
target_include_directories(test_glibc_thread_support
                           PRIVATE ${CMAKE_BINARY_DIR} ${CMOCKA_INCLUDE_DIR})
add_cmocka_test_environment(test_glibc_thread_support UID_WRAPPER_ROOT=1 CMOCKA_TEST_ABORT=1)

if (LINUX)
    add_executable(getuids getuids.c)
    target_compile_options(getuids
                           PRIVATE
                               ${DEFAULT_C_COMPILE_FLAGS}
                               -D_GNU_SOURCE)
endif (LINUX)
