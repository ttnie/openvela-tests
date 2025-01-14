/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "fstest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define TEST_FILE "pread_file"
#define BUF_SIZE 4

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: TestNuttxFsPread01
 ****************************************************************************/

void TestNuttxFsPread01(FAR void **state)
{
    int fd;
    struct fs_testsuites_state_s *test_state;

    test_state = (struct fs_testsuites_state_s *)*state;

    /* malloc memory */
    char *buf = malloc(BUF_SIZE * sizeof(char));
    assert_non_null(buf);
    test_state->ptr = buf;

    /* open file */
    fd = open(TEST_FILE, O_RDWR | O_CREAT, 0777);
    assert_int_not_equal(fd, -1);
    test_state->fd1 = fd;

    /* do write */
    int ret = write(fd, "ABCD", BUF_SIZE);
    assert_int_in_range(ret, 1, 4);

    /* reset file pos use fd */
    lseek(fd, 0, SEEK_SET);

    /* set memory */
    memset(buf, '\0', BUF_SIZE);

    /* do pread */
    pread(fd, buf, 2, 2);

    assert_int_equal(strncmp(buf, "CD", 2), 0);

    /* set memory */
    memset(buf, '\0', BUF_SIZE);

    /* the pread do not change file pointer */
    pread(fd, buf, 3, 1);
    assert_int_equal(strncmp(buf, "BCD", 3), 0);
}