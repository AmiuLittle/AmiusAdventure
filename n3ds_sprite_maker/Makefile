ARCH := `uname -m`

all:
	mkdir -p build
	cd build && cmake .. && make
	mv build/src/3dsSpriteMaker ../3dsSpriteMaker.$(ARCH)

clean:
	rm -rf build ../3dsSpriteMaker.* || true

windows:
	mkdir -p build
	cd build && /usr/x86_64-w64-mingw32/lib/qt6/bin/qt-cmake -DWIN32=1 .. && make
	mv build/3dsSpriteMaker.exe ..