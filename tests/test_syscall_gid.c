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

static void test_uwrap_syscall_setgid(void **state)
{
	long int rc;
	gid_t g;

	(void) state; /* unused */

	rc = syscall(SYS_setgid, 1);
	assert_int_equal(rc, 0);

	g = getgid();
	assert_int_equal(g, 1);
#ifdef __alpha__
	assert_int_equal(g, syscall(SYS_getxgid));
#else
	assert_int_equal(g, syscall(SYS_getgid));
#endif
}

static void test_uwrap_syscall_setregid(void **state)
{
	long int rc;
	gid_t g;

	(void) state; /* unused */

	rc = syscall(SYS_setregid, 2, 0);
	assert_int_equal(rc, 0);

	g = getegid();
	assert_int_equal(g, 0);
#ifdef SYS_getegid /* not available on Solaris */
	assert_int_equal(g, syscall(SYS_getegid));
#endif

	rc = syscall(SYS_setregid, -1, 42);
	assert_int_equal(rc, 0);

	g = getgid();
	assert_int_equal(g, 2);
	g = getegid();
	assert_int_equal(g, 42);
}

#if defined(SYS_setresgid)
static void test_uwrap_syscall_setresgid(void **state)
{
	long int rc;
	gid_t g;
	gid_t g_r, g_e, g_s;

	(void) state; /* unused */

	rc = syscall(SYS_setresgid, 42, 0, -1);
	assert_int_equal(rc, 0);

	g = getegid();
	assert_int_equal(g, 0);

#ifdef SYS_getresgid /* not available on Solaris */
	rc = syscall(SYS_getresgid, &g_r, &g_e, &g_s);
	assert_return_code(rc, errno);

	assert_int_equal(g_r, 42);
	assert_int_equal(g, g_e);
#endif

	rc = syscall(SYS_setregid, -1, 42);
	assert_return_code(rc, errno);

	g = getegid();
	assert_int_equal(g, 42);
}
#endif

#if defined(SYS_setgroups)
static void test_uwrap_syscall_setgroups(void **state)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	gid_t rlist[16];
	int rc = -1;

	(void) state; /* unused */

	rc = syscall(SYS_setgroups, ARRAY_SIZE(glist), glist);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 5);

	assert_memory_equal(glist, rlist, sizeof(glist));

	/* Drop all supplementary groups. This is often done by daemons */
	memset(rlist, 0, sizeof(rlist));
	rc = syscall(SYS_setgroups, 0, NULL);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 0);

	assert_int_equal(rlist[0], 0);
}
#endif

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall_setgid),
		cmocka_unit_test(test_uwrap_syscall_setregid),
#if defined(SYS_setresgid)
		cmocka_unit_test(test_uwrap_syscall_setresgid),
#endif
#if defined(SYS_setgroups)
		cmocka_unit_test(test_uwrap_syscall_setgroups),
#endif
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
