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

static void test_uwrap_syscall_setresuid32(void **state)
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
	assert_int_equal(u, 0);

	rc = syscall(SYS_setresuid32, -1, -1, -1);
	assert_return_code(rc, errno);

	rc = syscall(SYS_setresuid32, 0x4444, 0x5555, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0);
#endif

	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);

	/* We can go back cause the suid is 0 */
	rc = syscall(SYS_setresuid32, 0, -1, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0);
#endif

	u = getuid();
	assert_int_equal(u, 0);

	rc = syscall(SYS_setresuid32, -1, 0, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);
#endif

	u = geteuid();
	assert_int_equal(u, 0);

	rc = syscall(SYS_setresuid32, 0, 0x5555, 0x6666);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x6666);
#endif

	/*
	 * The euid needs to be 0 in order to change to an
	 * unknown value (here 0x4444)
	 */
	rc = syscall(SYS_setresuid32, 0x5555, 0x6666, 0x4444);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x6666);
#endif

	/* But we can rotate the values */
	errno = 0;
	rc = syscall(SYS_setresuid32, 0x5555, 0x6666, 0);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x5555);
	assert_int_equal(cp_euid, 0x6666);
	assert_int_equal(cp_suid, 0);
#endif

	/*
	 * The euid needs to be 0 in order to change to an
	 * unknown value (here 0x4444)
	 */
	rc = syscall(SYS_setresuid32, 0x5555, 0x6666, 0x4444);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x5555);
	assert_int_equal(cp_euid, 0x6666);
	assert_int_equal(cp_suid, 0);
#endif

	/* But we can rotate the values */
	rc = syscall(SYS_setresuid32, 0x5555, 0, 0x6666);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x5555);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0x6666);
#endif

	/*
	 * With euid == 0 we can change to completely unrelated values.
	 */
	rc = syscall(SYS_setresuid32, 0x1111, 0x2222, 0x3333);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x1111);
	assert_int_equal(cp_euid, 0x2222);
	assert_int_equal(cp_suid, 0x3333);
#endif

	rc = syscall(SYS_setresuid32, 0, -1, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x1111);
	assert_int_equal(cp_euid, 0x2222);
	assert_int_equal(cp_suid, 0x3333);
#endif

	rc = syscall(SYS_setresuid32, -1, 0, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x1111);
	assert_int_equal(cp_euid, 0x2222);
	assert_int_equal(cp_suid, 0x3333);
#endif

	rc = syscall(SYS_setresuid32, -1, -1, 0);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x1111);
	assert_int_equal(cp_euid, 0x2222);
	assert_int_equal(cp_suid, 0x3333);
#endif

	/*
	 * We can still roate the values
	 */
	rc = syscall(SYS_setresuid32, 0x2222, 0x3333, 0x1111);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x2222);
	assert_int_equal(cp_euid, 0x3333);
	assert_int_equal(cp_suid, 0x1111);
#endif

	/*
	 * We can still roate the values
	 */
	rc = syscall(SYS_setresuid32, 0x3333, 0x1111, 0x2222);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x3333);
	assert_int_equal(cp_euid, 0x1111);
	assert_int_equal(cp_suid, 0x2222);
#endif
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setresuid32),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
