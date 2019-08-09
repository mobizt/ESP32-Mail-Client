/*
 * Customized version of ESP32 HTTPClient Library. 
 * Allow custom header and payload with STARTTLS support
 * 
 * v 1.1.2
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * HTTPClient Arduino library for ESP32
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the HTTPClient for Arduino.
 * Port to ESP32 by Evandro Luis Copercini (2017), 
 * changed fingerprints to CA verification. 	
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef HTTPClientESP32Ex_CPP
#define HTTPClientESP32Ex_CPP

#include "HTTPClientESP32Ex.h"

class TransportTraits
{
public:
    virtual ~TransportTraits() {}

    virtual std::unique_ptr<WiFiClient> create()
    {
        return std::unique_ptr<WiFiClient>(new WiFiClient());
    }

    virtual bool
    verify(WiFiClient &client, const char *host, bool starttls, DebugMsgCallback cb)
    {
        return true;
    }
};

class TLSTraits : public TransportTraits
{
public:
    TLSTraits(const char *CAcert, const char *clicert = nullptr, const char *clikey = nullptr) : _cacert(CAcert), _clicert(clicert), _clikey(clikey) {}

    std::unique_ptr<WiFiClient> create() override
    {
        return std::unique_ptr<WiFiClient>(new WiFiClientSecureESP32());
    }

    bool verify(WiFiClient &client, const char *host, bool starttls, DebugMsgCallback cb) override
    {
        WiFiClientSecureESP32 &wcs = static_cast<WiFiClientSecureESP32 &>(client);
        wcs.setCACert(_cacert);
        wcs.setCertificate(_clicert);
        wcs.setPrivateKey(_clikey);
        wcs.setSTARTTLS(starttls);
        wcs.setDebugCB(cb);
        return true;
    }

protected:
    const char *_cacert;
    const char *_clicert;
    const char *_clikey;
};

HTTPClientESP32Ex::HTTPClientESP32Ex() {}

HTTPClientESP32Ex::~HTTPClientESP32Ex()
{
    if (_tcp)
        _tcp->stop();
}

bool HTTPClientESP32Ex::http_begin(const char *host, uint16_t port, const char *uri, const char *CAcert)
{
    http_transportTraits.reset(nullptr);

    _host = host;
    _port = port;
    _uri = uri;
    http_transportTraits = TransportTraitsPtr(new TLSTraits(CAcert));
    return true;
}

bool HTTPClientESP32Ex::http_connected()
{
    if (_tcp)
        return ((_tcp->available() > 0) || _tcp->connected());
    return false;
}

bool HTTPClientESP32Ex::http_sendHeader(const char *header)
{
    if (!http_connected())
        return false;
    return (_tcp->write(header, strlen(header)) == strlen(header));
}

int HTTPClientESP32Ex::http_sendRequest(const char *header, const char *payload)
{
    Serial.print(header);
    Serial.print(payload);
    size_t size = strlen(payload);
    if (strlen(header) > 0)
    {
        if (!http_connect())
            return HTTPC_ERROR_CONNECTION_REFUSED;
        if (!http_sendHeader(header))
            return HTTPC_ERROR_SEND_HEADER_FAILED;
    }
    if (size > 0)
        if (_tcp->write(&payload[0], size) != size)
            return HTTPC_ERROR_SEND_PAYLOAD_FAILED;

    return 0;
}

WiFiClient *HTTPClientESP32Ex::http_getStreamPtr(void)
{
    if (http_connected())
        return _tcp.get();
    return nullptr;
}

bool HTTPClientESP32Ex::http_connect(void)
{
    if (http_connected())
    {
        while (_tcp->available() > 0)
            _tcp->read();
        return true;
    }

    if (!http_transportTraits)
        return false;

    _tcp = http_transportTraits->create();

    if (!http_transportTraits->verify(*_tcp, _host.c_str(), false, _debugCallback))
    {
        _tcp->stop();
        return false;
    }

    if (!_tcp->connect(_host.c_str(), _port))
        return false;

    return http_connected();
}

bool HTTPClientESP32Ex::http_connect(bool starttls)
{
    if (http_connected())
    {
        while (_tcp->available() > 0)
            _tcp->read();
        return true;
    }

    if (!http_transportTraits)
        return false;

    _tcp = http_transportTraits->create();

    if (!http_transportTraits->verify(*_tcp, _host.c_str(), starttls, _debugCallback))
    {
        _tcp->stop();
        return false;
    }

    if (!_tcp->connect(_host.c_str(), _port))
        return false;

    return http_connected();
}

void HTTPClientESP32Ex::setDebugCallback(DebugMsgCallback cb)
{
    _debugCallback = std::move(cb);
}
#endif
