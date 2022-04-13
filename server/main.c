// toy implementation - use at your own risk
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include "quiche.h"

typedef struct sockaddr sockaddr;

void die(const char *str)
{
    fprintf(stderr, "%s: %s\n", str, strerror(errno));
    exit(1);
}

int create_listen(int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        die("Unable to create socket");
    }
    int enable = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        die("SO_REUSEADDR failed");
    }
    enable = 1;
    if (setsockopt(s, IPPROTO_IP, IP_DONTFRAG, &enable, sizeof(enable)) < 0) {
        die("IP_DONTFRAG failed");
    }
    if (bind(s, (sockaddr *)&addr, sizeof(addr)) < 0) {
        die("Unable to bind to port");
    }
    printf("Listening on port %d\n", port);
    return s;
}

size_t resolve_hostname(const char *host, const char *port, struct sockaddr_storage *addr)
{
    struct addrinfo *res = 0;
    if (getaddrinfo(host, port, 0, &res) != 0)
        die("Unable to transform address");
    size_t len = res->ai_addrlen;
    memcpy(addr, res->ai_addr, len);
    freeaddrinfo(res);
    return len;
}

void do_full_send(quiche_conn *conn, uint64_t stream, const char *data, size_t data_len)
{
    ssize_t wrote = quiche_conn_stream_send(conn, stream, (uint8_t *)data, data_len, true);
    if (wrote < 0)
        die("stream send error");
    if (wrote != data_len)
        die("short write");
    printf("Wrote [%.*s] to stream %lld (final)\n", (int)wrote, data, stream);
}

bool replied = false;

void handle_reads(quiche_conn *conn)
{
    quiche_stream_iter *iter = quiche_conn_readable(conn);
    if (!iter)
        die("conn readable failure");

    uint8_t buf[10000] = {0};
    for (;;) {
        uint64_t stream_id = 0;
        bool has_next = quiche_stream_iter_next(iter, &stream_id);
        if (!has_next)
            break;

        bool is_final = false;
        ssize_t num_read = quiche_conn_stream_recv(conn, stream_id, buf, sizeof(buf), &is_final);
        if (num_read == QUICHE_ERR_DONE)
            break;
        if (num_read < 0)
            die("stream recv failure");
        printf("Read [%.*s] from stream %lld%s\n", (int)num_read, buf, stream_id, is_final ? " (final)" : "");

        if (strcmp((const char *)buf, "ping") == 0) {
            do_full_send(conn, stream_id, "pong", 4);
            replied = true;
        }
    }
    quiche_stream_iter_free(iter);
}

void perform_recvs(quiche_conn *conn, int sock)
{
    struct sockaddr_storage addr_storage = { 0 };
    sockaddr *addr = (sockaddr *)&addr_storage;
    char buf[10000];
    socklen_t addr_len = sizeof(addr_storage);

    ssize_t rb = recvfrom(sock, buf, sizeof(buf), 0, addr, &addr_len);
    if (rb < 0)
        die("recv failure");
    uint8_t *start = (uint8_t *)buf;
    uint8_t *end = start + rb;
    quiche_recv_info recv_info = { addr, addr_len };
    while (end > start) {
        ssize_t b = quiche_conn_recv(conn, start, end - start, &recv_info);
        if (b < 0)
            die("quiche recv failure");
        start += b;

        handle_reads(conn);
    }
}

void perform_sends(quiche_conn *conn, int sock, sockaddr *addr, size_t addr_len)
{
    quiche_send_info send_info = {{ 0 }};
    char buf[10000];
    for (;;) {
        ssize_t tosend = quiche_conn_send(conn, (uint8_t *)buf, sizeof(buf), &send_info);
        if (tosend == QUICHE_ERR_DONE)
            return;
        if (tosend < 0)
            die("quiche send failure");
        ssize_t sent = sendto(sock, buf, tosend, 0, addr, addr_len);
        if (sent != tosend)
            die("send failure");
    }
}

void perform_sends_and_recvs(quiche_conn *conn, int sock, sockaddr *addr, size_t addr_len)
{
    quiche_conn_on_timeout(conn);
    perform_sends(conn, sock, addr, addr_len);
    struct pollfd fds[1] = {{ sock, POLLIN, 0 }};
    int timeout = quiche_conn_timeout_as_millis(conn);
    if (timeout <= 10)
        timeout = 10;
    if (timeout > 1000)
        timeout = 1000;
    int poll_ret = poll(fds, 1, timeout);
    if (poll_ret < 0)
        die("poll failure");
    if (poll_ret > 0) {
        if (fds[0].revents & POLLIN) {
            perform_recvs(conn, sock);
        }
    }
}

sockaddr *peek_packet_addr(int sock, struct sockaddr_storage *storage, socklen_t *addr_len)
{
    // wait for our first caller (and get their address)
    sockaddr *addr = (sockaddr *)storage;
    char buf[10000];
    if (recvfrom(sock, buf, sizeof(buf), MSG_PEEK, addr, addr_len) < 0)
        die("peek failure");
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    getnameinfo(addr, *addr_len, host, sizeof(host),
        port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
    printf("Accepting connection from %s:%s\n", host, port);
    return (sockaddr *)storage;
}

int main(int argc, char **argv)
{
    quiche_config *config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (!config)
        die("new config failure");
    quiche_config_verify_peer(config, false);
    quiche_config_log_keys(config);
    quiche_config_set_application_protos(config, (uint8_t *)"\x08ping/1.0", 9);
    if (quiche_config_load_priv_key_from_pem_file(config, "server.key") < 0)
        die("set priv key");
    if (quiche_config_load_cert_chain_from_pem_file(config, "server.crt") < 0)
        die("set cert chain");
    quiche_config_set_initial_max_data(config, 5*1024*1024);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 512*1024);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 512*1024);
    quiche_config_set_initial_max_stream_data_uni(config, 512*1024);
    quiche_config_set_initial_max_streams_bidi(config, 2);
    quiche_config_set_initial_max_streams_uni(config, 2);
    quiche_config_set_max_idle_timeout(config, 2*60*1000);

    int sock = create_listen(8400);

    // wait for our first caller (and get their address)
    struct sockaddr_storage storage = { 0 };
    socklen_t addr_len = sizeof(storage);
    sockaddr *addr = peek_packet_addr(sock, &storage, &addr_len);

    const char conn_id[] = "s_cid";
    quiche_conn *conn = quiche_accept((const uint8_t *)conn_id, strlen(conn_id),
            NULL, 0, addr, addr_len, config);
    if (!conn)
        die("quiche connect failure");
    quiche_conn_set_keylog_fd(conn, STDOUT_FILENO);

    while (!quiche_conn_is_established(conn)) {
        perform_sends_and_recvs(conn, sock, addr, addr_len);
    }

    // wait for a "ping", reply "pong"
    while (!replied) {
        perform_sends_and_recvs(conn, sock, addr, addr_len);
    }
    perform_sends_and_recvs(conn, sock, addr, addr_len);

    // shut down the connection
    uint8_t reason[] = "graceful shutdown";
    if (quiche_conn_close(conn, false, 0x00, reason, sizeof(reason)-1) < 0)
        die("close error");

    // keep looping while connection drains
    while (!quiche_conn_is_closed(conn)) {
        perform_sends_and_recvs(conn, sock, addr, addr_len);
    }
    printf("Closed connection\n");

    quiche_conn_free(conn);
    quiche_config_free(config);

    return 0;
}
