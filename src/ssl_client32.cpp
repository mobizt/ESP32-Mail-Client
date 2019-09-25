/*
 *Customized ssl_client.cpp to support STARTTLS protocol, version 1.0.2
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
*
* Adapted from the ssl_client1 example of mbedtls.
*
* Original Copyright (C) 2006-2015, ARM Limited, All Rights Reserved, Apache 2.0 License.
* Additions Copyright (C) 2017 Evandro Luis Copercini, Apache 2.0 License.
*/

#include "Arduino.h"
#include <esp32-hal-log.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <mbedtls/sha256.h>
#include <mbedtls/oid.h>
#include <algorithm>
#include <string>
#include "ssl_client32.h"
#include "WiFi.h"

const char *pers32 = "esp32-tls";

static int handle_error(int err)
{
    if (err == -30848)
    {
        return err;
    }
#ifdef MBEDTLS_ERROR_C
    char error_buf[100];
    mbedtls_strerror(err, error_buf, 100);
    log_e("%s", error_buf);
#endif
    log_e("MbedTLS message code: %d", err);
    return err;
}

void ssl_init(sslclient_context32 *ssl_client)
{
    mbedtls_ssl_init(&ssl_client->ssl_ctx);
    mbedtls_ssl_config_init(&ssl_client->ssl_conf);
    mbedtls_ctr_drbg_init(&ssl_client->drbg_ctx);
    mbedtls_net_init(&ssl_client->server_fd);
}

