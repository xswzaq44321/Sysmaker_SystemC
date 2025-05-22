#ifndef SOCKET_API_H

#define SOCKET_API_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>

// for server

/// @brief 傳遞「模式 + 位址資訊」：若是 UNIX，就帶 unixPath；若是 TCP，就帶 ip, port
/// - isUnix = true => 使用 unixPath
/// - isUnix = false => 使用 ip + port
struct server
{
    bool m_isUnix;
    const char *m_pathOrIp;
    int m_port;
    int client_fd;
    int _server_fd;
};

/// @brief create socket on server side
/// @param obj you need to fill in members starting with `m_` in struct server in advance
void server_create_socket(struct server *obj);

/// @brief bind server socket to given path or IP
///         and wait for client's connection
/// @param obj struct server
void server_bind_listen_socket(struct server *obj);

/// @brief close server and client connections
/// @param obj struct server
void server_close(struct server *obj);

// for client

/// @brief 傳遞「模式 + 位址資訊」：若是 UNIX，就帶 unixPath；若是 TCP，就帶 ip, port
/// - isUnix = true => 使用 unixPath
/// - isUnix = false => 使用 ip + port
struct client
{
    bool m_isUnix;
    const char *m_pathOrIp;
    int m_port;
    int server_fd;
};

/// @brief create socket on client side
/// @param obj you need to fill in members starting with `m_` in struct server in advance
void client_create_socket(struct client *obj);

/// @brief connect to server with given paht or IP, exits if cannot connect
/// @param obj struct client
void client_connect(struct client *obj);

/// @brief connect to server with given paht or IP with given timeout
/// @param obj struct client
/// @param sec timeout in seconds
/// @return 0 if success, -1 if cannot connect or timeout
int client_connect_timeout(struct client *obj, int sec);

/// @brief close socket connection
/// @param obj struct client
void client_close(struct client *obj);

// for both

/// @brief send data to socket file descriptor
/// @param fd socket file descriptor
/// @param data data to send
/// @param data_size size of data to send
/// @return number of bytes sent
int send_data(const int fd, const void *data, int data_size);

/// @brief receive data from socket file descriptor
/// @param fd socket file descriptor
/// @param buf buffer to receive data
/// @param buf_size buffer size
/// @return number of bytes read
int recv_data(const int fd, void *buf, int buf_size);

/// @brief receive data from socket file descriptor non-blockingly
/// @param fd socket file descriptor
/// @param buf buffer to receive data
/// @param buf_size buffer size
/// @return number of bytes read, 0 if no available data to receive
int recv_data_nowait(const int fd, void *buf, int buf_size);

#endif