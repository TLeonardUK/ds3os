/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/Protobuf.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/Logging.h"

#include <google/protobuf/message_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite.h>

#include <vector>
#include <string>
#include <array>

using namespace google;
using namespace google::protobuf;
using namespace google::protobuf::internal;

void DecodedProtobufField::AddValue(DecodedProtobufValueType type, DecodedProtobufValue::Value_t value)
{
    for (auto& Value : Values)
    {
        if (Value->Type == type && Value->Value == value)
        {
            return;
        }
    }

    std::unique_ptr<DecodedProtobufValue> NewValue = std::make_unique<DecodedProtobufValue>();
    NewValue->Type = type;
    NewValue->Value = value;
    Values.push_back(std::move(NewValue));
}

DecodedProtobufField* DecodedProtobufMessage::FindOrCreateField(const std::string& fieldName)
{
    for (auto& Field : Fields)
    {
        if (Field->Name == fieldName)
        {
            return Field.get();
        }
    }

    std::unique_ptr<DecodedProtobufField> field = std::make_unique<DecodedProtobufField>();
    field->Name = fieldName;

    DecodedProtobufField* result = field.get();

    Fields.push_back(std::move(field));

    return result; 
}

bool DecodedProtobufRegistry::DecodeField(DecodedProtobufMessage* Message, google::protobuf::io::CodedInputStream& input, int tag, std::vector<DecodedProtobufField*>& GroupStack)
{
    int fieldNumber = WireFormatLite::GetTagFieldNumber(tag);
    std::string fieldName = StringFormat("field_%i", fieldNumber);
    if (!GroupStack.empty())
    {
        fieldName = StringFormat("group_field_%i", fieldNumber);
    }

    int wireType = WireFormatLite::GetTagWireType(tag);

    DecodedProtobufField* Field = nullptr;
    if (wireType != WireFormatLite::WIRETYPE_END_GROUP)
    {
        Field = Message->FindOrCreateField(fieldName);
        Field->FieldNumber = fieldNumber;
        Field->Repeated = Field->Repeated || Field->SeenThisParse;
        Field->SeenThisParse = true;

        if (!GroupStack.empty())
        {
            // Don't set to parent if decoding a repeated field.
            if (GroupStack.back() != Field)
            {
                Field->GroupParent = GroupStack.back();
            }
        }
    }

    switch (wireType) 
    {
    case WireFormatLite::WIRETYPE_VARINT: 
        {
            uint64 value;
            if (!input.ReadVarint64(&value)) return false;
            Field->AddValue(DecodedProtobufValueType::int64, value);
            return true;
        }
    case WireFormatLite::WIRETYPE_FIXED64: 
        {
            uint64 value;
            if (!input.ReadLittleEndian64(&value)) return false;
            Field->AddValue(DecodedProtobufValueType::fixed64, value);
            return true;
        }
    case WireFormatLite::WIRETYPE_LENGTH_DELIMITED: 
        {
            uint32 length;
            if (!input.ReadVarint32(&length)) return false;
            string temp;
            if (!input.ReadString(&temp, length)) return false;

            // If human readable text, assume its a string.
            if (StringIsHumanReadable(temp))
            {
                Field->AddValue(DecodedProtobufValueType::_string, temp);
            }
            else
            {
                // Atteempt to parse as an embedded protobuf, if not successfuly assume a bytes array.
                std::string SubMessageName = StringFormat("%s_field_%i", Message->Name.c_str(), fieldNumber);
                const DecodedProtobufMessage* Message = Decode(SubMessageName, (uint8_t*)temp.data(), temp.size());
                if (Message)
                {
                    Field->AddValue(DecodedProtobufValueType::protobuf, SubMessageName);
                }
                else
                {
                    Field->AddValue(DecodedProtobufValueType::bytes, temp);
                }
            }

            return true;
        }
    case WireFormatLite::WIRETYPE_START_GROUP: 
        {
            Field->Group = true;
            Field->Repeated = true; // Assumption is that groups are repeated.
            GroupStack.push_back(Field);
            return true;
        }
    case WireFormatLite::WIRETYPE_END_GROUP: 
        {
            if (GroupStack.empty())
            {
                return false;
            }
            GroupStack.pop_back();
            return true;
        }
    case WireFormatLite::WIRETYPE_FIXED32: 
        {
            uint32 value;
            if (!input.ReadLittleEndian32(&value)) return false;
            Field->AddValue(DecodedProtobufValueType::fixed32, value);
            return true;
        }
    default: 
        {
            return false;
        }
    }

    return false;
}