int start_ssl_client(sslclient_context32 *ssl_client, const char *host, uint32_t port, int timeout, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey)
{
    char buf[512];
    int ret, flags;
    int enable = 1;

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: starting socket");

    log_v("Free internal heap before TLS %u", ESP.getFreeHeap());

    log_v("Starting socket");
    ssl_client->socket = -1;

    ssl_client->socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ssl_client->socket < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: opening socket");
        log_e("ERROR opening socket");
        return ssl_client->socket;
    }

    IPAddress srv((uint32_t)0);
    if (!WiFiGenericClass::hostByName(host, srv))
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: could not get ip from host");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = srv;
    serv_addr.sin_port = htons(port);

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: connecting to Server...");

    if (lwip_connect(ssl_client->socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
    {
        if (timeout <= 0)
        {
            timeout = 30000;
        }
        lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        lwip_setsockopt(ssl_client->socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
        lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: server connected");
    }
    else
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: connect to Server failed!");
        log_e("Connect to Server failed!");
        return -1;
    }

    fcntl(ssl_client->socket, F_SETFL, fcntl(ssl_client->socket, F_GETFL, 0) | O_NONBLOCK);

    if (ssl_client->starttls && (port == 25 || port == 587 || port == 143))
    {

        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: begin STARTTLS handshake");

        if ((ret = starttlsHandshake(ssl_client, port)) != 0)
        {
            log_e("STARTTLS failed!");
            return -1;
        }
    }

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: seeding the random number generator");

    log_v("Seeding the random number generator");
    mbedtls_entropy_init(&ssl_client->entropy_ctx);

    ret = mbedtls_ctr_drbg_seed(&ssl_client->drbg_ctx, mbedtls_entropy_func,
                                &ssl_client->entropy_ctx, (const unsigned char *)pers32, strlen(pers32));
    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
        {
            char *error_buf = new char[100];
            memset(buf, 0, 512);
            strcpy(buf, "ERROR: ");
            mbedtls_strerror(ret, error_buf, 100);
            strcat(buf, error_buf);
            ssl_client->_debugCallback(buf);
            delete[] error_buf;
        }

        return handle_error(ret);
    }

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: setting up the SSL/TLS structure...");

    log_v("Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&ssl_client->ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        if (ssl_client->_debugCallback)
        {
            char *error_buf = new char[100];
            memset(buf, 0, 512);
            strcpy(buf, "ERROR: ");
            mbedtls_strerror(ret, error_buf, 100);
            strcat(buf, error_buf);
            ssl_client->_debugCallback(buf);
            delete[] error_buf;
        }
        return handle_error(ret);
    }

    // MBEDTLS_SSL_VERIFY_REQUIRED if a CA certificate is defined on Arduino IDE and
    // MBEDTLS_SSL_VERIFY_NONE if not.

    if (rootCABuff != NULL)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: loading CA cert");
        log_v("Loading CA cert");
        mbedtls_x509_crt_init(&ssl_client->ca_cert);
        mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        ret = mbedtls_x509_crt_parse(&ssl_client->ca_cert, (const unsigned char *)rootCABuff, strlen(rootCABuff) + 1);
        mbedtls_ssl_conf_ca_chain(&ssl_client->ssl_conf, &ssl_client->ca_cert, NULL);
        //mbedtls_ssl_conf_verify(&ssl_client->ssl_ctx, my_verify, NULL );
        if (ret < 0)
        {
            if (ssl_client->_debugCallback)
            {
                char *error_buf = new char[100];
                memset(buf, 0, 512);
                strcpy(buf, "ERROR: ");
                mbedtls_strerror(ret, error_buf, 100);
                strcat(buf, error_buf);
                ssl_client->_debugCallback(buf);
                delete[] error_buf;
            }
            return handle_error(ret);
        }
    }
    else if (pskIdent != NULL && psKey != NULL)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: setting up PSK");
        log_v("Setting up PSK");
        // convert PSK from hex to binary
        if ((strlen(psKey) & 1) != 0 || strlen(psKey) > 2 * MBEDTLS_PSK_MAX_LEN)
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: pre-shared key not valid hex or too long");
            log_e("pre-shared key not valid hex or too long");
            return -1;
        }
        unsigned char psk[MBEDTLS_PSK_MAX_LEN];
        size_t psk_len = strlen(psKey) / 2;
        for (int j = 0; j < strlen(psKey); j += 2)
        {
            char c = psKey[j];
            if (c >= '0' && c <= '9')
                c -= '0';
            else if (c >= 'A' && c <= 'F')
                c -= 'A' - 10;
            else if (c >= 'a' && c <= 'f')
                c -= 'a' - 10;
            else
                return -1;
            psk[j / 2] = c << 4;
            c = psKey[j + 1];
            if (c >= '0' && c <= '9')
                c -= '0';
            else if (c >= 'A' && c <= 'F')
                c -= 'A' - 10;
            else if (c >= 'a' && c <= 'f')
                c -= 'a' - 10;
            else
                return -1;
            psk[j / 2] |= c;
        }
        // set mbedtls config
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: set mbedtls config");
        ret = mbedtls_ssl_conf_psk(&ssl_client->ssl_conf, psk, psk_len,
                                   (const unsigned char *)pskIdent, strlen(pskIdent));
        if (ret != 0)
        {
            if (ssl_client->_debugCallback)
            {
                char *error_buf = new char[100];
                memset(buf, 0, 512);
                strcpy(buf, "ERROR: ");
                mbedtls_strerror(ret, error_buf, 100);
                strcat(buf, error_buf);
                ssl_client->_debugCallback(buf);
                delete[] error_buf;
            }
            log_e("mbedtls_ssl_conf_psk returned %d", ret);
            return handle_error(ret);
        }
    }
    else
    {

        mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
        log_i("WARNING: Use certificates for a more secure communication!");
    }

    if (cli_cert != NULL && cli_key != NULL)
    {

        mbedtls_x509_crt_init(&ssl_client->client_cert);
        mbedtls_pk_init(&ssl_client->client_key);

        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: loading CRT cert");

        log_v("Loading CRT cert");

        ret = mbedtls_x509_crt_parse(&ssl_client->client_cert, (const unsigned char *)cli_cert, strlen(cli_cert) + 1);
        if (ret < 0)
        {
            if (ssl_client->_debugCallback)
            {
                char *error_buf = new char[100];
                memset(buf, 0, 512);
                strcpy(buf, "ERROR: ");
                mbedtls_strerror(ret, error_buf, 100);
                strcat(buf, error_buf);
                ssl_client->_debugCallback(buf);
                delete[] error_buf;
            }
            return handle_error(ret);
        }

        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: loading private key");

        log_v("Loading private key");
        ret = mbedtls_pk_parse_key(&ssl_client->client_key, (const unsigned char *)cli_key, strlen(cli_key) + 1, NULL, 0);

        if (ret != 0)
        {
            return handle_error(ret);
        }

        mbedtls_ssl_conf_own_cert(&ssl_client->ssl_conf, &ssl_client->client_cert, &ssl_client->client_key);
    }

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: setting hostname for TLS session...");

    log_v("Setting hostname for TLS session...");

    // Hostname set here should match CN in server certificate
    if ((ret = mbedtls_ssl_set_hostname(&ssl_client->ssl_ctx, host)) != 0)
    {
        if (ssl_client->_debugCallback)
        {
            char *error_buf = new char[100];
            memset(buf, 0, 512);
            strcpy(buf, "ERROR: ");
            mbedtls_strerror(ret, error_buf, 100);
            strcat(buf, error_buf);
            ssl_client->_debugCallback(buf);
            delete[] error_buf;
        }
        return handle_error(ret);
    }

    mbedtls_ssl_conf_rng(&ssl_client->ssl_conf, mbedtls_ctr_drbg_random, &ssl_client->drbg_ctx);

    if ((ret = mbedtls_ssl_setup(&ssl_client->ssl_ctx, &ssl_client->ssl_conf)) != 0)
    {
        if (ssl_client->_debugCallback)
        {
            char *error_buf = new char[100];
            memset(buf, 0, 512);
            strcpy(buf, "ERROR: ");
            mbedtls_strerror(ret, error_buf, 100);
            strcat(buf, error_buf);
            ssl_client->_debugCallback(buf);
            delete[] error_buf;
        }

        return handle_error(ret);
    }

    mbedtls_ssl_set_bio(&ssl_client->ssl_ctx, &ssl_client->socket, mbedtls_net_send, mbedtls_net_recv, NULL);

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: performing the SSL/TLS handshake...");

    log_v("Performing the SSL/TLS handshake...");
    unsigned long handshake_start_time = millis();
    while ((ret = mbedtls_ssl_handshake(&ssl_client->ssl_ctx)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            if (ssl_client->_debugCallback)
            {
                char *error_buf = new char[100];
                memset(buf, 0, 512);
                strcpy(buf, "ERROR: ");
                mbedtls_strerror(ret, error_buf, 100);
                strcat(buf, error_buf);
                ssl_client->_debugCallback(buf);
                delete[] error_buf;
            }
            return handle_error(ret);
        }
        if ((millis() - handshake_start_time) > ssl_client->handshake_timeout)
            return -1;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (cli_cert != NULL && cli_key != NULL)
    {
        log_d("Protocol is %s Ciphersuite is %s", mbedtls_ssl_get_version(&ssl_client->ssl_ctx), mbedtls_ssl_get_ciphersuite(&ssl_client->ssl_ctx));
        if ((ret = mbedtls_ssl_get_record_expansion(&ssl_client->ssl_ctx)) >= 0)
        {

            log_d("Record expansion is %d", ret);
        }
        else
        {
            log_w("Record expansion is unknown (compression)");
        }
    }

    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: verifying peer X.509 certificate...");

    log_v("Verifying peer X.509 certificate...");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl_client->ssl_ctx)) != 0)
    {
        bzero(buf, sizeof(buf));
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: failed to verify peer certificate!");
        log_e("Failed to verify peer certificate! verification info: %s", buf);
        stop_ssl_socket(ssl_client, rootCABuff, cli_cert, cli_key); //It's not safe continue.
        return handle_error(ret);
    }
    else
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: certificate verified");
        log_v("Certificate verified.");
    }

    if (rootCABuff != NULL)
    {
        mbedtls_x509_crt_free(&ssl_client->ca_cert);
    }

    if (cli_cert != NULL)
    {
        mbedtls_x509_crt_free(&ssl_client->client_cert);
    }

    if (cli_key != NULL)
    {
        mbedtls_pk_free(&ssl_client->client_key);
    }

    log_v("Free internal heap after TLS %u", ESP.getFreeHeap());

    return ssl_client->socket;
}

