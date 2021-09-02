/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <memory>
#include <Vector>
#include <curl/curl.h>

// Simple class for sending HTTP requests

enum class NetHttpMethod
{
    OPTIONS,
    GET,
    HEAD,
    POST,
    PUT,
    METHOD_DELETE,
    TRACE,
    CONNECT
};

class NetHttpResponse
{
public:
    std::vector<uint8_t> GetBody();
    bool GetWasSuccess();

private:
    friend class NetHttpRequest;

    std::vector<uint8_t> Body;
    bool WasSuccess = false;

};

class NetHttpRequest
{
public:
    ~NetHttpRequest();

    void SetUrl(const std::string& Path);
    void SetMethod(NetHttpMethod Method);
    void SetBody(const std::vector<uint8_t>& Body);

    bool Send();
    bool SendAsync();

    bool InProgress();

    std::shared_ptr<NetHttpResponse> GetResponse();

protected:
    bool StartRequest();
    void PollRequest();
    bool FinishRequest();

    static size_t RecieveBodyFunction(void* ptr, size_t size, size_t nmemb, NetHttpResponse* Response);

private:
    std::vector<uint8_t> Body;
    NetHttpMethod Method = NetHttpMethod::GET;
    std::string Url = "/";
    std::shared_ptr<NetHttpResponse> Response;

    CURL* Handle;
    CURLM* HandleMulti;

};