/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <vector>
#include <string>

// Abstract class that just holds various build-time 
// configuration variables. Might be worth dumping this
// into a json file at some point in future so it can 
// be changed.
class BuildConfig
{
public:
    
    BuildConfig() = delete;

    // Turns off redirectly to custom server, goes to retail instead.
    // (WARNING: This may get you banned, don't touch unless you know what you are doing).
    inline static const bool DO_NOT_REDIRECT = false;

    // Writes out all protobufs to a temporary folder
    inline static const bool WRITE_OUT_PROTOBUFS = false;

#ifdef _DEBUG
    // Writes out all the decoded format of all protobufs encountered so far.
    inline static const bool WRITE_OUT_DECODED_PROTOBUFS = true;
#else
    // Writes out all the decoded format of all protobufs encountered so far.
    inline static const bool WRITE_OUT_DECODED_PROTOBUFS = false;
#endif

    // Writes out all protobufs to a temporary folder
    inline static const char* TEMP_LOG_FOLDER = "D:\\Temp\\";

};