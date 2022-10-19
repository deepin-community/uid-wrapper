#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <pwd.h>

static void test_uwrap_seteuid(void **state)
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
	assert_int_equal(u, 0x0);


#ifndef BSD
	/* BSD sets 0xFFFFFF as the UID number in this case */
	rc = seteuid(-1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EINVAL);
#endif

	u = geteuid();
	assert_int_equal(u, 0x0);

	rc = seteuid(0x4444);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x4444);
	assert_int_equal(cp_suid, 0);
#endif

	u = geteuid();
	assert_int_equal(u, 0x4444);

	/*
	 * The euid needs to be 0 in order to change to an
	 * unknown value (here 0x5555).
	 */
	rc = seteuid(0x5555);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x4444);
	assert_int_equal(cp_suid, 0);
#endif

	u = geteuid();
	assert_int_equal(u, 0x4444);

	rc = seteuid(0);
	assert_int_equal(rc, 0);

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

	rc = seteuid(0x5555);
	assert_return_code(rc, errno);

#ifdef HAVE_GETRESUID
	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0);
#endif

	u = geteuid();
	assert_int_equal(u, 0x5555);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_seteuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
