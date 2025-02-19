/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <syslog.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "fstest.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: TestNuttxFsReaddir01
 ****************************************************************************/
void TestNuttxFsReaddir01(FAR void **state)
{
	int fd, ret;
	char buf[20] = {0};
	char *filename[] = {
		"testFile1",
		"testFile2",
		"testFile3",
		"testFile4",
		"testFile5",
		"testFile6",
		"testFile7"};
	DIR *test_dir;
	struct dirent *dptr;
	struct fs_testsuites_state_s *test_state;

	test_state = (struct fs_testsuites_state_s *)*state;

	for (int i = 0; i < 6; i++)
	{
		/* open file */
		fd = open(filename[i], O_RDWR | O_CREAT, 0700);
		assert_true(fd > 0);
		test_state->fd1 = fd;
		/* do wirte */
		ret = write(fd, "hello!\n", 6);
		assert_uint_in_range(ret, 1, 6);

		close(fd);
	}

	/* do getcwd */
	getcwd(buf, sizeof(buf));

	/* open directory */
	test_dir = opendir(buf);
	assert_non_null(test_dir);

	while ((dptr = readdir(test_dir)) != 0)
	{
		if (strcmp(dptr->d_name, ".") && strcmp(dptr->d_name, ".."))
			continue;
	}

	/* close dir */
	assert_int_equal(closedir(test_dir), 0);
}