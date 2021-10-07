# TinyORM library defines
# ---

CONFIG(shared, dll|shared|static|staticlib) | \
CONFIG(dll, dll|shared|static|staticlib): \
    DEFINES += TINYORM_LINKING_SHARED

# TinyORM library headers include path
# ---

INCLUDEPATH += $$quote($$TINYORM_SOURCE_TREE/include/)

# Link against TinyORM library
# ---

LIBS += $$quote(-L$$TINYORM_BUILD_TREE/src$$TINY_RELEASE_TYPE/)
LIBS += -lTinyOrm
