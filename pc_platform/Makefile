ARCH := $(shell uname -m)

GLSL_VERT_SRC = $(wildcard shaders/*.glsl.vert)
GLSL_FRAG_SRC = $(wildcard shaders/*.glsl.frag)

VERT_SPV = $(patsubst shaders/%.glsl.vert,build/shaders/%.vert.spv,$(GLSL_VERT_SRC))
FRAG_SPV = $(patsubst shaders/%.glsl.frag,build/shaders/%.frag.spv,$(GLSL_FRAG_SRC))

SHADER_SPV_LINUX = $(sort $(patsubst build/shaders/%.vert.spv,build/$(ARCH)/linux/data/gfx/shaders/%.spv,$(VERT_SPV)) $(patsubst build/shaders/%.frag.spv,build/$(ARCH)/linux/data/gfx/shaders/%.spv,$(FRAG_SPV)))
SHADER_SPV_WINDOWS = $(sort $(patsubst build/shaders/%.vert.spv,build/$(ARCH)/windows/data/gfx/shaders/%.spv,$(VERT_SPV)) $(patsubst build/shaders/%.frag.spv,build/$(ARCH)/windows/data/gfx/shaders/%.spv,$(FRAG_SPV)))

all: $(VERT_SPV) $(FRAG_SPV) $(SHADER_SPV_LINUX)
	mkdir -p build/$(ARCH)/linux
	cp -r data build/$(ARCH)/linux
	cd build/$(ARCH)/linux && cmake ../../.. && make
	rm -rf ../target/$(ARCH)/linux || true
	mkdir -p ../target/$(ARCH)/linux
	mv build/$(ARCH)/linux/src/AmiusAdventurePC ../target/$(ARCH)/linux/AmiusAdventurePC.$(ARCH)
	mv build/$(ARCH)/linux/data ../target/$(ARCH)/linux

release: $(VERT_SPV) $(FRAG_SPV) $(SHADER_SPV_LINUX)
	mkdir -p build/$(ARCH)/linux
	cp -r data build/$(ARCH)/linux
	cd build/$(ARCH)/linux && cmake -DCMAKE_BUILD_TYPE=Release ../../.. && make
	rm -rf ../target/$(ARCH)/linux || true
	mkdir -p ../target/$(ARCH)/linux
	mv build/$(ARCH)/linux/src/AmiusAdventurePC ../target/$(ARCH)/linux/AmiusAdventurePC.$(ARCH)
	mv build/$(ARCH)/linux/data ../target/$(ARCH)/linux

build/$(ARCH)/linux/data/gfx/shaders/%.spv: build/shaders/%.vert.spv build/shaders/%.frag.spv
	mkdir -p build/$(ARCH)/linux/data/gfx/shaders
	spirv-link $^ -o $@

build/shaders/%.vert.spv: shaders/%.glsl.vert
	mkdir -p build/shaders
	glslc -fshader-stage=vert $< -o $@

build/shaders/%.frag.spv: shaders/%.glsl.frag
	mkdir -p build/shaders
	glslc -fshader-stage=frag $< -o $@ -fentry-point=frag

clean:
	rm -rf build ../target || true

windows:
	mkdir -p build/$(ARCH)/windows
	cp -r data build/$(ARCH)/windows
	cd build/$(ARCH)/windows && x86_64-w64-mingw32-cmake -DCMAKE_TOOLCHAIN_FILE=../../../../windows-toolchain.cmake ../../.. && make
	rm -rf ../target/$(ARCH)/windows || true
	mkdir -p ../target/$(ARCH)/windows
	mv build/$(ARCH)/windows/src/AmiusAdventurePC.exe ../target/$(ARCH)/windows/AmiusAdventurePC.exe
	mv build/$(ARCH)/windows/data ../target/$(ARCH)/windows


