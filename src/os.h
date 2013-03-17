/*
 * Xtrans expects "os.h" to provide ErrorF() & VErrorF() functions compatible
 * with the ones provided by the X server.
 */
static inline void _X_ATTRIBUTE_PRINTF(1, 0)
VErrorF(const char *f, va_list args)
{
    vfprintf(stderr, f, args);
    fflush(stderr);
}

static inline void  _X_ATTRIBUTE_PRINTF(1, 2)
ErrorF(const char *f, ...)
{
    va_list args;

    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
}
