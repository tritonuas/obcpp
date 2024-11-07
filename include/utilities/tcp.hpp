#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <mutex>


#define PORT "4345"

#define MAX_MSG_SIZE 100

class TCPClient{
 public:
    TCPClient(std::string host, uint16_t port);
    ~TCPClient();
 private:
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    std::mutex server_mut;
    int rv;
    char s[INET6_ADDRSTRLEN];
}

// https://man7.org/linux/man-pages/man2/connect.2.html
// https://man7.org/linux/man-pages/man3/getaddrinfo.3.html