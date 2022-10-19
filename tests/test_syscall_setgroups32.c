#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <grp.h>

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

static void test_uwrap_syscall_setgroups32(void **state)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	gid_t rlist[16];
	int rc = -1;

	(void) state; /* unused */

	rc = syscall(SYS_setgroups32, ARRAY_SIZE(glist), glist);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 5);

	assert_memory_equal(glist, rlist, sizeof(glist));

	/* Drop all supplementary groups. This is often done by daemons */
	memset(rlist, 0, sizeof(rlist));
	rc = syscall(SYS_setgroups32, 0, NULL);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 0);

	assert_int_equal(rlist[0], 0);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setgroups32),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
