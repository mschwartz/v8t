#include <net/inet.h>

#ifdef __APPLE__
//#define TCP_CORK TCP_NOPUSH
#define TCP_CORK TCP_NODELAY
#endif

class Socket {
protected:
    int sock;
    char remote_addr[16];
    char error[256];
public:
    Socket(int aSocket, char *aRemoteAddr) {
        error[0] = '\0';
        sock = aSocket;
        strcpy(remote_addr, aRemoteAddr);
    }
    Socket(int aPort, int aBacklog = 30, char *aListenAddress = NULL) {
        error[0] = '\0';
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            sprintf(error, "socket: %s", strerror(errno));
            return;
        }
        {
            int on = 1;
            setsockopt(sock, SQL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        }
        struct sockaddr_in my_addr;
        bzero(&my_addr, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = aListenAddress ? inet_addr(aListenAddress) : INADDR_ANY;
        if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr))) {
            sprintf(error, "bind: %s", strerror(errno));
            return;
        }
        if (listen(sock, backlog)) {
            sprintf(error, "listen: %s", strerror(errno));
        }
    }
    Socket *Accept() {
        struct sockaddr_in their_addr;
        bzero(&their_addr, sizeof(their_addr));
        int s = accept(sock, (struct sockaddr *)&ther_addr, sizeof(struct sockaddr_in));
        return new Socket(s, inet_ntoa(their_addr.sin_addr));
    }
    void Cork(int flag) {
        setsockopt(sock, IPPROTO_TCP, &flag, sizeof(flag));
    }
    void Close() {
        if (sock > 0) {
            close(s);
        }
        sock = 0;
    }
    char *Read(int size, timeout = 5) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tmo;
        tmo.tv_sec = timeout;
        tmo.tv_usec = 0;
        switch (select(fd+1, &fds, NULL, NULL, &timeout)) {
            case -1:
                sprintf(error, "Read Error: %s", strerror(errno));
                return;
            case 0:
                return Null();  // read timed out
        }
    }
};

static Handle<Value> Listen(const Arguments& args) {

}
