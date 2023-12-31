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
#include <vector>
#include <memory>
#include <variant>
#include <mutex>

#include <google/protobuf/message_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite.h>

// Type of value decoded for a field.
enum class DecodedProtobufValueType
{
    int64,
    fixed64,
    fixed32,
    _string,
    bytes,
    protobuf,

    COUNT
};

// A value seen for a given field.
class DecodedProtobufValue
{
public:
    using Value_t = std::variant<uint32_t, uint64_t, std::string>;

    DecodedProtobufValueType Type = DecodedProtobufValueType::int64;
    Value_t Value;

public:
    std::string ToString();

};

// A field within a decoded message.
class DecodedProtobufField
{
public:
    int FieldNumber = 0;

    std::string Name;
    bool Repeated = false;
    bool Required = true;
    bool Group = false;
    DecodedProtobufField* GroupParent = nullptr;

    bool SeenThisParse = false;

    std::vector<std::unique_ptr<DecodedProtobufValue>> Values;

public:
    void AddValue(DecodedProtobufValueType type, DecodedProtobufValue::Value_t value);

    std::string GetTypeString();

};

// An individual decoded message.
class DecodedProtobufMessage
{
public:
    std::string Name;
    std::vector<std::unique_ptr<DecodedProtobufField>> Fields;

public:
    DecodedProtobufField* FindOrCreateField(const std::string& fieldName);
    std::string ToString();

    std::string FieldToString(DecodedProtobufField* Field, int NestLevel);
};

// Holds a set of messages that have been decoded.
class DecodedProtobufRegistry
{
public:
    const DecodedProtobufMessage* Decode(const std::string& messageName, const uint8_t* data, size_t length);

    std::string ToString();

private:
    DecodedProtobufMessage* FindMessage(const std::string& messageName);

    bool DecodeField(DecodedProtobufMessage* Message, google::protobuf::io::CodedInputStream& input, int tag, std::vector<DecodedProtobufField*>& GroupStack);

private:
    std::recursive_mutex Mutex;
    std::vector<std::unique_ptr<DecodedProtobufMessage>> Messages;
};