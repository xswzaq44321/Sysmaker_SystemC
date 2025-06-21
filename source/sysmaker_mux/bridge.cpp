#include "sysmaker_mux/bridge.h"

using namespace sc_core;
using namespace std;

bridge::bridge(sc_module_name name, bool isUnix, std::string pathOrIp, int port)
    : sc_module(name)
{
    char *tmp = (char *)malloc(sizeof(char) * (pathOrIp.length() + 1));
    strcpy(tmp, pathOrIp.c_str());
    client_obj = {
        .m_isUnix   = isUnix,
        .m_pathOrIp = tmp,
        .m_port     = port
    };
    client_create_socket(&client_obj);

    SC_THREAD(run);
    sensitive << clk.pos();
    dont_initialize();
}

bridge::~bridge()
{
    client_close(&client_obj);
}

void bridge::run()
{
    int waitFor = 60;
    printf("Waiting server(QEMU) connection on path \"%s\" for %d secs\n", client_obj.m_pathOrIp, waitFor);
    if (client_connect_timeout(&client_obj, waitFor) == -1) {
        printf("Failed to connect to server, closing...\n");
        sc_abort();
    }
    printf("Server(QEMU) connected\n");

    while (client_obj.server_fd != -1) {
        int bytesRecv = recv_data(client_obj.server_fd, buf, buf_size);
        /* TODO: deal with when bytesRecv is 0*/
        if (bytesRecv > 0) {
            // datas are now in `buf`, start processing...

            /* TODO: solve when data pack is too large to receive at once */

            printf("Data received: \"%.*s\"\n", bytesRecv, buf);
            in_reqs.write(std::string(buf));
        } else if (bytesRecv == -1) {
            // perror("bridge::run");
            printf("Server closed\n");
            sc_stop();
        }
        // std::cout << "thread triggered @ " << sc_time_stamp() << std::endl;
        wait(out_reqs.data_written_event());
        std::string resp_json = out_reqs.read();

        cout << "Data to send: " << resp_json << endl;
        // TODO: send transaction data back to QEMU
    }
    sc_stop();
}