void stop_ssl_socket(sslclient_context32 *ssl_client, const char *rootCABuff, const char *cli_cert, const char *cli_key)
{
    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: cleaning SSL connection");
    log_v("Cleaning SSL connection.");

    if (ssl_client->socket >= 0)
    {
        close(ssl_client->socket);
        ssl_client->socket = -1;
    }

    mbedtls_ssl_free(&ssl_client->ssl_ctx);
    mbedtls_ssl_config_free(&ssl_client->ssl_conf);
    mbedtls_ctr_drbg_free(&ssl_client->drbg_ctx);
    mbedtls_entropy_free(&ssl_client->entropy_ctx);
}

int data_to_read(sslclient_context32 *ssl_client)
{
    int ret, res;
    ret = mbedtls_ssl_read(&ssl_client->ssl_ctx, NULL, 0);
    //log_e("RET: %i",ret);   //for low level debug
    res = mbedtls_ssl_get_bytes_avail(&ssl_client->ssl_ctx);
    //log_e("RES: %i",res);    //for low level debug
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret < 0)
    {
        if (ssl_client->_debugCallback)
        {
            char *buf = new char[512];
            char *error_buf = new char[100];
            memset(buf, 0, 512);
            strcpy(buf, "ERROR: ");
            mbedtls_strerror(ret, error_buf, 100);
            strcat(buf, error_buf);
            ssl_client->_debugCallback(buf);
            delete[] error_buf;
            delete[] buf;
        }
        return handle_error(ret);
    }

    return res;
}

