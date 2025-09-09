# Simple makefile to:
# - Compile a library with the CDF code
# - Compile a test program for the CDF code

# Where to find the NASA CDF include and library files
CDF_INC = -I../../include
CDF_LIB = -L../../lib -lcdf

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g $(CDF_INC)
LDLIBS += $(LIB) $(CDF_LIB) -lm

# Library name
LIB = libimcdf.a

# Test program name
TEST_PROG = imag_cdf_test

# Library source and object files
LIB_SRCS = imcdf.c imcdf_low_level.c imcdf_utils.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Test program source and object files
TEST_PROG_SRCS = imag_cdf_test.c
TEST_PROG_OBJS = $(TEST_SRCS:.c=.o)

# Default target
all: $(LIB) $(TEST_PROG)

# Build the test program
$(TEST_PROG): $(TEST_PROG_OBJS) 

# Build the static library
$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

# Compile .c into .o
%.o: %.c imcdf.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(LIB_OBJS) $(LIB) $(TEST_PROG_OBJS) $(TEST_PROG)

.PHONY: all clean
