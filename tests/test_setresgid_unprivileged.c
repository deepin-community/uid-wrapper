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

static void test_uwrap_setresgid_unprivileged_uid(void **state)
{
	int rc;
	gid_t cp_rgid, cp_egid, cp_sgid;
	gid_t cp_ruid, cp_euid, cp_suid;

	(void) state; /* unused */

	rc = setresuid(0x0, 0x9999, 0x9999);
	assert_return_code(rc, errno);

	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x0);
	assert_int_equal(cp_euid, 0x9999);
	assert_int_equal(cp_suid, 0x9999);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x0);
	assert_int_equal(cp_sgid, 0x0);


	rc = setresgid(-1, -1, -1);
	assert_return_code(rc, errno);

	rc = setresgid(0x4444, -1, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x0);
	assert_int_equal(cp_sgid, 0x0);

	rc = setresgid(-1, 0x5555, -1);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x0);
	assert_int_equal(cp_sgid, 0x0);

	rc = setresgid(-1, -1, 0x6666);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x0);
	assert_int_equal(cp_egid, 0x0);
	assert_int_equal(cp_sgid, 0x0);

	setresuid(0x0, 0x0, 0x0);
}

static void test_uwrap_setresgid_unprivileged_uid_and_gid(void **state)
{
	int rc;
	gid_t cp_rgid, cp_egid, cp_sgid;
	gid_t cp_ruid, cp_euid, cp_suid;

	(void) state; /* unused */

	rc = setresgid(0x4444, 0x5555, 0x6666);
	assert_return_code(rc, errno);

	rc = setresuid(0x0, 0x9999, 0x9999);
	assert_return_code(rc, errno);

	cp_ruid = cp_euid = cp_suid = -1;
	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0x0);
	assert_int_equal(cp_euid, 0x9999);
	assert_int_equal(cp_suid, 0x9999);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x4444);
	assert_int_equal(cp_egid, 0x5555);
	assert_int_equal(cp_sgid, 0x6666);

	rc = setresgid(0x5555, 0x6666, 0x4444);
	assert_return_code(rc, errno);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0x6666);
	assert_int_equal(cp_sgid, 0x4444);

	rc = setresgid(0x5555, 0x4444, -1);
	assert_return_code(rc, errno);

	cp_rgid = cp_egid = cp_sgid = -1;
	rc = getresgid(&cp_rgid, &cp_egid, &cp_sgid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_rgid, 0x5555);
	assert_int_equal(cp_egid, 0x4444);
	assert_int_equal(cp_sgid, 0x4444);

	rc = setresgid(0x1111, 0x2222, 0x3333);
	assert_int_equal(rc, -1);
	assert_int_equal(errno, EPERM);

	setresuid(0x0, 0x0, 0x0);
	setresgid(0x0, 0x0, 0x0);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_setresgid_unprivileged_uid),
		cmocka_unit_test(test_uwrap_setresgid_unprivileged_uid_and_gid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
