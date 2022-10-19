#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <pwd.h>

static void test_uwrap_getresuid(void **state)
{
	int rc;
	uid_t u, ru, eu, su;

	(void) state; /* unused */

	rc = setresuid(0x4444, 0x5555, -1);
	assert_return_code(rc, errno);

	u = getuid();
	assert_int_equal(u, 0x4444);

	u = geteuid();
	assert_int_equal(u, 0x5555);

	rc = getresuid(&ru, &eu, &su);
	assert_return_code(rc, errno);

	assert_int_equal(ru, 0x4444);
	assert_int_equal(eu, 0x5555);
	assert_int_equal(su, 0);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_getresuid),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
