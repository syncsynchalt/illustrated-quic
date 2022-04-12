#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include "quiche.h"

void die(const char *str)
{
    fprintf(stderr, "%s: %s\n", str, strerror(errno));
    exit(1);
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

bool got_done = false;

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

        if (strcmp((const char *)buf, "done") == 0) {
            got_done = true;
        }
    }
    quiche_stream_iter_free(iter);
}

void perform_recvs(quiche_conn *conn, int sock)
{
    struct sockaddr_storage addr_storage = { 0 };
    struct sockaddr *addr = (struct sockaddr *)&addr_storage;
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

void perform_sends(quiche_conn *conn, int sock)
{
    quiche_send_info send_info = {{ 0 }};
    char buf[10000];
    for (;;) {
        ssize_t tosend = quiche_conn_send(conn, (uint8_t *)buf, sizeof(buf), &send_info);
        if (tosend == QUICHE_ERR_DONE)
            return;
        if (tosend < 0)
            die("quiche send failure");
        ssize_t sent = send(sock, buf, tosend, 0);
        if (sent != tosend)
            die("send failure");
    }
}

void perform_sends_and_recvs(quiche_conn *conn, int sock)
{
    quiche_conn_on_timeout(conn);
    perform_sends(conn, sock);
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

void do_full_send(quiche_conn *conn, uint64_t stream, const char *data, size_t data_len)
{
    ssize_t wrote = quiche_conn_stream_send(conn, stream, (uint8_t *)data, data_len, true);
    if (wrote < 0)
        die("stream send error");
    if (wrote != data_len)
        die("short write");
    printf("Wrote [%.*s] to stream %lld (final)\n", (int)wrote, data, stream);
}

int main(int argc, char **argv)
{
    quiche_config *config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (!config)
        die("new config failure");
    quiche_config_verify_peer(config, false);
    quiche_config_log_keys(config);
    quiche_config_set_application_protos(config, (uint8_t *)"\x08ping/1.0", 9);
    quiche_config_set_initial_max_data(config, 10*1024*1024);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1024*1024);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1024*1024);
    quiche_config_set_initial_max_stream_data_uni(config, 1024*1024);
    quiche_config_set_initial_max_streams_bidi(config, 10);
    quiche_config_set_initial_max_streams_uni(config, 10);

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        die("Unable to create socket");

    // create connection to localhost:8400
    struct sockaddr_storage addr_storage;
    size_t addr_len = resolve_hostname("127.0.0.1", "8400", &addr_storage);
    struct sockaddr *addr = (struct sockaddr *)&addr_storage;
    if (connect(sock, addr, addr_len) < 0)
        die("Unable to connect to 127.0.0.1:8400");

    const char conn_id[] = "c_cid";
    quiche_conn *conn = quiche_connect("example.ulfheim.net",
            (const uint8_t *)conn_id, strlen(conn_id), addr, addr_len, config);
    if (!conn)
        die("quiche connect failure");
    unlink("/tmp/keylog.txt");
    int keylogfd = open("/tmp/keylog.txt", O_WRONLY | O_CREAT, 0644);
    quiche_conn_set_keylog_fd(conn, keylogfd);

    while (!quiche_conn_is_established(conn)) {
        perform_sends_and_recvs(conn, sock);
    }

    // write "ping" on channel 0b0000 (first client-initiated
    // bidirectional stream) and close it
    do_full_send(conn, 0x0000, "ping", 4);

    // read until we get a "done"
    while (!got_done) {
        perform_sends_and_recvs(conn, sock);
    }

    uint8_t reason[] = "graceful shutdown";
    if (quiche_conn_close(conn, false, 0x00, reason, sizeof(reason)-1) < 0)
        die("close error");
    while (!quiche_conn_is_closed(conn)) {
        perform_sends_and_recvs(conn, sock);
    }
    printf("Closed connection\n");

    quiche_conn_free(conn);
    quiche_config_free(config);
    close(keylogfd);

    return 0;
}
