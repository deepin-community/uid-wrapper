#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <pwd.h>

static void test_uwrap_setgid_root(void **state)
{
	int rc;
	uid_t u;
	gid_t g;
#ifdef HAVE_GETRESGID
	gid_t cp_rgid, cp_egid, cp_sgid;
#endif

	(void) state; /* unused */

	u = getuid();
	assert_int_equal(u, 0x0);
	u = geteuid();
	assert_int_equal(u, 0x0);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0);
#endif

	g = getgid();
	assert_int_equal(g, 0x0);

	g = getegid();
	assert_int_equal(g, 0x0);

	rc = setgid(-1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EINVAL);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0);
#endif

	rc = setgid(5555);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 5555);
	assert_int_equal(cp_egid, 5555);
	assert_int_equal(cp_sgid, 5555);
#endif

	g = getgid();
	assert_int_equal(g, 5555);

	g = getegid();
	assert_int_equal(g, 5555);

	rc = setgid(0);
	assert_return_code(rc, errno);
}

static void test_uwrap_setgid_user(void **state)
{
	int rc;
	uid_t u;
#ifdef HAVE_GETRESGID
	gid_t cp_rgid, cp_egid, cp_sgid;
#endif

	(void) state; /* unused */

	u = getuid();
	assert_int_equal(u, 0x0);
	u = geteuid();
	assert_int_equal(u, 0x0);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0);
#endif

	rc = setuid(0x5555);
	assert_return_code(rc, errno);

	u = getuid();
	assert_int_equal(u, 0x5555);
	u = geteuid();
	assert_int_equal(u, 0x5555);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0);
#endif

	rc = setgid(5555);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0);
#endif

	rc = setgid(0);
	assert_return_code(rc, errno);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_setgid_root),
		cmocka_unit_test(test_uwrap_setgid_user),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
