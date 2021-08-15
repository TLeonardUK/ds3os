/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

enum class ENUM_NAME
{
    Undefined = -1,

#   define ENTRY(ident, display_name, value) ident = value,
#   include ENUM_FILE
#   undef ENTRY
};

#define strings_join2(a, b) a ## b
#define strings_join(a, b) strings_join2(a, b)
#define strings_array_name strings_join(strings_, ENUM_NAME)

inline std::unordered_map<ENUM_NAME, std::string> strings_array_name = {
#   define ENTRY(ident, display_name, value) { (ENUM_NAME)value, display_name },
#   include ENUM_FILE
#   undef ENTRY
};

template<>
inline std::string GetEnumString<ENUM_NAME>(ENUM_NAME value)
{
    return strings_array_name[value];
}

template<>
inline ENUM_NAME GetEnumValue<ENUM_NAME>(const std::string& value)
{
    for (auto& pair : strings_array_name)
    {
        if (value == pair.second)
        {
            return pair.first;
        }
    }
    return ENUM_NAME::Undefined;
}

#undef strings_join2
#undef strings_join
#undef strings_array_name