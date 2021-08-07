// Dark Souls 3 - Open Server

#include "Core/Utils/File.h"

#include <fstream>

bool ReadTextFromFile(const std::filesystem::path& path, std::string& Output)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
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
        return false;
    }

    file << Input;

    file.close();

    return true;
}

bool ReadBytesFromFile(const std::filesystem::path& path, std::vector<uint8_t>& Output)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    Output.resize(file.tellg());
    file.read((char*)Output.data(), Output.size());

    return true;
}

bool WriteBytesToFile(const std::filesystem::path& path, const std::vector<uint8_t>& Input)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
    {
        return false;
    }

    file.write((const char*)Input.data(), Input.size());
    file.close();

    return true;
}