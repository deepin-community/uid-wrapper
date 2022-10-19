#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif

#define ZERO_STRUCT(x) memset((char *)&(x), 0, sizeof(x))

static void test_uwrap_syscall(void **state)
{
	long int rc;
	struct timeval tv1, tv2;
	struct timezone tz1, tz2;

	(void) state; /* unused */

#ifdef __alpha__
	rc = syscall(SYS_getxpid);
#else
	rc = syscall(SYS_getpid);
#endif
	assert_int_equal(rc, getpid());

	ZERO_STRUCT(tv1);
	ZERO_STRUCT(tv2);
	ZERO_STRUCT(tz1);
	ZERO_STRUCT(tz2);

	rc = gettimeofday(&tv1, &tz1);
	assert_int_equal(rc, 0);

#ifdef OSX
	tv2.tv_sec = syscall(SYS_gettimeofday, &tv2, NULL);
#else
	rc = syscall(SYS_gettimeofday, &tv2, &tz2);
	assert_int_equal(rc, 0);
	assert_int_equal(tz1.tz_dsttime, tz2.tz_dsttime);
	assert_int_equal(tz1.tz_minuteswest, tz2.tz_minuteswest);
#endif

	assert_int_equal(tv1.tv_sec, tv2.tv_sec);
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_syscall),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
