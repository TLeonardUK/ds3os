/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

 // The version of protobuf we have to use to support DS3 doesn't generate
 // code that compiles without warnings under x64 (lots of size_t truncation).
 // To keep things a bit cleaner we import all the files through this header and cpp.
#pragma warning(disable: 4267 4244 4018)

#include "Frpg2RequestMessage.pb.cc"

#pragma warning(default: 4267 4244 4018)
