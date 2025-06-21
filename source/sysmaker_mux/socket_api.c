#include "sysmaker_mux/socket_api.h"

#define SOCKET_API_DEBUG_MSG 1

void free_server_obj(struct server *obj)
{
    free((char *)obj->m_pathOrIp);
    obj->m_pathOrIp = NULL;
}

void server_create_socket(struct server *obj)
{
    int _server_fd;
    if (obj->m_isUnix) {
        _server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    } else {
        _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    if (_server_fd < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
        perror("socket");
#endif
        exit(EXIT_FAILURE);
    }
    obj->_server_fd = _server_fd;
}

void server_bind_listen_socket(struct server *obj)
{
    if (obj->m_isUnix) {
        // UNIX socket
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, obj->m_pathOrIp, sizeof(addr.sun_path) - 1);

        // 如果該路徑已存在，先刪除
        unlink(obj->m_pathOrIp);

        if (bind(obj->_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("bind(unix)");
#endif
            exit(EXIT_FAILURE);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("UNIX server bind at %s\n", obj->m_pathOrIp);
#endif
    } else {
        // TCP
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(obj->m_port);
        // ipbuf 轉成 addr.sin_addr
        if (strcmp(obj->m_pathOrIp, "0.0.0.0") == 0) {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            if (inet_pton(AF_INET, obj->m_pathOrIp, &addr.sin_addr) <= 0) {
#if SOCKET_API_DEBUG_MSG >= 1
                perror("inet_pton");
#endif
                exit(EXIT_FAILURE);
            }
        }
        if (bind(obj->_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("bind(tcp)");
#endif
            exit(EXIT_FAILURE);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("TCP server bind at %s:%d\n", obj->m_pathOrIp, obj->m_port);
#endif
    }

    if (listen(obj->_server_fd, 5) < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
        perror("listen");
#endif
        exit(EXIT_FAILURE);
    }
    // 等待客戶端連線
#if SOCKET_API_DEBUG_MSG >= 2
    printf("Server listening...\n");
#endif
    obj->client_fd = accept(obj->_server_fd, NULL, NULL);
    if (obj->client_fd < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
        perror("accept");
#endif
        exit(EXIT_FAILURE);
    }
#if SOCKET_API_DEBUG_MSG >= 2
    printf("Client connected!\n");
#endif
}

void server_close(struct server *obj)
{
    /// 關閉 client 連線
    if (obj->client_fd >= 0) {
        close(obj->client_fd);
        obj->client_fd = -1;
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Client connection closed.\n");
#endif
    }
    /// 關閉 server 連線
    if (obj->_server_fd >= 0) {
        if (obj->m_isUnix) {
            // 清理 unix path
            unlink(obj->m_pathOrIp);
        }
        close(obj->_server_fd);
        obj->_server_fd = -1;
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Server closed.\n");
#endif
    }
    free_server_obj(obj);
}

void free_client_obj(struct client *obj)
{
    free((char *)obj->m_pathOrIp);
    obj->m_pathOrIp = NULL;
}

void client_create_socket(struct client *obj)
{
    int server_fd;
    if (obj->m_isUnix) {
        server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    } else {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    if (server_fd < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
        perror("socket");
#endif
        exit(EXIT_FAILURE);
    }
    obj->server_fd = server_fd;
}

void client_connect(struct client *obj)
{
    if (obj->m_isUnix) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, obj->m_pathOrIp, sizeof(addr.sun_path) - 1);

        if (connect(obj->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("connect (unix)");
#endif
            close(obj->server_fd);
            exit(EXIT_FAILURE);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Connected to UNIX server: %s\n", obj->m_pathOrIp);
#endif
    } else {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(obj->m_port);
        if (inet_pton(AF_INET, obj->m_pathOrIp, &addr.sin_addr) <= 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("inet_pton");
#endif
            close(obj->server_fd);
            exit(EXIT_FAILURE);
        }
        if (connect(obj->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("connect (tcp)");
#endif
            close(obj->server_fd);
            exit(EXIT_FAILURE);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Connected to TCP server: %s:%d\n", obj->m_pathOrIp, obj->m_port);
#endif
    }
}

int client_connect_timeout(struct client *obj, int sec)
{
    if (obj->m_isUnix) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, obj->m_pathOrIp, sizeof(addr.sun_path) - 1);

        for (int cnt = 0; connect(obj->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0; ++cnt) {
            if (cnt == 0) {
#if SOCKET_API_DEBUG_MSG >= 2
                printf("Retry connect to UNIX server: %s for %d secs\n", obj->m_pathOrIp, sec);
#endif
            }
            if (cnt >= sec) {
#if SOCKET_API_DEBUG_MSG >= 1
                perror("connect (unix)");
#endif
                return -1;
            }
            sleep(1);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Connected to UNIX server: %s\n", obj->m_pathOrIp);
#endif
    } else {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(obj->m_port);
        if (inet_pton(AF_INET, obj->m_pathOrIp, &addr.sin_addr) <= 0) {
#if SOCKET_API_DEBUG_MSG >= 1
            perror("inet_pton");
#endif
            close(obj->server_fd);
            exit(EXIT_FAILURE);
        }
        for (int cnt = 0; connect(obj->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0; ++cnt) {
            if (cnt == 0) {
#if SOCKET_API_DEBUG_MSG >= 2
                printf("Retry connect to UNIX server: %s for %d secs\n", obj->m_pathOrIp, sec);
#endif
            }
            if (cnt >= sec) {
#if SOCKET_API_DEBUG_MSG >= 1
                perror("connect (tcp)");
#endif
                return -1;
            }
            sleep(1);
        }
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Connected to TCP server: %s:%d\n", obj->m_pathOrIp, obj->m_port);
#endif
    }
    return 0;
}

void client_close(struct client *obj)
{
    if (obj->server_fd >= 0) {
        close(obj->server_fd);
        obj->server_fd = -1;
#if SOCKET_API_DEBUG_MSG >= 2
        printf("Client connection closed.\n");
#endif
    }
    free_client_obj(obj);
}

int send_data(const int fd, const void *data, int data_size)
{
    int bytesSent = send(fd, (const void *)data, data_size, 0);
    return bytesSent;
}

int recv_data(const int fd, void *buf, int buf_size)
{
    int bytesRead = recv(fd, (void *)buf, buf_size, 0);
    return bytesRead;
}

int recv_data_nowait(const int fd, void *buf, int buf_size)
{
    int bytesRead = recv(fd, (void *)buf, buf_size, MSG_DONTWAIT);
    if (bytesRead == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
    }
    if (bytesRead == 0) {
        return -1;
    }
    return bytesRead;
}
