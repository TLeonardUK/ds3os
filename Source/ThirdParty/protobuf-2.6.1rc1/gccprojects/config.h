/* protobuf config.h for GCC. We should use auto-generation for this, this is just a quick shortcut to match MSVC */

#define HASH_NAMESPACE std

#define HASH_MAP_H <unordered_map>
#define HASH_SET_H <unordered_set>

#define HAVE_HASH_MAP 1
#define HAVE_HASH_SET 1
#define HASH_SET_CLASS unordered_set
#define HASH_MAP_CLASS unordered_map

