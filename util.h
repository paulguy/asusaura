#ifndef UTIL_H
#define UTIL_H

#define STATIC_STRCPY(DEST, SRC) \
    strncpy(DEST, SRC, sizeof(DEST)); \
    DEST[sizeof(DEST)-1] = '\0'; \

#endif
