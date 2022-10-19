#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <pwd.h>

static void test_uwrap_setresgid(void **state)
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

	rc = setresgid(-1, -1, -1);
	assert_return_code(rc, errno);

	rc = setresgid(0x4444, 0x5555, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x4444);
	assert_int_equal(cp_egid, 0x5555);
	assert_int_equal(cp_sgid, 0);
#endif

	g = getgid();
	assert_int_equal(g, 0x4444);
	g = getegid();
	assert_int_equal(g, 0x5555);

	/* We can go back cause the sgid is 0 */
	rc = setresgid(0, -1, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0x5555);
	assert_int_equal(cp_sgid, 0);
#endif

	u = getgid();
	assert_int_equal(u, 0x0);
	g = getegid();
	assert_int_equal(g, 0x5555);

	rc = setresgid(-1, 0, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x0);
	assert_int_equal(cp_sgid, 0x0);
#endif

	u = getegid();
	assert_int_equal(u, 0);

	rc = setresgid(0, 0x5555, 0x6666);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0);
	assert_int_equal(cp_egid, 0x5555);
	assert_int_equal(cp_sgid, 0x6666);
#endif

	/*
	 * The egid needs to be 0 in order to change to an
	 * unknown value (here 0x4444)
	 */
	rc = setresgid(0x5555, 0x6666, 0x4444);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0x6666);
	assert_int_equal(cp_sgid, 0x4444);
#endif

	errno = 0;
	rc = setresgid(0x5555, 0x6666, 0);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0x6666);
	assert_int_equal(cp_sgid, 0);
#endif

	rc = setresgid(0x5555, 0x6666, 0x4444);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0x6666);
	assert_int_equal(cp_sgid, 0x4444);
#endif

	rc = setresgid(0x5555, 0, 0x6666);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0);
	assert_int_equal(cp_sgid, 0x6666);
#endif

	rc = setresgid(0x1111, 0x2222, 0x3333);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x1111);
	assert_int_equal(cp_egid, 0x2222);
	assert_int_equal(cp_sgid, 0x3333);
#endif

	rc = setresgid(0, -1, -1);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESGID
	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x2222);
	assert_int_equal(cp_sgid, 0x3333);
#endif
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_setresgid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
