/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SocketTest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define SERVER_PORT 9001 //
#define INVALID_SOCKET -1 //
#define BACKLOG CLIENT_NUM

/****************************************************************************
 * Public Functions
 ****************************************************************************/
static int gFds[FD_SETSIZE];
static int gBye;

static void InitFds(void)
{
    for (int i = 0; i < FD_SETSIZE; ++i) {
        gFds[i] = INVALID_SOCKET;
    }
}

static void GetReadfds(fd_set *fds, int *nfd)
{
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (gFds[i] == INVALID_SOCKET) {
            continue;
        }
        FD_SET(gFds[i], fds);
        if (*nfd < gFds[i]) {
            *nfd = gFds[i];
        }
    }
}

static int AddFd(int fd)
{
    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (gFds[i] == INVALID_SOCKET) {
            gFds[i] = fd;
            return 0;
        }
    }
    return -1;
}

static void DelFd(int fd)
{
    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (gFds[i] == fd) {
            gFds[i] = INVALID_SOCKET;
        }
    }
    (void)close(fd);
}

static int CloseAllFd(void)
{
    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (gFds[i] != INVALID_SOCKET) {
            (void)close(gFds[i]);
            gFds[i] = INVALID_SOCKET;
        }
    }
    return 0;
}

static int HandleRecv(int fd)
{
    char buf[256];
    int ret = recv(fd, buf, sizeof(buf)-1, 0);
    if (ret < 0) {
        syslog(LOG_INFO,"[%d]Error: %s", fd, strerror(errno));
        DelFd(fd);
    } else if (ret == 0) {
        syslog(LOG_INFO,"[%d]Closed", fd);
        DelFd(fd);
    } else {
        buf[ret] = 0;
        syslog(LOG_INFO,"[%d]Received: %s", fd, buf);
        if (strstr(buf, "Bye") != NULL) {
            DelFd(fd);
            gBye++;
        }
    }
    return -(ret < 0);
}

static int HandleAccept(int lsfd)
{
    struct sockaddr_in sa;
    socklen_t saLen = sizeof(sa);
    int fd = accept(lsfd, (struct sockaddr *)&sa, &saLen);//?
    if (fd == INVALID_SOCKET) {
        perror("accept");
        return -1;
    }

    if (AddFd(fd) == -1) {
        syslog(LOG_INFO,"Too many clients, refuse %s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
        close(fd);
        return -1;
    }
    syslog(LOG_INFO,"New client %d: %s:%d", fd, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    return 0;
}

static int HandleReadfds(fd_set *fds, int lsfd)
{
    int ret = 0;
    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (gFds[i] == INVALID_SOCKET || !FD_ISSET(gFds[i], fds)) {
            continue;
        }
        if (gFds[i] == lsfd) {
            ret += HandleAccept(lsfd);
        } else {
            ret += HandleRecv(gFds[i]);
        }
    }
    return ret;
}

static void *ClientsThread(void *param)
{
    int fd;
    int thrNo = (int)(intptr_t)param;

    syslog(LOG_INFO,"<%d>socket client thread started", thrNo);
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET) {
        perror("socket");
        return NULL;
    }

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa.sin_port = htons(SERVER_PORT);
    if (connect(fd, (struct sockaddr *)(&sa), sizeof(sa)) == -1) {
        perror("connect");
        return NULL;
    }

    syslog(LOG_INFO,"[%d]<%d>connected to %s:%d successful", fd, thrNo, inet_ntoa(sa.sin_addr), SERVER_PORT);

    const char *msg[] = {
        "hello, ",
        "ohos, ",
        "my name is net_socket_test_008, ",
        "see u next time, ",
        "Bye!"
    };

    for (int i = 0; i < sizeof(msg) / sizeof(msg[0]); ++i) {
        if (send(fd, msg[i], strlen(msg[i]), 0) < 0) {
            syslog(LOG_INFO,"[%d]<%d>send msg [%s] fail", fd, thrNo, msg[i]);
        }
    }

    (void)shutdown(fd, SHUT_RDWR);
    (void)close(fd);
    return param;
}

static int StartClients(pthread_t *cli, int cliNum)
{
    int ret;
    pthread_attr_t attr;

    for (int i = 0; i < cliNum; ++i) {
        ret = pthread_attr_init(&attr);
        assert_int_equal(ret, 0);

        ret = pthread_create(&cli[i], &attr, ClientsThread, (void *)(intptr_t)i);
        assert_int_equal(ret, 0);

        ret = pthread_attr_destroy(&attr);
        assert_int_equal(ret, 0);
    }
    return 0;
}

/****************************************************************************
 * Name: TestNuttxNetSocketTest08
 ****************************************************************************/

void TestNuttxNetSocketTest08(FAR void **state)
{
    struct sockaddr_in sa = {0};
    int lsfd;
    int ret;

    lsfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert_int_not_equal(lsfd, -1);

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(SERVER_PORT);
    ret = bind(lsfd, (struct sockaddr *)(&sa), sizeof(sa));
    assert_int_not_equal(ret, -1);

    ret = listen(lsfd, BACKLOG);
    assert_int_not_equal(ret, -1);

    InitFds();
    AddFd(lsfd);
    syslog(LOG_INFO,"[%d]Waiting for client to connect on port %d", lsfd, SERVER_PORT);

    pthread_t clients[CLIENT_NUM];

    ret = StartClients(clients, CLIENT_NUM);
    assert_int_equal(ret, 0);

    for ( ; ; ) {
        int nfd;
        fd_set readfds;
        struct timeval timeout;

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        nfd = 0;
        FD_ZERO(&readfds);

        GetReadfds(&readfds, &nfd);

        ret = select(nfd + 1, &readfds, NULL, NULL, &timeout);
        syslog(LOG_INFO,"select %d", ret);
        if (ret == -1) {
            perror("select");
            break; // error occurred
        } else if (ret == 0) {
            break; // timed out
        }

        if (HandleReadfds(&readfds, lsfd) < 0) {
            break;
        }
    }

    for (int i = 0; i < CLIENT_NUM; ++i) {
        ret = pthread_join(clients[i], NULL);
        assert_int_equal(ret, 0);
    }

    assert_int_equal(gBye, CLIENT_NUM);
    (void)CloseAllFd();
}
