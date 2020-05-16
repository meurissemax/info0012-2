|; Group: MEURISSE Maxime & VERMEYLEN Valentin

|; Dynamic allocation registers:
|; - Base block pointer (BBP): points to the first block
|; - Free pointer (FP): points the first free block of the free list
|; - NULL: value of the null pointer (0)
BBP = R26
FP = R25
NULL = 0

bbp_init_val:
	LONG(0x3FFF8)

|; reset the global memory registers
.macro beta_alloc_init() LDR(bbp_init_val, BBP) MOVE(BBP, FP)
|; call malloc to get an array of size Reg[Ra]
.macro MALLOC(Ra)        PUSH(Ra) CALL(malloc, 1)
|; call malloc to get an array of size CC
.macro CMALLOC(CC)	     CMOVE(CC, R0) PUSH(R0) CALL(malloc, 1)
|; call free on the array at address Reg[Ra]
.macro FREE(Ra)          PUSH(Ra) CALL(free, 1)


|;--------------------------------------------------------------------------------------------------
|; Purpose : Given a block, finds the address at the end of it, i.e. the address of the block that is directly greater, in address, than the given block.
|; Argument :
|; 	-RA contains the address of the block.
|; 	-RB contains nothing of importance, as it will contain the address we computed..
|; Produces :
|; 	-RA is unchanged
|; 	-RB contains the address at the end of the block.
|;--------------------------------------------------------------------------------------------------
.macro FIND_ADDRESS_NEXT(RA, RB) {
	PUSH(RA) |; We do not want to modify it.
	LD(RA, 1*4, RB) |; RB contains the size, in words, of the given block.
	ADDC(RA, 2*4, RA) |; RA contains the address of the first usable memory word in the block.
	MULC(RB, 4, RB) |; RB contains the size, in bytes, of the given block.
	ADD(RA, RB, RB) |; RB contains the address at the end of the given block.
	POP(RA) |; RA is not modified.
}


|;--------------------------------------------------------------------------------------------------
|; Dynamically allocates an array of size n.
|; Args:
|;  - n (>0): size of the array to allocate
|; Returns:
|;  - the address of the allocated array
|;--------------------------------------------------------------------------------------------------
malloc:
	PUSH(LP) PUSH(BP)
	MOVE(SP, BP)

	|; Insert your malloc implementation here ....

  	PUSH(R1) |; Will contain n.
	PUSH(R7) |; Will hold the address of the block under consideration in the free list.
  	PUSH(R6) |; For "predecessor" of R7
  	PUSH(R3) |; For intermediary results
  	PUSH(R2) |; To contain the size of the free blocks
  	PUSH(R9) |; Later used to contain the address of the next free block in the linked list.
	PUSH(R11) |; Will be used to contain the address the previous node in the linked list must be updated to (first word of its header).

  	LD(BP, -4 * 3, R1) |; We placed n in R1.

	CMPLEC(R1, 0, R3) |; Is n <= 0 ?
	BNE(R3, argument_error) |; If so, we must return immediately.

  	|; We now have to check if there is a block of the correct size in memory that we can use.

  	MOVE(FP, R7) |; R7 will be used to navigate the linked list.
  	BR(find_block) |; We do not hold LP as we have finished with this block.


|;--------------------------------------------------------------------------------------------------
|; Purpose : Used to find a free block of the right size in the linked list.
|; Registers before entering :
|;  - R1 contains the value n.
|;  - R7 contains the address of the free block under consideration.
|; 	- R6 contains the previous node, or nothing if it is the first time we enter this function.
|;  - R3 can be used for intermediary values.
|;  - R2 is used to contain the size of the free blocks.
|; Registers after leaving :
|; 	- R0 contains the address of the first usable word in that block.
|; 	- All other changes do not need to be documented as we branch to the end of malloc, which will pop all registers except R0.
|;--------------------------------------------------------------------------------------------------
find_block:
	LD(R7, 0, R3) |; R3 contains the address of the next free block
  	CMPEQC(R3, NULL, R3) |; Do we have a free block ? (Are we at the end of the list ?)
  	BNE(R3,maybe_create_free_block) |; We do not, so we have to create a free block.

  	LD(R7, 1 * 4, R2) |; R2 contains the size of the block under consideration.

  	CMPLT(R2, R1, R3) |; Is the size of our block (m) strictly smaller than n ?
  	BNE(R3, find_other_block) |; It is, we thus have to find another block.

  	CMPEQ(R2, R1, R3) |; Does m == n ?
  	BNE(R3, block_exact_size)

	|; We know that m > n.

  	SUBC(R2, 1, R2) |; m = m-1
  	CMPEQ(R2, R1, R3) |; Is m-1 == n ?
  	BNE(R3, find_other_block) |; The block is of size n+1. That is not acceptable.


