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

static void test_uwrap_setresuid(void **state)
{
	int rc;
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

	rc = setresuid(-1, -1, -1);
	assert_return_code(rc, errno);

	rc = setresuid(0x4444, 0x5555, -1);
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
	rc = setresuid(0, -1, -1);
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

	rc = setresuid(-1, 0, -1);
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

	rc = setresuid(0, 0x5555, 0x6666);
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
	rc = setresuid(0x5555, 0x6666, 0x4444);
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
	rc = setresuid(0x5555, 0x6666, 0);
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
	rc = setresuid(0x5555, 0x6666, 0x4444);
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
	rc = setresuid(0x5555, 0, 0x6666);
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
	rc = setresuid(1111, 2222, 3333);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 1111);
	assert_int_equal(cp_euid, 2222);
	assert_int_equal(cp_suid, 3333);
#endif

	rc = setresuid(0, -1, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 1111);
	assert_int_equal(cp_euid, 2222);
	assert_int_equal(cp_suid, 3333);
#endif

	rc = setresuid(-1, 0, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 1111);
	assert_int_equal(cp_euid, 2222);
	assert_int_equal(cp_suid, 3333);
#endif

	rc = setresuid(-1, -1, 0);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 1111);
	assert_int_equal(cp_euid, 2222);
	assert_int_equal(cp_suid, 3333);
#endif

	/*
	 * We can still roate the values
	 */
	rc = setresuid(2222, 3333, 1111);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 2222);
	assert_int_equal(cp_euid, 3333);
	assert_int_equal(cp_suid, 1111);
#endif

	/*
	 * We can still roate the values
	 */
	rc = setresuid(3333, 1111, 2222);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 3333);
	assert_int_equal(cp_euid, 1111);
	assert_int_equal(cp_suid, 2222);
#endif
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_setresuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