const DecodedProtobufMessage* DecodedProtobufRegistry::Decode(const std::string& messageName, const uint8_t* data, size_t length)
{
    std::scoped_lock mutex(Mutex);

    std::unique_ptr<DecodedProtobufMessage> NewMessage = nullptr;
    DecodedProtobufMessage* Message = FindMessage(messageName);
    if (Message == nullptr)
    {
        NewMessage = std::make_unique<DecodedProtobufMessage>();
        NewMessage->Name = messageName;
        Message = NewMessage.get();        
    }
    
    // Mark all fields as not seen this parse so we can determine which repeat/are-optional.
    for (auto& Field : Message->Fields)
    {
        Field->SeenThisParse = false;
    }

    // Parse each field until we get to the end of the data.
    std::vector<DecodedProtobufField*> GroupStack;

    google::protobuf::io::CodedInputStream input(data, length);
    while (true)
    {
        std::pair<google::protobuf::uint32, bool> p = input.ReadTagWithCutoff(127);
        google::protobuf::uint32 tag = p.first;

        if (!p.second || p.first == 0)
        {
            break;
        }

        if (!DecodeField(Message, input, tag, GroupStack))
        {
            return nullptr;
        }
    }

    // Mark any fields we didn't visit this parase as optional.
    for (auto& Field : Message->Fields)
    {
        if (!Field->SeenThisParse)
        {
            Field->Required = false;
        }
    }

    if (NewMessage)
    {
        Messages.push_back(std::move(NewMessage));
    }
    return Message;
}

DecodedProtobufMessage* DecodedProtobufRegistry::FindMessage(const std::string& messageName)
{
    for (auto& Message : Messages)
    {
        if (Message->Name == messageName)
        {
            return Message.get();
        }
    }
    return nullptr;
}

std::string DecodedProtobufRegistry::ToString()
{
    std::string Result = "";
    for (auto& Message : Messages)
    {
        Result += Message->ToString();
        Result += "\n\n";
    }
    return Result;
}

std::string DecodedProtobufMessage::FieldToString(DecodedProtobufField* Field, int NestLevel)
{
    std::string Result = "";
    std::string NestTabs(NestLevel, '\t');

    // Expected format:
    // optional string field_1 = 1; // value, value, value

    Result += NestTabs;
    if (Field->Repeated)
    {
        Result += "repeated ";
    }
    if (Field->Group)
    {
        Result += "group ";
    }
    if (!Field->Group && !Field->Repeated)
    {
        if (Field->Required)
        {
            Result += "required ";
        }
        else
        {
            Result += "optional ";
        }
    }

    if (Field->Group)
    {
        Result += StringFormat("%s = %i {\n", Field->Name.c_str(), Field->FieldNumber);

        for (auto& GroupField : Fields)
        {
            if (GroupField->GroupParent != Field)
            {
                continue;
            }
            Result += FieldToString(GroupField.get(), NestLevel + 1);
        }

        Result += NestTabs + "}";
    }
    else
    {
        Result += Field->GetTypeString();
        Result += StringFormat(" %s = %i; // ", Field->Name.c_str(), Field->FieldNumber);

        // Append all potential values.
        bool FirstValue = true;
        for (auto& Value : Field->Values)
        {
            std::string ValueStr = Value->ToString();
            if (ValueStr.empty())
            {
                continue;
            }

            if (!FirstValue)
            {
                Result += ", ";
            }
            FirstValue = false;

            Result += ValueStr;
        }
    }

    Result += "\n";

    return Result;
}

std::string DecodedProtobufMessage::ToString()
{
    std::string Result = StringFormat("message %s {\n", Name.c_str());

    for (auto& Field : Fields)
    {
        if (Field->GroupParent != nullptr)
        {
            continue;
        }

        Result += FieldToString(Field.get(), 1);
    }

    Result += StringFormat("}\n");

    return Result;
}

std::string DecodedProtobufField::GetTypeString()
{
    // Go through values and try and determine the most common type. 
    // This should generally be fixed but may be incorrect in cases where protobuf/etc
    // data is incorrectly identified.
    std::array<int, (int)DecodedProtobufValueType::COUNT> ValuesOfType = {};

    for (auto& Value : Values)
    {
        ValuesOfType[(int)Value->Type]++;
    }

    auto iter = std::max_element(ValuesOfType.begin(), ValuesOfType.end());
    DecodedProtobufValueType BestType = (DecodedProtobufValueType)std::distance(ValuesOfType.begin(), iter);

    switch (BestType)
    {
        case DecodedProtobufValueType::int64:       return "int64";
        case DecodedProtobufValueType::fixed64:     return "fixed64";
        case DecodedProtobufValueType::fixed32:     return "fixed32";
        case DecodedProtobufValueType::_string:     return "string";
        case DecodedProtobufValueType::bytes:       return "bytes";
        case DecodedProtobufValueType::protobuf:
        {
            // Grab name of protobuf from first value of protobuf type.
            for (auto& Value : Values)
            {
                if (Value->Type == BestType)
                {
                    return std::get<std::string>(Value->Value);
                }
            }
            break;
        }
    }

    return "unknown";
}

std::string DecodedProtobufValue::ToString()
{
    switch (Type)
    {
        case DecodedProtobufValueType::int64:
        case DecodedProtobufValueType::fixed64:
        {
            return std::to_string(std::get<uint64_t>(Value));
        }
        case DecodedProtobufValueType::fixed32:
        {
            return std::to_string(std::get<uint32_t>(Value));
        }
        case DecodedProtobufValueType::_string:
        {
            return "\"" + std::get<std::string>(Value) + "\"";
        }
        default:
        {
            // We don't want to convert raw bytes/protobufs to strings.
            return "";
        }
    }
    return "";
}