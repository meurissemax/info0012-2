.include beta.uasm

|; init stack and memory allocation
CMOVE(stack__, SP)
MOVE(SP, BP)
BR(main)

.include malloc.asm

main:
	|; initialize the malloc system
	beta_alloc_init()
	|; allocate an array of size 5 and then free it
	CMALLOC(5)
	MOVE(R0, R1)
	FREE(R1)
	HALT()

LONG(0xDEADCAFE)
stack__:
	|; ...
