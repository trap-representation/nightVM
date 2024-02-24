#ifndef NVM_ALLOC_H
#define NVM_ALLOC_H

static inline void *aligned_malloc(size_t alignment, size_t size){
	void *return_mem;
	if(posix_memalign(&return_mem, alignment, size) == 0){
		return return_mem;
	}
	else{
		return NULL;
	}
}

static inline void *aligned_realloc(void *ptr, size_t alignment, size_t size, size_t prev_size){
	void *new_mem = aligned_malloc(alignment, (size + alignment - 1) & ~ (alignment - 1));

	if(new_mem != NULL && ptr != NULL){
		size_t size_to_copy=prev_size > size ? size : prev_size;
		memcpy(new_mem, ptr, size_to_copy);
		free(ptr);
	}

	return new_mem;
}

#endif
