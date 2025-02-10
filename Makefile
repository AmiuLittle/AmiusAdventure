3dsx:
	cd n3ds_platform && make all

cia:
	cd n3ds_platform && make all

clean:
	cd n3ds_platform && make clean

testlib:
	cd amius_adventure && make test

spritemaker:
	cd n3ds_sprite_maker && make all