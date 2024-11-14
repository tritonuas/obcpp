#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <mutex>


#define PORT "4345"
#define MAX_MSG_SIZE 100

/*
Reference: https://www.beej.us/guide/bgnet/html/
*/

class TCPClient{
 public:
    TCPClient(std::string host, uint16_t port);
    ~TCPClient();
 private:
    std::mutex server_mut;
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    void *get_in_addr(struct sockaddr *sa);
}

// https://man7.org/linux/man-pages/man2/connect.2.html
// https://man7.org/linux/man-pages/man3/getaddrinfo.3.html