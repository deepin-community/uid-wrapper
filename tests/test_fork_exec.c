#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define TEST_MOCK_EXECUTABLE "mock_exec_uid"

static void test_uwrap_fork_exec(void **state)
{
	pid_t pid;
	int rc;
	uid_t cp_ruid, cp_euid, cp_suid;
	gid_t glist[] = {0, 10000, 20000, 40000, 100000};

	(void)state; /* unused */

	rc = getresuid(&cp_ruid, &cp_euid, &cp_suid);
	assert_return_code(rc, errno);
	assert_int_equal(cp_ruid, 0);
	assert_int_equal(cp_euid, 0);
	assert_int_equal(cp_suid, 0);

	rc = setresuid(2000, -1, 4000);
	assert_return_code(rc, errno);

	rc = setgid(5000);
	assert_return_code(rc, errno);

	rc = setgroups(ARRAY_SIZE(glist), glist);
	assert_return_code(rc, errno);

	pid = fork();
	assert_return_code(pid, errno);

	/* child */
	if (pid == 0) {
		char const *argv[] = {
			TEST_MOCK_EXECUTABLE,
			"-r 2000",
			"-e 0",
			"-s 4000",
			"-g 5000",
			NULL
		};
		char cmd[1024] = {0};

		snprintf(cmd, sizeof(cmd),
			 "%s/tests/%s",
			 BINARYDIR,
			 TEST_MOCK_EXECUTABLE);

		rc = execvp(cmd, (char *const *)argv);
		exit(rc);
	}

	/* parent */
	if (pid > 0) {
		pid_t child_pid;
		int wstatus = -1;

		rc = setresuid(cp_ruid, cp_euid, cp_suid);
		assert_return_code(rc, errno);

		child_pid = waitpid(-1, &wstatus, 0);
		assert_return_code(child_pid, errno);

		assert_true(WIFEXITED(wstatus));

		assert_int_equal(WEXITSTATUS(wstatus), 0);
	}
}

int main(void) {
	int rc;

	const struct CMUnitTest uwrap_tests[] = {
		cmocka_unit_test(test_uwrap_fork_exec),
	};

	rc = cmocka_run_group_tests(uwrap_tests, NULL, NULL);

	return rc;
}
