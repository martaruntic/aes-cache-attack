// Encryption service with a secret key (the victim).
//
// Usage: victim <lib.so> <key-hex> [socket-path]
//
// Loads the given AES implementation, sets the key once and then encrypts
// every 16-byte block a client sends. It has no bug in the usual sense:
// it never leaks the key through its output. The only leak is which cache
// lines of the AES tables it touches.

#include "aes_loader.h"
#include "protocol.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int parse_hex_key(const char *hex, uint8_t key[16])
{
    if (strlen(hex) != 32)
        return -1;
    for (int i = 0; i < 16; i++) {
        unsigned int byte;
        if (sscanf(hex + 2 * i, "%2x", &byte) != 1)
            return -1;
        key[i] = (uint8_t)byte;
    }
    return 0;
}

static int read_full(int fd, uint8_t *buf, size_t len)
{
    size_t done = 0;
    while (done < len) {
        ssize_t n = read(fd, buf + done, len - done);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        done += (size_t)n;
    }
    return 0;
}

static int write_full(int fd, const uint8_t *buf, size_t len)
{
    size_t done = 0;
    while (done < len) {
        ssize_t n = write(fd, buf + done, len - done);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        done += (size_t)n;
    }
    return 0;
}

static void serve(int client, const aes_lib *lib, const aes128_ctx *ctx)
{
    uint8_t in[AES_BLOCK_SIZE], out[AES_BLOCK_SIZE];
    unsigned long blocks = 0;

    while (read_full(client, in, sizeof(in)) == 0) {
        lib->encrypt(ctx, in, out);
        if (write_full(client, out, sizeof(out)) != 0)
            break;
        blocks++;
    }
    printf("[victim] client done, %lu blocks encrypted\n", blocks);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <lib.so> <key-hex> [socket-path]\n", argv[0]);
        return 2;
    }

    const char *socket_path = argc > 3 ? argv[3] : DEFAULT_SOCKET_PATH;
    aes_lib lib;
    aes128_ctx ctx;
    uint8_t key[16];

    if (parse_hex_key(argv[2], key) != 0) {
        fprintf(stderr, "key must be 32 hex characters\n");
        return 2;
    }
    if (aes_lib_load(&lib, argv[1]) != 0)
        return 1;

    lib.set_key(&ctx, key);
    memset(key, 0, sizeof(key));

    signal(SIGPIPE, SIG_IGN);

    int server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    unlink(socket_path);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) != 0 || listen(server, 1) != 0) {
        perror("bind/listen");
        return 1;
    }

    printf("[victim] %s, listening on %s\n", lib.impl_name(), socket_path);
    fflush(stdout);

    for (;;) {
        int client = accept(server, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR)
                continue;
            perror("accept");
            break;
        }
        serve(client, &lib, &ctx);
        close(client);
    }

    close(server);
    unlink(socket_path);
    aes_lib_close(&lib);
    return 0;
}
