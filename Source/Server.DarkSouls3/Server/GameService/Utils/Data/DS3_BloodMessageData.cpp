/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/Utils/Data/DS3_BloodMessageData.h"

#include "Shared/Core/Utils/Strings.h"

bool DS3_BloodMessageData::Parse(const std::vector<uint8_t>& data, DS3_BloodMessageData& output)
{
    constexpr size_t k_messageDataOffset = 0x18C;
    constexpr size_t k_messageDataSize = 28;

    if (data.size() < k_messageDataOffset + k_messageDataSize)
    {
        return false;
    }

    uint16_t msg_template_id = *reinterpret_cast<const uint16_t*>(data.data() + 0x19C);
    uint8_t msg_gesture_id = *reinterpret_cast<const uint8_t*>(data.data() + 0x19E);
    uint16_t msg_word_id = *reinterpret_cast<const uint16_t*>(data.data() + 0x1A0);
    uint16_t msg_conjunction_id = *reinterpret_cast<const uint16_t*>(data.data() + 0x1A2);
    uint16_t msg_template_2_id = *reinterpret_cast<const uint16_t*>(data.data() + 0x1A4);
    uint16_t msg_word_2_id = *reinterpret_cast<const uint16_t*>(data.data() + 0x1A6);

    output.Template = static_cast<DS3_BloodMessageTemplate>(msg_template_id);
    output.Gesture = static_cast<DS3_BloodMessageGesture>(msg_gesture_id);
    output.Word = static_cast<DS3_BloodMessageWord>(msg_word_id);
    output.Conjunction = static_cast<DS3_BloodMessageConjunction>(msg_conjunction_id);
    output.Second_Template = static_cast<DS3_BloodMessageTemplate>(msg_template_2_id);
    output.Second_Word = static_cast<DS3_BloodMessageWord>(msg_word_2_id);

    return true;
}

std::string DS3_BloodMessageData::ToString()
{
    std::string template_1 = StringFormat(GetEnumString(Template).c_str(), GetEnumString(Word).c_str());
    
    if (Conjunction != DS3_BloodMessageConjunction::none)
    {
        std::string template_2 = StringFormat(GetEnumString(Second_Template).c_str(), GetEnumString(Second_Word).c_str());
        std::string conjunction = GetEnumString(Conjunction);
        
        return StringFormat("%s\n%s %s", template_1.c_str(), conjunction.c_str(), template_2.c_str());
    }
    else
    {
        return template_1;
    }
}