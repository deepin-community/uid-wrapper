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

#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_SYSCALL_H
#include <syscall.h>
#endif

#define NUM_THREADS 10

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


static pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sleep_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *uwrap_getuid_sync(void *arg)
{
	uid_t u;

	(void) arg; /* unused */

	pthread_mutex_unlock(&sleep_mutex);
	pthread_mutex_lock(&wait_mutex);

	u = getuid();
	assert_int_equal(u, 888);

	return NULL;
}

static void *uwrap_setuid_sync(void *arg)
{
	int rc;

	(void) arg; /* unused */

	rc = setuid(888);
	assert_int_equal(rc, 0);

	return NULL;
}

static void test_real_sync_setuid(void **state)
{
	pthread_t threads[2];
	uid_t u;

	(void) state; /* unused */

	pthread_mutex_lock(&wait_mutex);
	pthread_mutex_lock(&sleep_mutex);

	/* Create thread which will wait for change. */
	pthread_create(&threads[0],
		       NULL,
		       uwrap_getuid_sync,
		       NULL);

	pthread_mutex_lock(&sleep_mutex);

	pthread_create(&threads[1],
		       NULL,
		       uwrap_setuid_sync,
		       NULL);

	pthread_join(threads[1], NULL);

	pthread_mutex_unlock(&wait_mutex);

	pthread_join(threads[0], NULL);

	u = getuid();
	assert_int_equal(u, 888);
}

int main(void) {
	int rc;

	const struct CMUnitTest thread_tests[] = {
		cmocka_unit_test(test_real_sync_setuid),
	};

	rc = cmocka_run_group_tests(thread_tests, NULL, NULL);

	return rc;
}
