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

static void test_uwrap_syscall_setreuid(void **state)
{
	long int rc;
	uid_t u;
#ifdef SYS_getresuid
	uid_t cp_ruid, cp_euid, cp_suid;
#endif

	(void) state; /* unused */

	u = getuid();
	assert_int_equal(u, 0);
	u = geteuid();
	assert_int_equal(u, 0);

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif

	rc = syscall(SYS_setreuid, -1, -1);
	assert_return_code(rc, errno);

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif

	rc = syscall(SYS_setreuid, 0x4444, 0x5555);
	assert_return_code(rc, errno);

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x5555);
#endif

	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);
#ifdef SYS_geteuid /* not available on Solaris */
	assert_int_equal(u, syscall(SYS_geteuid));
#endif

	rc = syscall(SYS_setreuid, -1, 0x6666);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef SYS_getresuid
	cp_ruid = cp_euid = cp_suid = -1;
	rc = syscall(SYS_getresuid, &cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x5555);
#endif
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setreuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
