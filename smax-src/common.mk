# Build flags to be used when building files or projects

# Optimize the code as much as possible
CFLAGS    += -flto

CFLAGS     += -Wextra -Wno-unused-parameter

# Make sure we do not introduce warnings again
CFLAGS     += -Wno-literal-suffix -Werror
