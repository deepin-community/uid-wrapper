#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <pthread.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <errno.h>

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif

#define NUM_THREADS 10

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

struct parm {
	int id;
	int ready;
};

pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *syscall_setreuid(void *arg)
{
	long int rc;
	uid_t u;
	uid_t eu;

	(void) arg; /* unused */

	/* This load can help with revealing race conditions. */
	for (eu = 1; eu < 2048; ++eu) {
		rc = syscall(SYS_setreuid, -1, eu);
		assert_return_code(rc, errno);

		u = geteuid();
		assert_int_equal(u, eu);

		rc = syscall(SYS_setreuid, -1, 0);
		assert_return_code(rc, errno);
	}
	rc = syscall(SYS_setreuid, -1, 666);
	assert_return_code(rc, errno);

	return NULL;
}

static void test_syscall_setreuid(void **state)
{
	pthread_attr_t pthread_custom_attr;
	pthread_t threads[NUM_THREADS];
	uid_t u;
	int i;

	(void) state; /* unused */

	pthread_attr_init(&pthread_custom_attr);

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i],
			       &pthread_custom_attr,
			       syscall_setreuid,
			       NULL);
	}

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	u = geteuid();
	assert_int_equal(u, 0);

	pthread_attr_destroy(&pthread_custom_attr);
}

int main(void) {
	int rc;

	const struct CMUnitTest thread_tests[] = {
		cmocka_unit_test(test_syscall_setreuid),
	};

	rc = cmocka_run_group_tests(thread_tests, NULL, NULL);

	return rc;
}
