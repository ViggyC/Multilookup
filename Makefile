# Makefile v1 for CSCI3753-SP22 PA3C
# Do not modify anything other MSRCS & MHDRS

CC = clang
CFLAGS =  -Wextra -Wall -pedantic -g -std=gnu99
INCLUDES = 
LFLAGS = 
LIBS = -lpthread

MAIN = multi-lookup

# Add any additional .c files to MSRCS and .h files to MHDRS
MSRCS = test.c array.c #multi-lookup.c
MHDRS = array.h #multi-lookup.h

# Do not modify these lines
SRCS = $(MSRCS) util.c
HDRS = $(MHDRS) util.h

OBJS = $(SRCS:.c=.o) 

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean: 
	$(RM) *.o *~ $(MAIN)

SUBMITFILES = $(MSRCS) $(MHDRS) Makefile
submit: 
	@read -r -p "Enter your identikey username: " username; \
	echo; echo Bundling the following files for submission; \
	tar --transform "s|^|PA3-$$username/|" -cvf PA3-$$username.txt $(SUBMITFILES); \
	echo; echo Please upload the file PA3-$$username.txt to Canvas to complete your submission; echo
