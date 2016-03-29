#define VER_MAJOR     "0"
#define VER_MINOR     "0"
#define VER_REVISION  "4"

#define VER_NUMERIC   000004L



const char* kv_version()
{
    return VER_MAJOR "." VER_MINOR "." VER_REVISION;
}

long kv_version_numeric()
{
    return VER_NUMERIC;
}
