include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckPrototypeDefinition)
include(TestBigEndian)

set(PACKAGE ${PROJECT_NAME})
set(VERSION ${PROJECT_VERSION})

set(BINARYDIR ${uid_wrapper_BINARY_DIR})
set(SOURCEDIR ${uid_wrapper_SOURCE_DIR})

function(COMPILER_DUMPVERSION _OUTPUT_VERSION)
    # Remove whitespaces from the argument.
    # This is needed for CC="ccache gcc" cmake ..
    string(REPLACE " " "" _C_COMPILER_ARG "${CMAKE_C_COMPILER_ARG1}")

    execute_process(
        COMMAND
            ${CMAKE_C_COMPILER} ${_C_COMPILER_ARG} -dumpversion
        OUTPUT_VARIABLE _COMPILER_VERSION
    )

    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
           _COMPILER_VERSION "${_COMPILER_VERSION}")

    set(${_OUTPUT_VERSION} ${_COMPILER_VERSION} PARENT_SCOPE)
endfunction()

if(CMAKE_COMPILER_IS_GNUCC AND NOT MINGW AND NOT OS2)
    compiler_dumpversion(GNUCC_VERSION)
    if (NOT GNUCC_VERSION EQUAL 34)
        set(CMAKE_REQUIRED_FLAGS "-fvisibility=hidden")
        check_c_source_compiles(
"void __attribute__((visibility(\"default\"))) test() {}
int main(void){ return 0; }
" WITH_VISIBILITY_HIDDEN)
        set(CMAKE_REQUIRED_FLAGS "")
    endif (NOT GNUCC_VERSION EQUAL 34)
endif(CMAKE_COMPILER_IS_GNUCC AND NOT MINGW AND NOT OS2)

# HEADERS
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/syscall.h HAVE_SYS_SYSCALL_H)
check_include_file(syscall.h HAVE_SYSCALL_H)
check_include_file(grp.h HAVE_GRP_H)
check_include_file(unistd.h HAVE_UNISTD_H)

# FUNCTIONS
check_function_exists(strncpy HAVE_STRNCPY)
check_function_exists(vsnprintf HAVE_VSNPRINTF)
check_function_exists(snprintf HAVE_SNPRINTF)

check_function_exists(seteuid HAVE_SETEUID)
check_function_exists(setreuid HAVE_SETREUID)
check_function_exists(setresuid HAVE_SETRESUID)
check_function_exists(getresuid HAVE_GETRESUID)
check_function_exists(getresgid HAVE_GETRESGID)

check_function_exists(setegid HAVE_SETEGID)
check_function_exists(setregid HAVE_SETREGID)
check_function_exists(setresgid HAVE_SETRESGID)

check_function_exists(getgroups HAVE_GETGROUPS)
check_function_exists(setgroups HAVE_SETGROUPS)

if (HAVE_SETGROUPS)
    check_prototype_definition(setgroups
        "int setgroups(int size, const gid_t *list)"
        "-1"
        "unistd.h"
        HAVE_SETGROUPS_INT)
endif (HAVE_SETGROUPS)

check_function_exists(syscall HAVE_SYSCALL)

if (HAVE_SYSCALL)
    set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

    check_prototype_definition(syscall
        "int syscall(int sysno, ...)"
        "-1"
        "unistd.h;sys/syscall.h"
        HAVE_SYSCALL_INT)
    set(CMAKE_REQUIRED_DEFINITIONS)
endif (HAVE_SYSCALL)

# OPTIONS

if (LINUX)
    if (HAVE_SYS_SYSCALL_H)
       list(APPEND CMAKE_REQUIRED_DEFINITIONS "-DHAVE_SYS_SYSCALL_H")
    endif (HAVE_SYS_SYSCALL_H)
    if (HAVE_SYSCALL_H)
        list(APPEND CMAKE_REQUIRED_DEFINITIONS "-DHAVE_SYSCALL_H")
    endif (HAVE_SYSCALL_H)

check_c_source_compiles("
#include <sys/types.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif
#include <unistd.h>

int main(void) {
    syscall(SYS_setresuid32, -1, -1, -1);
    syscall(SYS_setresgid32, -1, -1, -1);
    syscall(SYS_setreuid32, -1, -1);
    syscall(SYS_setregid32, -1, -1);
    syscall(SYS_setuid32, -1);
    syscall(SYS_setgid32, -1);
    syscall(SYS_setgroups32, 0, NULL);

    return 0;
}" HAVE_LINUX_32BIT_SYSCALLS)

    set(CMAKE_REQUIRED_DEFINITIONS)
endif (LINUX)

check_c_source_compiles("
#include <stdbool.h>
int main(void) {
    bool x;
    bool *p_x = &x;
    __atomic_load(p_x, &x, __ATOMIC_RELAXED);
    return 0;
}" HAVE_GCC_ATOMIC_BUILTINS)

check_c_source_compiles("
__thread int tls;

int main(void) {
    return 0;
}" HAVE_GCC_THREAD_LOCAL_STORAGE)

check_c_source_compiles("
void test_constructor_attribute(void) __attribute__ ((constructor));

void test_constructor_attribute(void)
{
     return;
}

int main(void) {
     return 0;
}" HAVE_CONSTRUCTOR_ATTRIBUTE)

check_c_source_compiles("
void test_destructor_attribute(void) __attribute__ ((destructor));

void test_destructor_attribute(void)
{
    return;
}

int main(void) {
    return 0;
}" HAVE_DESTRUCTOR_ATTRIBUTE)

# If this produces a warning treat it as error!
set(CMAKE_REQUIRED_FLAGS "-Werror")
check_c_source_compiles("
void test_address_sanitizer_attribute(void) __attribute__((no_sanitize_address));

void test_address_sanitizer_attribute(void)
{
    return;
}

int main(void) {
    return 0;
}" HAVE_ADDRESS_SANITIZER_ATTRIBUTE)
set(CMAKE_REQUIRED_FLAGS)

check_c_source_compiles("
void log_fn(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int main(void) {
    return 0;
}" HAVE_FUNCTION_ATTRIBUTE_FORMAT)

check_c_source_compiles("
#define FALL_THROUGH __attribute__((fallthrough))

enum direction_e {
    UP = 0,
    DOWN,
};

int main(void) {
    enum direction_e key = UP;
    int i = 10;
    int j = 0;

    switch (key) {
    case UP:
        i = 5;
        FALL_THROUGH;
    case DOWN:
        j = i * 2;
        break;
    default:
        break;
    }

    return 0;
}" HAVE_FALLTHROUGH_ATTRIBUTE)

# SYSTEM LIBRARIES

find_library(DLFCN_LIBRARY dl)
if (DLFCN_LIBRARY)
    list(APPEND _REQUIRED_LIBRARIES ${DLFCN_LIBRARY})
else()
    check_function_exists(dlopen HAVE_DLOPEN)
    if (NOT HAVE_DLOPEN)
        message(FATAL_ERROR "FATAL: No dlopen() function detected")
    endif()
endif()

if (OSX)
    set(HAVE_APPLE 1)
endif (OSX)

# ENDIAN
if (NOT WIN32)
    test_big_endian(WORDS_BIGENDIAN)
endif (NOT WIN32)

set(UIDWRAP_REQUIRED_LIBRARIES ${_REQUIRED_LIBRARIES} CACHE INTERNAL "uidwrap required system libraries")
