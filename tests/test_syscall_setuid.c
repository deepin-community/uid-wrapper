#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <pwd.h>

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif

static void test_uwrap_syscall_setuid(void **state)
{
	long int rc;
	uid_t u;
#if defined(HAVE_GETRESUID) || defined(SYS_getresuid)
	uid_t cp_ruid, cp_euid, cp_suid;
#endif

	(void) state; /* unused */

	u = getuid();
	assert_int_equal(u, 0);
	u = geteuid();
	assert_int_equal(u, 0);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif

	rc = syscall(SYS_setuid, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EINVAL);

	rc = syscall(SYS_setuid, 5555);
	assert_return_code(rc, errno);

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 5555);
	assert_int_equal(cp_euid, 5555);
	assert_int_equal(cp_suid, 5555);
#endif

	u = getuid();
	assert_int_equal(u, 5555);
#ifdef __alpha
	assert_int_equal(u, syscall(SYS_getxuid));
#else
	assert_int_equal(u, syscall(SYS_getuid));
#endif

	rc = syscall(SYS_setuid, 0);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
