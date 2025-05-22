#include "sysmaker_mux/bridge.h"

bridge::bridge(sc_module_name name, bool isUnix, const char *pathOrIp, int port)
    : sc_module(name)
{
    client_obj = {
        .m_isUnix = isUnix,
        .m_pathOrIp = pathOrIp,
        .m_port = port};
    client_create_socket(&client_obj);

    SC_THREAD(run);
    sensitive << clk.pos();
}

bridge::~bridge()
{
    client_close(&client_obj);
}

void bridge::run()
{
    int waitFor = 60;
    printf("Waiting server(QEMU) connection on path \"%s\" for %d secs\n", client_obj.m_pathOrIp, waitFor);
    if (client_connect_timeout(&client_obj, waitFor) == -1)
    {
        printf("Failed to connect to server, closing...\n");
        sc_abort();
    }
    printf("Server(QEMU) connected\n");

    while (client_obj.server_fd != -1)
    {
        int bytesRecv = recv_data_nowait(client_obj.server_fd, buf, buf_size);
        if (bytesRecv > 0)
        {
            // datas are now in `buf`, start processing...

            printf("Data received: \"%.*s\"\n", bytesRecv, buf);
        }
        else if (bytesRecv == -1)
        {
            // perror("bridge::run");
            printf("Server closed\n");
            sc_stop();
        }
        // std::cout << "thread triggered @ " << sc_time_stamp() << std::endl;
        wait();
    }
    sc_stop();
}