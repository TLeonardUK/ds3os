// Dark Souls 3 - Open Server

#pragma once

#include <string>

// Base class for all network services
// that the server wants to make available.

class Service
{
public:
    virtual ~Service() {};

    virtual bool Init() = 0;
    virtual bool Term() = 0;
    virtual void Poll() = 0;

    virtual std::string GetName() = 0;

};