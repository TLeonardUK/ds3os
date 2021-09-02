/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Network/NetHttpRequest.h"
#include "Core/Utils/Logging.h"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

NetHttpRequest::~NetHttpRequest()
{
    Response = nullptr;
    if (Handle)
    {
        FinishRequest();
    }
}

std::vector<uint8_t> NetHttpResponse::GetBody()
{
    return Body;
}

bool NetHttpResponse::GetWasSuccess()
{
    return WasSuccess;
}

void NetHttpRequest::SetUrl(const std::string& InPath)
{
    Url = InPath;
}

void NetHttpRequest::SetMethod(NetHttpMethod InMethod)
{
    Method = InMethod;
}

void NetHttpRequest::SetBody(const std::vector<uint8_t>& InBody)
{
    Body = InBody;
}

size_t NetHttpRequest::RecieveBodyFunction(void* ptr, size_t size, size_t nmemb, NetHttpResponse* Response)
{
    size_t Offset = Response->Body.size();
    Response->Body.resize(Response->Body.size() + nmemb);
    memcpy(Response->Body.data() + Offset, ptr, nmemb);

    return nmemb;
}

bool NetHttpRequest::StartRequest()
{
    Handle = curl_easy_init();
    HandleMulti = curl_multi_init();
    Response = std::make_shared<NetHttpResponse>();

    curl_easy_setopt(Handle, CURLOPT_URL, Url.c_str());
    curl_easy_setopt(Handle, CURLOPT_WRITEFUNCTION, RecieveBodyFunction);
    curl_easy_setopt(Handle, CURLOPT_WRITEDATA, Response.get());

    switch (Method)
    {
        case NetHttpMethod::OPTIONS:            curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");     break;
        case NetHttpMethod::GET:                curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "GET");         break;
        case NetHttpMethod::HEAD:               curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "HEAD");        break;
        case NetHttpMethod::POST:               curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "POST");        break;
        case NetHttpMethod::PUT:                curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "PUT");         break;
        case NetHttpMethod::METHOD_DELETE:      curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "DELETE");      break;
        case NetHttpMethod::TRACE:              curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "TRACE");       break;
        case NetHttpMethod::CONNECT:            curl_easy_setopt(Handle, CURLOPT_CUSTOMREQUEST, "CONNECT");     break;
    }

    if (Body.size() > 0)
    {
        curl_easy_setopt(Handle, CURLOPT_POSTFIELDS, Body.data());
        curl_easy_setopt(Handle, CURLOPT_POSTFIELDSIZE, Body.size());
    }

    curl_multi_add_handle(HandleMulti, Handle);

    return true;
}

void NetHttpRequest::PollRequest()
{
    bool Complete = false;

    int ActiveHandles = 0;
    CURLMcode Result = curl_multi_perform(HandleMulti, &ActiveHandles);
    if (Result != CURLE_OK || ActiveHandles == 0)
    {
        Complete = true;
        Response->WasSuccess = (Result == CURLE_OK);
    }

    if (Complete)
    {
        FinishRequest();
    }
}

bool NetHttpRequest::FinishRequest()
{
    curl_multi_remove_handle(HandleMulti, Handle);

    curl_easy_cleanup(Handle);
    Handle = nullptr;

    curl_multi_cleanup(HandleMulti);
    HandleMulti = nullptr;

    return true;
}

bool NetHttpRequest::Send()
{
    Ensure(!InProgress());

    if (!StartRequest())
    {
        return false;
    }
    while (InProgress())
    {
        PollRequest();
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

bool NetHttpRequest::SendAsync()
{
    Ensure(!InProgress());
    if (!StartRequest())
    {
        return false;
    }
    return true;
}

bool NetHttpRequest::InProgress()
{
    if (Handle)
    {
        PollRequest();
    }
    return Handle != nullptr;
}

std::shared_ptr<NetHttpResponse> NetHttpRequest::GetResponse()
{
    Ensure(!InProgress());
    return Response;
}
