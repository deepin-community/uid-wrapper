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

struct parm {
	int id;
	int ready;
};

pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *syscall_setreuid(void *arg)
{
	long int rc;
	uid_t ru;

	(void) arg; /* unused */

	/* This load can help with revealing race conditions. */
	for (ru = 0; ru < 2048; ++ru) {
		uid_t ruid, euid, suid;

		rc = syscall(SYS_setreuid, -1, ru);
		assert_int_equal(rc, 0);

#ifdef HAVE_GETRESUID
		ruid = euid = suid = -1;
		rc = getresuid(&ruid, &euid, &suid);
		assert_int_equal(ruid, 0);
		assert_int_equal(euid, ru);
		assert_int_equal(suid, ru);
#endif
		ruid = getuid();
		assert_int_equal(ruid, 0);
		euid = geteuid();
		assert_int_equal(euid, ru);

		rc = syscall(SYS_setreuid, -1, 0);
		assert_int_equal(rc, 0);

#ifdef HAVE_GETRESUID
		ruid = euid = suid = -1;
		rc = getresuid(&ruid, &euid, &suid);
		assert_int_equal(ruid, 0);
		assert_int_equal(euid, 0);
		assert_int_equal(suid, ru);
#endif

		ruid = getuid();
		assert_int_equal(ruid, 0);
		euid = geteuid();
		assert_int_equal(euid, 0);
	}

	return NULL;
}

static void *sync_setreuid(void *arg)
{
	struct parm *p = (struct parm *)arg;
	uid_t u;

	syscall_setreuid(arg);

	p->ready = 1;

	pthread_mutex_lock(&msg_mutex);

	u = geteuid();
	assert_int_equal(u, 42);

	pthread_mutex_unlock(&msg_mutex);

	return NULL;
}

static void test_sync_setreuid(void **state)
{
	pthread_attr_t pthread_custom_attr;
	pthread_t threads[NUM_THREADS];
	struct parm *p;
	int rc;
	int i;

	(void) state; /* unused */

	pthread_attr_init(&pthread_custom_attr);

	p = malloc(NUM_THREADS * sizeof(struct parm));
	assert_non_null(p);

	pthread_mutex_lock(&msg_mutex);

	for (i = 0; i < NUM_THREADS; i++) {
		p[i].id = i;
		p[i].ready = 0;

		pthread_create(&threads[i],
			       &pthread_custom_attr,
			       sync_setreuid,
			       (void *)&p[i]);
	}

	/* wait for the threads to set euid to 0 */
	for (i = 0; i < NUM_THREADS; i++) {
		while (p[i].ready != 1) {
			sleep(1);
		}
	}

	rc = setreuid(-1, 42);
	assert_int_equal(rc, 0);

	pthread_mutex_unlock(&msg_mutex);

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	pthread_attr_destroy(&pthread_custom_attr);
	free(p);
}

int main(void) {
	int rc;

	const struct CMUnitTest thread_tests[] = {
		cmocka_unit_test(test_sync_setreuid),
	};

	rc = cmocka_run_group_tests(thread_tests, NULL, NULL);

	return rc;
}