int send_ssl_data(sslclient_context32 *ssl_client, const uint8_t *data, uint16_t len)
{

    log_v("Writing HTTP request..."); //for low level debug
    int ret = -1;

    while ((ret = mbedtls_ssl_write(&ssl_client->ssl_ctx, data, len)) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            return handle_error(ret);
        }
    }

    len = ret;
    //log_v("%d bytes written", len);  //for low level debug
    return ret;
}

int get_ssl_receive(sslclient_context32 *ssl_client, uint8_t *data, int length)
{

    //log_d( "Reading HTTP response...");   //for low level debug
    int ret = -1;

    ret = mbedtls_ssl_read(&ssl_client->ssl_ctx, data, length);

    //log_v( "%d bytes read", ret);   //for low level debug
    return ret;
}

static bool parseHexNibble(char pb, uint8_t *res)
{
    if (pb >= '0' && pb <= '9')
    {
        *res = (uint8_t)(pb - '0');
        return true;
    }
    else if (pb >= 'a' && pb <= 'f')
    {
        *res = (uint8_t)(pb - 'a' + 10);
        return true;
    }
    else if (pb >= 'A' && pb <= 'F')
    {
        *res = (uint8_t)(pb - 'A' + 10);
        return true;
    }
    return false;
}

// Compare a name from certificate and domain name, return true if they match
static bool matchName(const std::string &name, const std::string &domainName)
{
    size_t wildcardPos = name.find('*');
    if (wildcardPos == std::string::npos)
    {
        // Not a wildcard, expect an exact match
        return name == domainName;
    }

    size_t firstDotPos = name.find('.');
    if (wildcardPos > firstDotPos)
    {
        // Wildcard is not part of leftmost component of domain name
        // Do not attempt to match (rfc6125 6.4.3.1)
        return false;
    }
    if (wildcardPos != 0 || firstDotPos != 1)
    {
        // Matching of wildcards such as baz*.example.com and b*z.example.com
        // is optional. Maybe implement this in the future?
        return false;
    }
    size_t domainNameFirstDotPos = domainName.find('.');
    if (domainNameFirstDotPos == std::string::npos)
    {
        return false;
    }
    return domainName.substr(domainNameFirstDotPos) == name.substr(firstDotPos);
}

// Verifies certificate provided by the peer to match specified SHA256 fingerprint
bool verify_ssl_fingerprint(sslclient_context32 *ssl_client, const char *fp, const char *domain_name)
{
    // Convert hex string to byte array
    uint8_t fingerprint_local[32];
    int len = strlen(fp);
    int pos = 0;
    for (size_t i = 0; i < sizeof(fingerprint_local); ++i)
    {
        while (pos < len && ((fp[pos] == ' ') || (fp[pos] == ':')))
        {
            ++pos;
        }
        if (pos > len - 2)
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: fingerprint too short");
            log_d("pos:%d len:%d fingerprint too short", pos, len);
            return false;
        }
        uint8_t high, low;
        if (!parseHexNibble(fp[pos], &high) || !parseHexNibble(fp[pos + 1], &low))
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: invalid hex sequence");
            log_d("pos:%d len:%d invalid hex sequence: %c%c", pos, len, fp[pos], fp[pos + 1]);
            return false;
        }
        pos += 2;
        fingerprint_local[i] = low | (high << 4);
    }

    // Get certificate provided by the peer
    const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&ssl_client->ssl_ctx);

    if (!crt)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: could not fetch peer certificate");
        log_d("could not fetch peer certificate");
        return false;
    }

    // Calculate certificate's SHA256 fingerprint
    uint8_t fingerprint_remote[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, false);
    mbedtls_sha256_update(&sha256_ctx, crt->raw.p, crt->raw.len);
    mbedtls_sha256_finish(&sha256_ctx, fingerprint_remote);

    // Check if fingerprints match
    if (memcmp(fingerprint_local, fingerprint_remote, 32))
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: fingerprint doesn't match");
        log_d("fingerprint doesn't match");
        return false;
    }

    // Additionally check if certificate has domain name if provided
    if (domain_name)
        return verify_ssl_dn(ssl_client, domain_name);
    else
        return true;
}

