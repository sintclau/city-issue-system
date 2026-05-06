CC=gcc
CFLAGS=-MMD -Wall -Wextra -pedantic -std=c11 -I src

# Sources shared between both binaries
SHARED_SRC=$(wildcard src/actions/*.c src/filesystem/*.c src/monitor-communication/*.c)
SHARED_OBJ=$(SHARED_SRC:%.c=%.o)

# city_manager: main.c + shared
MANAGER_SRC=src/main.c
MANAGER_OBJ=$(MANAGER_SRC:%.c=%.o)

# monitor_reports: its own main
MONITOR_SRC=src/monitor_reports.c
MONITOR_OBJ=$(MONITOR_SRC:%.c=%.o)

OBJ=$(SHARED_OBJ) $(MANAGER_OBJ) $(MONITOR_OBJ)
DEP=$(OBJ:%.o=%.d)

LIBS=$(addprefix -l,)
TARGET=/usr/local

all: debug

debug: CFLAGS += -g
debug: city_manager monitor_reports

remake: clean debug
.NOTPARALLEL: remake

release: CFLAGS += -O3 -DNDEBUG
release: clean city_manager monitor_reports
.NOTPARALLEL: release

clean:
	rm -f $(OBJ) $(DEP) city_manager monitor_reports

install: all
	cp city_manager $(TARGET)/bin
	cp monitor_reports $(TARGET)/bin

city_manager: $(MANAGER_OBJ) $(SHARED_OBJ)
	$(CC) -o $@ $^ $(LIBS)

monitor_reports: $(MONITOR_OBJ) $(SHARED_OBJ)
	$(CC) -o $@ $^ $(LIBS)

-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
