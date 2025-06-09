# Compiler and tools
CC = gcc
EMCC = emcc

# Directories
SRC = .
SDL_DIR = $(SRC)/SDL2

# Flags
CFLAGS_DEBUG = -g -DDEBUG_BUILD -std=c99
CFLAGS_RELEASE = -g -O3 -std=c99
SDL_FLAGS = -I$(SDL_DIR)/include -L$(SDL_DIR)/lib -lmingw32 -lSDL2main -lSDL2

# Sources
SRCS = \
	$(SRC)/pi_main.c \
	$(SRC)/pi_token.c \
	$(SRC)/pi_lex.c \
	$(SRC)/list.c \
	$(SRC)/pi_stack.c \
	$(SRC)/pi_table.c \
	$(SRC)/string.c \
	$(SRC)/pi_value.c \
	$(SRC)/pi_object.c \
	$(SRC)/pi_compiler.c \
	$(SRC)/pi_parser.c \
	$(SRC)/pi_vm.c \
	$(SRC)/screen.c \
	$(SRC)/common.c \
	$(SRC)/pi_func.c \
	$(SRC)/pi_frame.c \
	$(SRC)/gc.c \
	$(SRC)/builtin/pi_math.c \
	$(SRC)/builtin/pi_plot.c \
	$(SRC)/builtin/pi_time.c \
	$(SRC)/builtin/pi_string.c \
	$(SRC)/builtin/pi_io.c \
	$(SRC)/builtin/pi_sys.c \
	$(SRC)/builtin/pi_audio.c \
	$(SRC)/builtin/pi_col.c \
	$(SRC)/builtin/pi_fun.c \
	$(SRC)/builtin/pi_mat.c \
	$(SRC)/builtin/pi_type.c \
	$(SRC)/builtin/pi_obj.c

# Targets
all: release

debug:
	$(CC) $(CFLAGS_DEBUG) -o pi $(SRCS) $(SDL_FLAGS)

release:
	$(CC) $(CFLAGS_RELEASE) -o pi $(SRCS) $(SDL_FLAGS)

emscripten:
	$(EMCC) -s ASYNCIFY -s ALLOW_MEMORY_GROWTH \
		-s MODULARIZE=1 -s EXPORT_NAME=createMyModule -s EXPORT_ES6=1 \
		-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s FULL_ES3=1 \
		-s OFFSCREEN_FRAMEBUFFER=1 \
		-s EXPORTED_FUNCTIONS='["_main","_set_source","_stop_execution"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		-Os -o piscript.html $(SRCS) --shell-file index.html

run: release
	./pi

clean:
	rm -f pi *.o *.wasm *.html *.js *.data
