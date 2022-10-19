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

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

static void test_uwrap_getgroups(void **state)
{
	gid_t rlist[20] = {0};
	int num_groups;
	int rc;

	(void) state; /* unused */
	num_groups = getgroups(0, NULL);
	assert_int_not_equal(num_groups, -1);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, num_groups);
	assert_int_equal(rlist[0], getegid());
}

static void test_uwrap_setgroups(void **state)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	gid_t rlist[16];
	int rc;

	(void) state; /* unused */

	rc = setgroups(ARRAY_SIZE(glist), glist);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 5);

	assert_memory_equal(glist, rlist, sizeof(glist));

	/* Drop all supplementary groups. This is often done by daemons */
	memset(rlist, 0, sizeof(rlist));

	rc = setgroups(0, NULL);
	assert_int_equal(rc, 0);

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 0);

	assert_int_equal(rlist[0], 0);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_getgroups),
		cmocka_unit_test(test_uwrap_setgroups),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
