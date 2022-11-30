/*
 * Copyright (c) 2015 Kazuho Oku, DeNA Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef NEVERBLEED_H
#define NEVERBLEED_H

#include <pthread.h>
#include <sys/un.h>
#include <openssl/engine.h>

#ifdef __cplusplus
extern "C" {
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
#define NEVERBLEED_HAS_PTHREAD_SETAFFINITY_NP 1
#if defined(__linux__)
#define NEVERBLEED_CPU_SET_T cpu_set_t
#else
#define NEVERBLEED_CPU_SET_T cpuset_t
#endif
#endif

#define NEVERBLEED_ERRBUF_SIZE (256)
#define NEVERBLEED_AUTH_TOKEN_SIZE 32

typedef struct st_neverbleed_t {
    ENGINE *engine;
    pid_t daemon_pid;
    struct sockaddr_un sun_;
    pthread_key_t thread_key;
    unsigned char auth_token[NEVERBLEED_AUTH_TOKEN_SIZE];
} neverbleed_t;

struct expbuf_t {
    char *buf;
    char *start;
    char *end;
    size_t capacity;

    void *data;
};

/**
 * initializes the privilege separation engine (returns 0 if successful)
 */
int neverbleed_init(neverbleed_t *nb, char *errbuf);
/**
 * loads a private key file (returns 1 if successful)
 */
int neverbleed_load_private_key_file(neverbleed_t *nb, SSL_CTX *ctx, const char *fn, char *errbuf);
/**
 * setuidgid (also changes the file permissions so that `user` can connect to the daemon, if change_socket_ownership is non-zero)
 */
int neverbleed_setuidgid(neverbleed_t *nb, const char *user, int change_socket_ownership);

#if NEVERBLEED_HAS_PTHREAD_SETAFFINITY_NP
/**
 * set the cpu affinity for the neverbleed thread (returns 0 if successful)
 */
int neverbleed_setaffinity(neverbleed_t *nb, NEVERBLEED_CPU_SET_T *cpuset);
#endif

/**
 * an optional callback that can be registered by the application for doing stuff immediately after the neverbleed process is being
 * spawned
 */
extern void (*neverbleed_post_fork_cb)(void);
extern void (*neverbleed_transaction_cb)(struct expbuf_t *);

typedef void (*neverbleed_cb)(int);

int neverbleed_get_fd(neverbleed_t *nb);
void neverbleed_set_buffer_data(neverbleed_t *nb, void *data);
size_t neverbleed_buffer_size(struct expbuf_t *buf);
void neverbleed_transaction_read(neverbleed_t *nb, struct expbuf_t *buf);
void neverbleed_transaction_write(neverbleed_t *nb, struct expbuf_t *buf);

#ifdef __cplusplus
}
#endif

#endif
