.PHONY : all clean

CFLAGS := -Wall -D_GNU_SOURCE
LDFLAGS := -lusb-1.0 -lefence
OBJS := $(addsuffix .o, $(basename $(wildcard */*.c *.c)))
APP := boatlooder

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(APP)

clean:
	rm -f $(OBJS) $(APP) $(OBJS:.o=.d)

%.d: %.c
	$(CC) -MM -MF $@ $<

-include $(OBJS:.o=.d)
