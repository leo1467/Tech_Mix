CC=g++
CCFlags=-std=c++17 -O3 -Wall -Wextra

tmpDir=./tmp/
binDir=./bin/
srcDir=./src/

target=main
srcFiles:=$(wildcard $(srcDir)*.c $(srcDir)*.cc)
objs:=$(patsubst $(srcDir)%, $(tmpDir)%, $(patsubst %.c, %.o, $(patsubst %.cc, %.o, $(srcFiles))))

all: init $(tmpDir)$(target)

init:
	@if [ ! -d $(tmpDir) ]; then \
		mkdir -p $(tmpDir); \
	fi
	
	@if [ ! -d $(binDir) ]; then \
		mkdir -p $(binDir); \
	fi
	@echo compile $(objs)
	@echo build $(tmpDir)$(target)

$(tmpDir)$(target): $(objs)
	@echo "---- build $@ ----"
	$(CC) $(CCFlags) -o $@ $^
	@echo .

$(tmpDir)%.o: $(srcDir)%.cc
	@echo "---- Compile $< ----"
	$(CC) $(CCFlags) -o $@ -c $<
	@echo .

$(tmpDir)%.o: $(srcDir)%.c
	@echo "---- Compile $< ----"
	$(CC) $(CCFlags) -o $@ -c $<
	@echo .

clean:
	rm $(tmpDir)$(target) $(objs); \
	rm $(binDir)$(target);

install: all
	@echo "--- install binaries ... $(target)"; \
	cp -p $(tmpDir)$(target) $(binDir);

