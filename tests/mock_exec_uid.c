#include "config.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#ifndef discard_const
#define discard_const(ptr) ((void *)((uintptr_t)(ptr)))
#endif

#ifndef discard_const_p
#define discard_const_p(type, ptr) ((type *)discard_const(ptr))
#endif

struct user_ids {
	uid_t ruid;
	uid_t euid;
	uid_t suid;
	gid_t gid;
};

static void print_usage(const char *prog_name)
{
	fprintf(stderr,
		"Usage: %s [-r ruid] [-e euid] [-s suid] [-g gid]\n"
		"\n"
		"    -r The ruid to validate\n"
		"    -e The euid to validate\n"
		"    -s The suid to validate\n"
		"    -g The gid to validate\n"
		"\n",
		prog_name);
}

int main(int argc, char *argv[])
{
	const struct option long_options[] = {
		{ discard_const_p(char, "ruid"),  required_argument,  0, 'r' },
		{ discard_const_p(char, "euid"),  required_argument,  0, 'e' },
		{ discard_const_p(char, "suid"),  required_argument,  0, 's' },
		{ discard_const_p(char, "gid"),   required_argument,  0, 'g' },
		{ 0,                              0,                  0, 0 }
	};
	int opt_idx;
	int opt;
	struct user_ids expected_ids = {
		.ruid = (uid_t)-1,
		.euid = (uid_t)-1,
		.suid = (uid_t)-1,
		.gid  = (gid_t)-1,
	};
	struct user_ids real_ids = {
		.ruid = (uid_t)-1,
		.euid = (uid_t)-1,
		.suid = (uid_t)-1,
		.gid  = (gid_t)-1,
	};
	int rc;

	for (opt = getopt_long(argc, argv, "r:e:s:g:", long_options, &opt_idx);
	     opt != -1;
	     opt = getopt_long(argc, argv, "r:e:s:g:", long_options, &opt_idx)) {
		errno = 0;

		switch (opt) {
			case 0:
				break;
			case 'r':
				expected_ids.ruid = strtol(optarg,
							   (char **)NULL,
							   10);
				break;
			case 'e':
				expected_ids.euid = strtol(optarg,
						           (char **)NULL,
							   10);
				break;
			case 's':
				expected_ids.suid = strtol(optarg,
						           (char **)NULL,
							   10);
				break;
			case 'g':
				expected_ids.gid = strtol(optarg,
						          (char **)NULL,
							  10);
				break;
			default:
				print_usage(argv[0]);
				return 1;
		}

		if (errno == EINVAL || errno == ERANGE) {
			return 1;
		}
	}

	if (expected_ids.ruid == (uid_t)-1 &&
	    expected_ids.euid == (uid_t)-1 &&
	    expected_ids.suid == (uid_t)-1) {
		print_usage(argv[0]);
		return 1;
	}

	rc = getresuid(&real_ids.ruid, &real_ids.euid, &real_ids.suid);
	if (rc != 0) {
		fprintf(stderr, "getresuid() failed - %s\n", strerror(errno));
		return 1;
	}

	if (expected_ids.ruid != (uid_t)-1) {
		if (expected_ids.ruid != real_ids.ruid) {
			printf("MOCK_TEST ruid mismatch - ruid=%u, expected ruid=%u\n",
			       real_ids.ruid,
			       expected_ids.ruid);
			return 1;
		}
		printf("MOCK_TEST ruid=%d\n", real_ids.ruid);
	}

	if (expected_ids.euid != (uid_t)-1) {
		if (expected_ids.euid != real_ids.euid) {
			printf("MOCK_TEST euid mismatch - euid=%u, expected euid=%u\n",
			       real_ids.euid,
			       expected_ids.euid);
			return 1;
		}
		printf("MOCK_TEST euid=%d\n", real_ids.euid);
	}

	if (expected_ids.suid != (uid_t)-1) {
		if (expected_ids.suid != real_ids.suid) {
			printf("MOCK_TEST suid mismatch - suid=%u, expected suid=%u\n",
			       real_ids.suid,
			       expected_ids.suid);
			return 1;
		}
		printf("MOCK_TEST suid=%d\n", real_ids.suid);
	}

	real_ids.gid = getgid();
	if (real_ids.gid != (gid_t)-1) {
		if (expected_ids.gid != real_ids.gid) {
			printf("MOCK_TEST gid mismatch - gid=%u, expected gid=%u\n",
			       real_ids.gid,
			       expected_ids.gid);
			return 1;
		}
		printf("MOCK_TEST gid=%d\n", real_ids.gid);
	}

	return 0;
}
