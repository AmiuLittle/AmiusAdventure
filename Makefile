3dsx:
	cd n3ds_platform && make all
3dsx_tests:
	cd n3ds_platform/tests && make all

cia:
	cd n3ds_platform && make all

linux:
	cd pc_platform && make all

linux_release:
	cd pc_platform && make release

linux_run:
	cd target/x86_64/linux && ./AmiusAdventurePC.x86_64

exe:
	cd pc_platform && make windows

clean:
	cd n3ds_platform && make clean
	cd n3ds_platform/tests && make clean
	cd n3ds_sprite_maker && make clean
	cd pc_platform && make clean
	rm -rf build GLCache || true

testlib:
	cd amius_adventure && make test

spritemaker:
	cd n3ds_sprite_maker && make all

spritemaker_windows:
	cd n3ds_sprite_maker && make windows