|;--------------------------------------------------------------------------------------------------
|; The free block is of size >= n + 2. We need to cut it. The registers used and the effect produced is the same as find_block.
|;--------------------------------------------------------------------------------------------------
find_block_continued:
	LD(R7, 1 * 4, R2) |; R2 contains the size of the block under consideration.

	MOVE(R7, R11) |; To later check whether R7 is equal to FP or not, as R7 will change value before the check.

	MOVE(R7, R0) |; R0 contains the address of the free block that will be allocated.
	ADDC(R0, 2 * 4, R0) |; We store in R0 the return value of malloc, i.e. the address of the first word in the free block that has been created. (NB : we do not return the address of the free block, but the one that is 2 words further as we don't want the header to be overriden by the user when he will use the allocated space.)

	ST(R1, 1*4, R7) |; We write the size of the block (n) in the header.
  	LD(R7, 0, R9) |; R9 contains the address of the next free block in the linked list.

	|; We now have to cut the block in two.

  	ADDC(R7, 2*4, R7) |; R7 contains the address of the first element of the first block, after the header.
  	MULC(R1, 4, R1) |; R1 contains the size of the block, in bytes.
  	ADD(R7, R1, R7) |; R7 now contains the address just after the end of the first new block, i.e. the address of the new free block.
  	ST(R9, 0, R7) |; The first word of the header of the new free block contains the address of the next element in the linked list.
  	DIVC(R1, 4, R1) |; R1 contains n, in words and not in bytes anymore.
  	SUBC(R2, 2, R2) |; Substract the size of the new header to m (size of the initial block).
  	SUB(R2, R1, R2) |; R2 now contains the real size of the second block (the free one).
  	ST(R2, 1*4, R7) |; Store the size of the second block in the second word of the header (header of the free block).

	|; The header of the free block has been updated successfully. We now have to update the "predecessor" of the block in the list (the previous node).

  	CMPEQ(R11, FP, R3) |; Was our block at the beginning of the linked list (R11 is the address of the initial block that we have cut) ?
  	BNE(R3, update_fp) |; Our block was at the beginning of the linked list, so we must update FP, and give it the address contained in R7, i.e. that of the newly "created" free block.

  	ST(R7, 0, R6) |; Else we update the pointer of the previous node, as R7 now contains the address of the "new" free block.

	CMOVE(NULL, R2)
	ST(R2, 0, R11) |; The allocated block points to nothing.

  	BR(end_of_malloc) |; We have successfully removed our block from the linked list.


|;--------------------------------------------------------------------------------------------------
|; Purpose : The block under consideration is not of the right size, we must iterate to the next one.
|; Registers before entering :
|; 	- R7 contains the address of the last free block uder consideration.
|; 	- R6 contains the address of the previous node in the list.
|; Registers after leaving :
|; 	- R7 contains the address of the next block to consider.
|; 	- R6 contains the address of the previous node.
|;--------------------------------------------------------------------------------------------------
find_other_block:
	MOVE(R7, R6) |; Update the previous node
  	LD(R7, 0, R7) |; Update the address of the node under consideration to the next one in the linked list.
  	BR(find_block)


|;--------------------------------------------------------------------------------------------------
|; Purpose : The block at the address contained in R7 is of the exact same size as that required by the user, we allocate it.
|; Meaningful registers before entering :
|;  - R1 contains the value n.
|;  - R7 contains the address of the free block under consideration.
|; 	- R6 contains the address of the previous node in the list, or nothing if the block is the first of the list..
|;  - R3 can be used for intermediary values.
|; Meaningful registers after leaving :
|; 	- R0 contains the address of the first usable word in that block.
|; 	- All other changes in registers are of no interest as they will be popped right after.
|;--------------------------------------------------------------------------------------------------
block_exact_size:
	MOVE(R7, R0) |; Store the address of the first free word in R0 to return.
	ADDC(R0, 2*4, R0) |; R0 now holds the address of the first memory space of the block.

  	|; The block is of the right size ! It can either be in the beginning of the heap, in the end or inside.

	CMPEQ(R7, FP, R3) |; Are we at the beginning of the heap?
	BNE(R3, block_beginning) |; Yes we are. We must therefore only update FP to remove our block from the linked list.

	|; We are not at the beginning of the list. We must update the header of the previous block, and the address of that block is contained in R6.

	LD(R7, 0, R3) |; R3 contains the address of the next node in the list (the one after the block that will be used).
	ST(R3, 0, R6) |; R6 now contains the address to the next node, and the used block has been removed from the freed list.

	CMOVE(NULL, R2)
	ST(R2, 0, R7) |; The header of the allocated block points to nothing.

	BR(end_of_malloc)


|;--------------------------------------------------------------------------------------------------
|; Purpose : The block, of the exact size, is at the beginning of the free list. We must therefore only update FP to remove it.
|; Registers before entering :
|; 	- R7 contains the address of the block we want to allocate.
|; 	- FP contains the address of the block we want to allocate.
|; Registers after leaving :
|; 	- FP contains the address of the next block in the linked list.
|;--------------------------------------------------------------------------------------------------
block_beginning:
	LD(R7, 0, FP) |; The address of the next free block is charged in FP, removing the block from the freed list.
	CMOVE(NULL, R2)
	ST(R2, 0, R7) |; The header points to NULL.
	BR(end_of_malloc)


|;--------------------------------------------------------------------------------------------------
|; We have reached the last block in the linked list, pointing to NULL. Does it have the size we want ?
|;  - R1 contains the value n.
|; 	- R7 contains the address of the last free block (address of the first word of the header)
|; 	- R3 and R2 can be used for intermediary results.
|;--------------------------------------------------------------------------------------------------
maybe_create_free_block:
	LD(R7, 1*4, R3) |; R3 contains the size of the block.
	CMPLT(R3, R1, R2) |; Is the size less strictly less than n ?
	BNE(R2, create_free_block)

	CMPEQ(R3, R1, R2)
	BNE(R2, block_exact_size)

	SUBC(R3, 1, R3) |; Is the size == n + 1
	CMPEQ(R3, R1, R2)
	BNE(R2, create_free_block)

	BR(find_block_continued) |; else we have a block of size >= n+2


|;--------------------------------------------------------------------------------------------------
|; Purpose : There is no free block large enough to handle the size of the block that is requested by the user. We must therefore create one at the end of the heap.
|; Registers before entering :
|;  - R1 contains the value n.
|; 	- BBP contains the address of the last header of the heap.
|; Registers after leaving :
|; 	- R0 contains the address of the first word of addressable memory of the newly created block.
|; 	- BBP contains the address of the last header of the heap.
|; 	- All other changes to the registers are of no interest as they will be popped right after.
|;--------------------------------------------------------------------------------------------------
create_free_block:
	|; Is the block not too large ?
	MOVE(BBP, R3) |; R3 contains the last address used in the heap.
	MULC(R1, 4, R1) |; R1 contains the size of the block, in bytes.
	SUBC(R3, 4, R3) |; R3 points to the first free block in memory.
	SUB(R3, R1, R3) |; R3 points to the second word of the header of the block.
	SUBC(R3, 4, R3) |; R3 points to the header of the block we want to create.
	DIVC(R1, 4, R1) |; We restore the content of R1

	|; Now, we must make sure it does not overflow on the stack.

	CMPLT(R3, SP, R2) |; Does it overflow ?
	BNE(R2, argument_error) |; If so, we must return an error.

	|; And we must make sure it does not become greater than BBP due to integer overflow.
	CMPLE(BBP, R3, R2)
	BNE(R2, argument_error)


	MULC(R1, 4, R1) |; R1 contains the size of the block to allocate, in bytes.
	SUBC(BBP, 4, BBP) |; BBP now points to the space right after the last block in the heap.
	SUB(BBP, R1, BBP) |; BBP now points to the second words of the header, that must contain the size.
	DIVC(R1, 4, R1) |; R1 contains the size of the block to allocate, in words.
	ST(R1, 0, BBP) |; The newly created block contains its size.
	CMOVE(NULL, R2)
	ST(R2, -1*4, BBP) |; We store the address NULL in the header.
	SUBC(BBP, 1*4, BBP) |; BBP points to the header of the last block.

	ADDC(BBP, 2*4, R0) |; R0 holds the address of the first free memory space of the newly created block.
	BR(end_of_malloc)


|;--------------------------------------------------------------------------------------------------
|; Purpose : We have cut a block of size >= n+2. This function will update FP with the address contained in register R7. (NOT the address pointed by the content of R7, as a LD would do.)
|; Registers before entering :
|; 	- R7 contains the address of the new free block, after the large block has been cut in two.
|; 	- FP holds the address of the newly allocated block.
|; 	- R11 holds the address of the newly allocated block.
|; Registers after leaving :
|; 	- FP holds the address of the first free block in the linked list.
|;--------------------------------------------------------------------------------------------------
update_fp:
	MOVE(R7, FP)
	CMOVE(NULL, R2)
	ST(R2, 0, R11) |; The allocated block points to nothing.
	BR(end_of_malloc)


|;--------------------------------------------------------------------------------------------------
|; Purpose : The argument n was too big or too small (<= 0). We must therefore signal the user an error has occured by placing NULL in R0.
|; Registers after leaving :
|; 	- R0 contains the value hold in the NULL "variable".
|;--------------------------------------------------------------------------------------------------
argument_error:
	CMOVE(NULL, R0) |; We must return an "error".


|;--------------------------------------------------------------------------------------------------
|; Purpose : Pop all pushed registers and return to the calling code.
|;--------------------------------------------------------------------------------------------------
end_of_malloc:
	POP(R11)
	POP(R9)
	POP(R2)
	POP(R3)
	POP(R6)
	POP(R7)
	POP(R1)
	POP(BP)
	POP(LP)
	RTN()


|;##################################################################################################
|;##################################################################################################


|;--------------------------------------------------------------------------------------------------
|; Free a dynamically allocated array starting at address p.
|; Args:
|;  - p: address of the dynamically allocated array
|;--------------------------------------------------------------------------------------------------
free:
	PUSH(LP) PUSH(BP)
	MOVE(SP, BP)

	|; Insert your free implementation here ....

	PUSH(R1) |; Will hold the address of the block we must free.
	PUSH(R2) |; For intermediary results
	PUSH(R3) |; Will contain the address of the previous node in the linked list.
	PUSH(R7) |; Will contain the address of the next node in the linked list.
	PUSH(R8) |; To hold the return value of some branchings.

	LD(BP, -4 * 3, R1) |; We placed p in R1.
	SUBC(R1, 2*4, R1) |; We want R1 to hold the address of the beginning of the block, i.e. the header, and not the beginning of the usable space in the block.

	CMPLT(R1, BBP, R2) |; Is the address in the heap ?
	BNE(R2, end_of_free) |; If it is not, we simply return.

	MOVE(FP, R7) |; R7 contains the address of the first free block in the list.
	CMOVE(NULL, R3) |; R3 contains the value defined in NULL.

	BR(find_greater_address)


|;--------------------------------------------------------------------------------------------------
|; Purpose : Will find the addresses of the free nodes located before and after the address p.
|; Registers before entering :
|; 	-R1 contains the address of the block we must free.
|; 	-R2 is used for intermediary results.
|; 	-R3 contains the address of the previous node.
|; 	-R7 contains the address of the next node.
|; Registers after leaving :
|; 	- We directly go to end_of_free, so they will all be popped.
|;--------------------------------------------------------------------------------------------------
find_greater_address:
	CMPLT(R7, R1, R2) |; Is the search over ? (i.e. Is the address of the next node greater than that p ?)
	BNE(R2, loop_find_address) |; The search is not over. We must iterate.

	|; We have found the two addresses of the nodes that square p. But is the address p of the node located before that pointed by FP ? (in which case we must update FP.)

	CMPEQC(R3, NULL, R2) |; Is the address of the block to free before that pointed by FP ?
	BNE(R2, no_predecessor) |; Yes it is, there is no predecessor and we must update FP.

	|; There is a predecessor. We now just have to update the predecessor and the follower and merge them if necessary. Let's start with the update.

	LD(R3, 0, R2) |; R2 contains the address of the next block. (Used for the swap)
	ST(R1, 0, R3) |; The previous node now points to the newly freed block.
	ST(R2, 0, R1) |; The newly freed block now points to the successor.

	|; Now, we must see if we have to merge some blocks together.

	FIND_ADDRESS_NEXT(R3, R2) |; R2 contains the address at the end of the previous node.

	CMPEQ(R2, R1, R2) |; Is the freed block directly after the previous one, address-wise ?
	BNE(R2, merge_previous, R8) |; Yes it is, we therefore merge it with the previous node.

	MOVE(R1, R3) |; R3 contains the address of the current block.
	MOVE(R7, R1) |; R1 contains the address of the next block.

	FIND_ADDRESS_NEXT(R3, R2) |; R2 contains the address of the block at the end of the current one.

	CMPEQ(R2, R1, R2) |; Is the second free block directly after the first one ?
	BNE(R2, merge_previous, R8) |; Yes it is, we merge them.

	BR(end_of_free)


|;--------------------------------------------------------------------------------------------------
|; We have to keep on searching for the node that has a greater address than p.
|; Registers before entering :
|; 	- R3 holds the address of the previous node.
|; 	- R7 holds the value of the next node.
|; We must therefore update R3 and R7.
|; Registers after leaving:
|; 	-R3 and R7 have been updated.
|;--------------------------------------------------------------------------------------------------
loop_find_address:
	MOVE(R7, R3) |; We update the previous node.
	LD(R7, 0, R7) |; We update the next node.
	BR(find_greater_address)


|;--------------------------------------------------------------------------------------------------
|; We must update FP and possibly merge the block with its follower. The block to free is before FP (in address ordering).
|; Registers before entering :
|; 	-R1 contains the address of the block we must free.
|; 	-FP and R7 contain the address of the next node, in address ordering (<FP> > <R1>).
|; 	-R3 and R2 are of no use, we can use them for intermediary results.
|; Registers after leaving :
|; 	-We go directly to end_of_free, so their modification is of no interest.
|;--------------------------------------------------------------------------------------------------
no_predecessor:
	MOVE(R1, FP) |; We update FP to its new value, i.e. the newly freed block.
	ST(R7, 0, FP) |; The newly created block now points to the next free block in the list.

	|; Are the two block contiguous?

	MOVE(R7, R2) |; R2 contains the address of the next node.

	FIND_ADDRESS_NEXT(R1, R3) |; R3 contains the address at the end of the given block.

	CMPEQ(R3, R2, R3) |; Is the address at the end of the block equal to that of the next free block ?
	BNE(R3, merge_following) |; Yes it is, we have to merge the blocks.

	|; Else, it is over. The block contains its size in the header due to our implementation of malloc.
	BR(end_of_free)


|; NOTE : WE INITIALLY USED ONLY ONE MERGE AS A MACRO BUT IT TOOK 4 REGISTERS AND ONE LABEL AS PARAMETERS AND WE GOT A TIMEOUT ERROR ON THE PLATFORM. WE KNEW THIS SOLUTION, WITH 2 FUNCTIONS DOING THE SAME THING, HOWEVER INELEGANT, WORKED. AS WE ONLY HAD 2 SUBMITS LEFT, WE CHOSE TO GO WITH IT.


|;--------------------------------------------------------------------------------------------------
|; We have to merge the newly freed block with its follower.
|; Registers before entering :
|; 	- R2 contains the address of the next block.
|; 	- FP (and R1) contain the address of the current block.
|; Registers after leaving :
|; 	- FP contains the address of the first block in the linked list.
|;--------------------------------------------------------------------------------------------------
merge_following:
	PUSH(R1) |; Used as local variables.
	PUSH(R3) |; Used as local variables.

	LD(FP, 1*4, R1) |; R1 contains the size of the first block
	LD(R2, 1*4, R3) |; R3 contains the size of the next block
	ADD(R1, R3, R1) |; R1 contains the size of the two blocks, not counting the headers.
	ADDC(R1, 2, R1) |; We count the second header as two blocks of memory.
	ST(R1, 1*4, FP) |; The freed block now contains the real size of the merged block.
	LD(R2, 0, R1) |; R1 contains the address of the next node in the list.
	ST(R1, 0, FP) |; The newly merged block points to the next free one.

	POP(R3)
	POP(R1)

	BR(end_of_free)


|;--------------------------------------------------------------------------------------------------
|; We have to merge the newly freed block with its follower.
|; Registers before entering :
|; 	-R1 contains the address of the current node.
|; 	-R2 is used for intermediary results.
|; 	-R3 contains the address of the previous node.
|; 	-R8 contains the return address.
|; Registers after leaving :
|; 	- R1 and R3 hold the address of the free block.
|;--------------------------------------------------------------------------------------------------
merge_previous:
	PUSH(R4) PUSH(R5) |; Local "variables"

	LD(R1, 1*4, R4) |; R4 contains the size of the first block, in words.
	LD(R3, 1*4, R5) |; R5 contains the size of the second block, in words.

	ADD(R4, R5, R4) |; R4 contains the size of the two blocks, not counting the second header.
	ADDC(R4, 2, R4) |; R4 contains the size of the combined block, and the second header becomes "free" space.

	ST(R4, 1*4, R3) |; The newly freed block contains the total size.

	|; We have to update the pointer at this point.

	LD(R1, 0, R5) |; R5 contains the address of the next node of the list.
	ST(R5, 0, R3) |; The free block now points to the next free node in the list.

	MOVE(R3, R1) |; R1 contains the address of the newly created block.

	POP(R5) POP(R4)
	JMP(R8) |; Return to the calling code.


|;--------------------------------------------------------------------------------------------------
|; Pop all used registers.
|;--------------------------------------------------------------------------------------------------
end_of_free:
	POP(R8)
	POP(R7)
	POP(R3)
	POP(R2)
	POP(R1)
	POP(BP)
	POP(LP)
	RTN()
