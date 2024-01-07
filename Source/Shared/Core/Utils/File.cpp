/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Logging.h"

#include <fstream>
#include <sstream>

bool ReadTextFromFile(const std::filesystem::path& path, std::string& Output)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        Error("Failed to read from file: %s", path.string().c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Output = buffer.str();

    file.close();

    return true;
}

bool WriteTextToFile(const std::filesystem::path& path, const std::string& Input)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
    {
        Error("Failed to write to file: %s", path.string().c_str());
        return false;
    }

    file << Input;

    file.close();

    return true;
}

bool ReadBytesFromFile(const std::filesystem::path& path, std::vector<uint8_t>& Output)
{
    FILE* file = fopen(path.string().c_str(), "rb");
    if (!file)
    {
        Error("Failed to read from file: %s", path.string().c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    Output.resize(ftell(file));
    fseek(file, 0, SEEK_SET);

    size_t BytesRead = 0;
    while (BytesRead < Output.size())
    {
        BytesRead += fread((char*)Output.data() + BytesRead, 1, Output.size() - BytesRead, file);
    }

    fclose(file);

    return true;
}

bool WriteBytesToFile(const std::filesystem::path& path, const std::vector<uint8_t>& Input)
{
    FILE* file = fopen(path.string().c_str(), "wb");
    if (!file)
    {
        Error("Failed to write to file: %s", path.string().c_str());
        return false;
    }

    size_t BytesWritten = 0;
    while (BytesWritten < Input.size())
    {
        BytesWritten += fwrite((char*)Input.data() + BytesWritten, 1, Input.size() - BytesWritten, file);
    }

    fclose(file);

    return true;
}