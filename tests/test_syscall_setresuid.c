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

#if defined(SYS_setresuid)
static void test_uwrap_syscall_setresuid(void **state)
{
	long int rc;
	uid_t u;
	uid_t u_r, u_e, u_s;

	(void) state; /* unused */

	rc = syscall(SYS_setresuid, 0x4444, 0x5555, 0x6666);
	assert_return_code(rc, errno);

	u = getuid();
	assert_int_equal(u, 0x4444);

#ifdef SYS_getresuid /* not available on Solaris */
	rc = syscall(SYS_getresuid, &u_r, &u_e, &u_s);
	assert_return_code(rc, errno);

	assert_int_equal(u_r, u);
	assert_int_equal(u_e, 0x5555);
	assert_int_equal(u_s, 0x6666);
#endif

	rc = syscall(SYS_setresuid, -1, 42, -1);
	assert_int_equal(rc, -1);
}
#endif

int main(void) {
	int rc = 0;

#if defined(SYS_setresuid) && defined(SYS_getresuid)
	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setresuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);
#endif

	return rc;
}
