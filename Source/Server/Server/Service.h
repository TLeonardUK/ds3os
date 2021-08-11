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