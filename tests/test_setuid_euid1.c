#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <pwd.h>

static void test_uwrap_setuid(void **state)
{
	int rc;
	uid_t u;
	uid_t cp_ruid, cp_euid, cp_suid;

	(void) state; /* unused */

	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);

	u = geteuid();
	assert_int_equal(u, 0x0);

	rc = setresuid(0x4444, 0x5555, 0x6666);
	assert_int_equal(rc, 0);

	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x4444);
	assert_int_equal(cp_euid, 0x5555);
	assert_int_equal(cp_suid, 0x6666);

	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);

	rc = setuid(0x5555);
	if (rc == 0) {
		/* This is BSD */
		cp_ruid = cp_euid = cp_suid = -1;
		rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
		assert_return_code(rc, errno);
		assert_int_equal(cp_ruid, 0x5555);
		assert_int_equal(cp_euid, 0x5555);
		assert_int_equal(cp_suid, 0x5555);
	} else {
		/* This is Linux and uid_wrapper */
		assert_int_equal(rc, -1);
		assert_int_equal(errno, EPERM);

		rc = setuid(0x4444);
		assert_return_code(rc, errno);

		cp_ruid = cp_euid = cp_suid = -1;
		rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
		assert_return_code(rc, errno);
		assert_int_equal(cp_ruid, 0x4444);
		assert_int_equal(cp_euid, 0x4444);
		assert_int_equal(cp_suid, 0x6666);

		u = getuid();
		assert_int_equal(u, 0x4444);
		u = geteuid();
		assert_int_equal(u, 0x4444);
	}
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_setuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
