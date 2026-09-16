# ===== Compiler Settings =====
CC       := gcc
EMCC     := emcc
TARGET   := piscript
WEB_OUT  := piscript.html
SCRIPT   ?= test.pi

# ===== Source Files =====
SRC := \
    pi_main.c \
    pi_token.c \
    pi_lex.c \
    pi_min.c \
    pi_list.c \
    pi_stack.c \
    pi_table.c \
    pi_string.c \
    pi_value.c \
    pi_object.c \
    pi_compiler.c \
    pi_parser.c \
    pi_vm.c \
    screen.c \
    common.c \
    pi_func.c \
    pi_frame.c \
    gc.c \
    cart.c \
    builtin/pi_math.c \
    builtin/pi_plot.c \
    builtin/pi_time.c \
    builtin/_pi_string.c \
    builtin/pi_io.c \
    builtin/pi_sys.c \
    builtin/pi_audio.c \
    builtin/pi_col.c \
    builtin/pi_fun.c \
    builtin/pi_mat.c \
    builtin/pi_type.c \
    builtin/pi_obj.c \
    builtin/pi_render.c \
    builtin/pi_img.c \
    builtin/pi_builtin.c

EM_SRC := $(SRC)

# ===== Common Flags =====
CSTD := -std=c99

# ===== Debug Build =====
DEBUG_FLAGS := -g -DDEBUG_BUILD $(CSTD) -pthread
DEBUG_LIBS  := -lmingw32 -lSDL2main -lSDL2_image -lSDL2_Mixer -lSDL2 -lshlwapi

debug:
	$(CC) $(DEBUG_FLAGS) -o $(TARGET) $(SRC) $(DEBUG_LIBS)

# ===== Release Build =====
RELEASE_FLAGS := -g -O3 -DNDEBUG -flto $(CSTD) -static-libgcc -static-libstdc++
RELEASE_INC   := -ISDL2/include
RELEASE_LIBDIR:= -LSDL2/lib

RELEASE_LIBS := \
	-lmingw32 \
	-lSDL2main \
	-lSDL2_image \
	-lSDL2_Mixer \
	-lSDL2 \
	-Wl,-Bstatic -lwinpthread -Wl,-Bdynamic \
	-lwinmm -lole32 -loleaut32 -luuid \
	-lsetupapi -limm32 -lversion -lshlwapi

release:
	$(CC) $(RELEASE_FLAGS) $(RELEASE_INC) $(RELEASE_LIBDIR) \
	-o $(TARGET) $(SRC) $(RELEASE_LIBS)

# ===== Emscripten Build =====
EM_FLAGS := \
	-Os \
	-s ASYNCIFY \
	-s ALLOW_MEMORY_GROWTH \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=createMyModule \
	-s EXPORT_ES6=1 \
	-s USE_SDL=2 \
	-s USE_SDL_IMAGE=2 \
	-s USE_SDL_MIXER=2 \
	-s FULL_ES3=1 \
	-s OFFSCREEN_FRAMEBUFFER=1 \
	-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,FS,FS_createPreloadedFile \
	-s EXPORTED_FUNCTIONS=_main,_set_source,_stop_execution,_pause_execution,_resume_execution,_init_audio,_load_sprites_from_buffer,_load_sfx_from_buffer \
	--shell-file index.html

EM_WEB_FLAGS := \
	-Os \
	-s ASYNCIFY \
	-s ALLOW_MEMORY_GROWTH \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=createMyModule \
	-s EXPORT_ES6=1 \
	-s USE_SDL=2 \
	-s USE_SDL_IMAGE=2 \
	-s USE_SDL_MIXER=2 \
	-s FULL_ES3=1 \
	-s OFFSCREEN_FRAMEBUFFER=1 \
	-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,FS,FS_createPreloadedFile \
	-s EXPORTED_FUNCTIONS=_main,_set_source,_stop_execution,_pause_execution,_resume_execution,_init_audio,_load_sprites_from_buffer,_load_sfx_from_buffer

emscripten:
	$(EMCC) $(EM_FLAGS) -o $(WEB_OUT) $(EM_SRC)

website-wasm:
	$(EMCC) $(EM_WEB_FLAGS) -o website/src/lib/wasm/piscript.js $(EM_SRC)

# ===== Run =====
run: release
	./$(TARGET).exe $(SCRIPT)

# ===== Clean =====
clean:
	rm -f $(TARGET) $(TARGET).exe $(WEB_OUT) *.o
