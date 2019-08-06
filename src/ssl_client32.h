/*
 *Customized ssl_client.h to support STARTTLS protocol, version 1.0.1
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* Provide SSL/TLS functions to ESP32 with Arduino IDE
 * by Evandro Copercini - 2017 - Apache 2.0 License
 */

#ifndef SSL_CLIENT32_H
#define SSL_CLIENT32_H
#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

typedef void (*DebugMsgCallback)(const char* msg);

typedef struct sslclient_context32 {
    int socket;
    bool starttls;
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;
    mbedtls_net_context server_fd;

    mbedtls_ctr_drbg_context drbg_ctx;
    mbedtls_entropy_context entropy_ctx;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;
    DebugMsgCallback _debugCallback;

    unsigned long handshake_timeout;
} sslclient_context32;


void ssl_init(sslclient_context32 *ssl_client);
int start_ssl_client(sslclient_context32 *ssl_client, const char *host, uint32_t port, int timeout, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey);
void stop_ssl_socket(sslclient_context32 *ssl_client, const char *rootCABuff, const char *cli_cert, const char *cli_key);
int data_to_read(sslclient_context32 *ssl_client);
int send_ssl_data(sslclient_context32 *ssl_client, const uint8_t *data, uint16_t len);
int get_ssl_receive(sslclient_context32 *ssl_client, uint8_t *data, int length);
bool verify_ssl_fingerprint(sslclient_context32 *ssl_client, const char* fp, const char* domain_name);
bool verify_ssl_dn(sslclient_context32 *ssl_client, const char* domain_name);
int starttlsHandshake(sslclient_context32 *ssl_client, int port);
#endif
