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

    printf("Sending QEMU netlist to QEMU...\n");
    std::ifstream f(qemu_netlist);
    if (f.fail()) {
        fprintf(stderr, "Error: File \"%s\" does not exist\n", qemu_netlist.c_str());
        exit(1);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string json_str          = ss.str();
    uint64_t    qemu_netlist_size = json_str.length();
    send_data(client_obj.server_fd, &qemu_netlist_size, 8);
    send_data(client_obj.server_fd, json_str.data(), json_str.length());

    while (client_obj.server_fd != -1) {
        printf("waiting data\n");
        uint64_t incoming_size;
        int      bytesRecv = recv_data(client_obj.server_fd, &incoming_size, 8);
        // int      bytesRecv = recv_data(client_obj.server_fd, buf, buf_size);
        /* TODO: deal with when bytesRecv is 0*/
        if (bytesRecv > 0) {
            // datas are now in `buf`, start processing...

            /* TODO: solve when data pack is too large to receive at once */
            bytesRecv = recv_data(client_obj.server_fd, buf, incoming_size);
            assert(incoming_size == (uint64_t)bytesRecv && bytesRecv < buf_size);
            buf[bytesRecv] = '\0'; // append null-terminate just in case
            printf("Data size: %ld\n", incoming_size);
            printf("Data received: \"%s\"\n", buf);
            in_reqs.write(std::string(buf, bytesRecv));
        } else if (bytesRecv <= 0) {
            // perror("bridge::run");
            printf("Server closed\n");
            sc_stop();
        }
        // std::cout << "thread triggered @ " << sc_time_stamp() << std::endl;
        wait(out_reqs.data_written_event());
        std::string resp_json     = out_reqs.read();
        uint64_t    outgoing_size = resp_json.length();
        printf("Respond Data size: %ld\n", outgoing_size);
        printf("Respond Data: \"%s\"\n", resp_json.c_str());
        send_data(client_obj.server_fd, &outgoing_size, 8);
        send_data(client_obj.server_fd, resp_json.c_str(), outgoing_size);
        wait();
    }
    sc_stop();
}