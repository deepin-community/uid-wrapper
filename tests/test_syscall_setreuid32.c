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

static void test_uwrap_syscall_setreuid32(void **state)
{
	long int rc;
	uid_t u;
#ifdef HAVE_GETRESUID
	uid_t cp_ruid, cp_euid, cp_suid;
#endif

	(void) state; /* unused */

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif
	u = geteuid();
	assert_int_equal(u, 0x0);

	rc = syscall(SYS_setreuid32, -1, -1);
	assert_int_equal(rc, 0);

	rc = setreuid(0x4444, 0x5555);
	assert_int_equal(rc, 0);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x5555);
#endif

	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);

	rc = syscall(SYS_setreuid32, 0, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x5555);
#endif
	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);

	rc = syscall(SYS_setreuid32, -1, 0);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x5555);
#endif
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setreuid32),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}

