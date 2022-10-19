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

static void *uwrap_setgid_syscall(void *arg)
{
	int rc;
	gid_t g;
	(void) arg; /* unused */

	rc = syscall(SYS_setgid, 999);
	assert_int_equal(rc, 0);

	g = getgid();
	assert_int_equal(g, 999);

	return NULL;
}

static void test_sync_setgid_syscall(void **state)
{
	pthread_attr_t pthread_custom_attr;
	pthread_t threads[NUM_THREADS];
	gid_t go, gn;
	int i;

	(void) state; /* unused */

	pthread_attr_init(&pthread_custom_attr);

	go = getgid();

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i],
			       &pthread_custom_attr,
			       uwrap_setgid_syscall,
			       NULL);
	}

	/* wait for threaads */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	gn = getgid();
	assert_int_equal(gn, go);

	pthread_attr_destroy(&pthread_custom_attr);
}

static void *uwrap_setgid(void *arg)
{
	int rc;
	(void) arg; /* unused */

	rc = setgid(999);
	assert_int_equal(rc, 0);

	return NULL;
}

static void *uwrap_getgid(void *arg)
{
	gid_t g;
	(void) arg; /* unused */

	g = getgid();
	assert_int_equal(g, 1999);

	return NULL;
}

static void test_sync_setgid(void **state)
{
	pthread_attr_t pthread_custom_attr;
	pthread_t threads[NUM_THREADS];
	gid_t g;
	int i;
	int rc;

	(void) state; /* unused */

	pthread_attr_init(&pthread_custom_attr);

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i],
			       &pthread_custom_attr,
			       uwrap_setgid,
			       NULL);
	}

	/* wait for threaads */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	g = getgid();
	assert_int_equal(g, 999);

	rc = setgid(1999);
	assert_int_equal(rc, 0);

	pthread_create(&threads[0],
		       &pthread_custom_attr,
		       uwrap_getgid,
		       NULL);

	pthread_join(threads[0], NULL);

	g = getgid();
	assert_int_equal(g, 1999);

	pthread_attr_destroy(&pthread_custom_attr);
}


static void *uwrap_setgroups(void *arg)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	int rc;

	(void) arg; /* unused */

	rc = setgroups(ARRAY_SIZE(glist), glist);
	assert_int_equal(rc, 0);

	return NULL;
}

static void *uwrap_getgroups(void *arg)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	gid_t rlist[16];
	int rc;

	(void) arg; /* unused */

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 5);

	assert_memory_equal(glist, rlist, sizeof(glist));

	return NULL;
}

static void test_sync_setgroups(void **state)
{
	gid_t glist[] = { 100, 200, 300, 400, 500 };
	pthread_t threads[NUM_THREADS];
	gid_t rlist[16];
	int rc;
	int i;

	(void) state; /* unused */

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i],
			       NULL,
			       uwrap_setgroups,
			       NULL);
	}

	/* wait for threaads */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	rc = getgroups(ARRAY_SIZE(rlist), rlist);
	assert_int_equal(rc, 5);

	assert_memory_equal(glist, rlist, sizeof(glist));

	pthread_create(&threads[0],
		       NULL,
		       uwrap_getgroups,
		       NULL);

	pthread_join(threads[0], NULL);

}

static void *uwrap_create_thread_setgid(void *arg)
{
	pthread_t thread;
	signed int tmp;
	gid_t g, g_a;
	int rc;

	tmp = *((signed int *) arg);

	if (tmp >= 0) {
		g_a = (gid_t) *((unsigned int *) arg);

		rc = syscall(SYS_setgid, g_a);
		assert_int_equal(rc, 0);

		g = getgid();
		assert_int_equal(g, g_a);

		tmp *= -1;
		rc = pthread_create(&thread,
				NULL,
				&uwrap_create_thread_setgid,
				(void *) &tmp);
		assert_int_equal(rc, 0);

		pthread_join(thread, NULL);

		g = getgid();
		assert_int_equal(g, g_a);
	} else {
		tmp *= -1;
		g_a = (gid_t) tmp;
		g = getgid();
		assert_int_equal(g, g_a);
	}

	pthread_exit(NULL);
}

static void test_thread_create_thread_setgid(void **state)
{
	pthread_t thread;
	signed int tmp = 666;
	gid_t g;
	int rc;

	(void) state; /* unused */

	rc = setgid(555);
	assert_int_equal(rc, 0);
	g = getgid();
	assert_int_equal(g, 555);

	pthread_create(&thread,
			NULL,
			&uwrap_create_thread_setgid,
			&tmp);
	pthread_join(thread, NULL);
}

int main(void) {
	int rc;

	const struct CMUnitTest thread_tests[] = {
		cmocka_unit_test(test_sync_setgid),
		cmocka_unit_test(test_sync_setgid_syscall),
		cmocka_unit_test(test_sync_setgroups),
		cmocka_unit_test(test_thread_create_thread_setgid),
	};

	rc = cmocka_run_group_tests(thread_tests, NULL, NULL);

	return rc;
}