// Checks if peer certificate has specified domain in CN or SANs
bool verify_ssl_dn(sslclient_context32 *ssl_client, const char *domain_name)
{
    log_d("domain name: '%s'", (domain_name) ? domain_name : "(null)");
    std::string domain_name_str(domain_name);
    std::transform(domain_name_str.begin(), domain_name_str.end(), domain_name_str.begin(), ::tolower);

    // Get certificate provided by the peer
    const mbedtls_x509_crt *crt = mbedtls_ssl_get_peer_cert(&ssl_client->ssl_ctx);

    // Check for domain name in SANs
    const mbedtls_x509_sequence *san = &crt->subject_alt_names;
    while (san != nullptr)
    {
        std::string san_str((const char *)san->buf.p, san->buf.len);
        std::transform(san_str.begin(), san_str.end(), san_str.begin(), ::tolower);

        if (matchName(san_str, domain_name_str))
            return true;

        log_d("SAN '%s': no match", san_str.c_str());

        // Fetch next SAN
        san = san->next;
    }

    // Check for domain name in CN
    const mbedtls_asn1_named_data *common_name = &crt->subject;
    while (common_name != nullptr)
    {
        // While iterating through DN objects, check for CN object
        if (!MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &common_name->oid))
        {
            std::string common_name_str((const char *)common_name->val.p, common_name->val.len);

            if (matchName(common_name_str, domain_name_str))
                return true;

            log_d("CN '%s': not match", common_name_str.c_str());
        }

        // Fetch next DN object
        common_name = common_name->next;
    }

    return false;
}

int starttlsHandshake(sslclient_context32 *ssl_client, int port)
{

    int ret = 0;
    size_t msgLen = 100;
    size_t bufLen = 512;
    char *buf = new char[bufLen];
    char *hMsg = new char[msgLen];

    fd_set readset;
    fd_set writeset;
    fd_set errset;

    struct timeval tv;

    FD_ZERO(&readset);
    FD_SET(ssl_client->socket, &readset);
    FD_ZERO(&writeset);
    FD_SET(ssl_client->socket, &writeset);

    FD_ZERO(&errset);
    FD_SET(ssl_client->socket, &errset);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    ret = lwip_select(ssl_client->socket, &readset, &writeset, &errset, &tv);
    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: waiting incoming data failed!");

        goto starttls_exit;
    }

    ret = read(ssl_client->socket, buf, bufLen);

    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: reading incoming data failed!");
        goto starttls_exit;
    }
    else
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback(buf);
    }

    if (port == 587 || port == 25)
    {

        memset(hMsg, 0, msgLen);
        strcpy(hMsg, "EHLO DUDE\r\n");
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("INFO: send SMTP command extended HELO");
        ret = lwip_write(ssl_client->socket, hMsg, strlen(hMsg));

        if (ret < 0)
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: send SMTP command failed!");
            goto starttls_exit;
        }

        ret = lwip_select(ssl_client->socket, &readset, &writeset, &errset, &tv);

        if (ret < 0)
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: waiting incoming data failed!");
            goto starttls_exit;
        }

        memset(buf, 0, bufLen);
        ret = lwip_read(ssl_client->socket, buf, bufLen);

        if (ret < 0)
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback("ERROR: reading incoming data failed!");
            goto starttls_exit;
        }
        else
        {
            if (ssl_client->_debugCallback)
                ssl_client->_debugCallback(buf);
        }
    }

    memset(hMsg, 0, msgLen);
    strcpy(hMsg, "STARTTLS\r\n");
    if (ssl_client->_debugCallback)
        ssl_client->_debugCallback("INFO: send STARTTLS protocol command");
    ret = lwip_write(ssl_client->socket, hMsg, strlen(hMsg));

    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: send STARTTLS protocol command failed!");
        goto starttls_exit;
    }

    ret = lwip_select(ssl_client->socket, &readset, &writeset, &errset, &tv);

    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: waiting incoming data failed!");
        goto starttls_exit;
    }

    memset(buf, 0, bufLen);
    ret = lwip_read(ssl_client->socket, buf, bufLen);

    if (ret < 0)
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback("ERROR: reading incoming data failed!");
        goto starttls_exit;
    }
    else
    {
        if (ssl_client->_debugCallback)
            ssl_client->_debugCallback(buf);
    }

    delete[] buf;
    delete[] hMsg;

    return 0;

starttls_exit:

    delete[] buf;
    delete[] hMsg;

    return -1;
}
