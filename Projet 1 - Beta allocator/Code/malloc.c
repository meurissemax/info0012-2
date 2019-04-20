#define block_next(p)  (*p)
#define block_size(p)  (*(p+1))
#define block_start(p) (p+2)

int* base; // BBP
int* freep; // FP

/**
 * Try to merge one blocks and the next in the free list. Merge only if they are adjacent.
 * @param block The first block to merge
 */
int try_merge_next(int* block, int* next) {
	int curr_size = block_size(block);
	// check whether blocks are adjacents
	if (block + curr_size + 2 != next) {
		return 0;
	}
	block_size(block) = curr_size + 2 + block_size(next);
	block_next(block) = block_next(next);
	return 1;
}

/**
 * Check whether the current block (curr) can hold the requested space (n).
 * If the block is valid, the free list is updated to reflect the allocation of this block
 * (removing the block from the list and adding a new block with the remaining
 * space if necessary). Then, it returns 1.
 * If the block cannot be used, the function does nothing (and returns 1)
 *
 * @param n        Size requested for allocation
 * @param curr     Pointer to the header of the current block
 * @param prev     Pointer to the header of the previous block (can be NULL, indicating that curr is
 *                 the first block of the free list)
 *
 * @returns valid  1 if the block was used, 0 otherwise
 */
int try_use_block(int n, int* curr, int* prev) {
	int* next = block_next(curr);
	int curr_size = block_size(curr);
	int n_items = n + 2;
	if (curr_size >= n_items) { // if large enough but sizes don't match exactly
		int* new_block = curr + n_items;
		block_next(new_block) = next;
		block_size(new_block) = curr_size - n_items;
		next = new_block;
	} else if (curr_size != n) { // block is not valid, cannot split
		return 0;
	}

	// remove allocated block from free list + update its header
  block_size(curr) = n;
	block_next(curr) = NULL;
	if (!prev) { // no prev block yet, update the freep pointer directly
		freep = next;
	} else {
		block_next(prev) = next;
	}
	return 1;
}

/**
 * Allocate an array of size n on the heap
 * @param n The size of the array
 * @returns A pointer to the first element of the allocated array
 */
int* malloc(int n) {
	if (n <= 0) {
		return NULL;
	}

	// look for large enough block
	int *prev = NULL, *curr = freep;
	while(curr) {
		if (try_use_block(n, curr, prev)) {
			return block_start(curr);
		}
		prev = curr; curr = block_next(curr);
	}

	// at this point, no valid block could be found so need to allocate a new one,
	// add it to the beginning of the heap and return it to the caller
	// if the new block would overwrite the stack, return NULL !
	// Note: new_bbp < "Reg[SP"] is not valid C, it means that one should check that the stack won't be overwritten
	int n_items = n + 2;
	int new_bbp = base - n_items;
	if (new_bbp < "Reg[SP]" || new_bbp >= base) { // 'new_bbp >= base' to avoid integer substraction underflow !
		return NULL;
	}
	base -= n_items;
	block_next(base) = NULL;
	block_size(base) = n;
	return block_start(base);
}

/**
 * Free an array allocated on the heap
 * @param p A pointer to the first element of the array of free
 */
void free(int* p) {
	if (p < base) { return; } // invalid memory location
	int *prev = NULL, *curr = freep;

	// skip till finding insertion point
	while(curr && curr < p) {
		prev = curr; curr = block_next(curr);
	}

	// insert freed block between prev|freep and curr in the free list
	int* freed = p - 2;
	block_next(freed) = curr;
	try_merge_next(freed, curr);
	if (prev) {
		block_next(prev) = freed;
		try_merge_next(prev, freed);
	} else {
		freep = freed;
	}
}
