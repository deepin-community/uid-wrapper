#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdbool.h>

bool uid_wrapper_enabled(void);

static void test_uid_wrapper_enabled(void **state)
{
    bool ok;

    (void)state; /* unused */

    ok = uid_wrapper_enabled();
    assert_true(ok);
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_uid_wrapper_enabled),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
