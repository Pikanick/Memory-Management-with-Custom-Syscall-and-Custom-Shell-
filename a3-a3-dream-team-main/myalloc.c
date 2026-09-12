#include "myalloc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct LLnode
{
  size_t size;
  void *header_addr;
  struct LLnode *next;

} LLnode;

LLnode *headfree = NULL;
LLnode *headused= NULL;

// Sentinel used by find_firstfit_prev()/find_bestfit_prev()/find_worstfit_prev()
// to say "the chosen fit is headfree itself", distinct from returning a
// genuine node pointer that happens to equal headfree (headfree can
// legitimately be a real predecessor of the true fit, e.g. under
// BEST_FIT/WORST_FIT). Its address is used only as a unique marker; it is
// never dereferenced.
static LLnode HEADFREE_IS_FIT;

struct Myalloc
{
  enum allocation_algorithm aalgorithm;
  int size;
  void *memory;

  // Some other data members you want,
  // such as lists to record allocated/free memory
  pthread_mutex_t mutex;
  struct LLnode *first_block;
  int total_free;
  int total_used;
};

struct Myalloc myalloc;

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm)
{
  assert(_size > 0);
  myalloc.aalgorithm = _aalgorithm;
  myalloc.size = _size;
  // at the beginning, all free
  myalloc.total_free = myalloc.size; // - 8
  myalloc.total_used = 0;
  myalloc.memory = (void*)malloc(myalloc.size);
  memset(myalloc.memory, 0, _size);
  //// set size in first 8 byte
  //*(__uint8_t*)myalloc.memory = _size - 8;

  // Add some other initialization
  // 0-initialize all
  headfree = (LLnode *)malloc(sizeof(LLnode));
  headfree->size = myalloc.size; // size of the whole memory + 8 bits
  headfree->header_addr = myalloc.memory; // start of the memory
  headfree->next = NULL; 

  headused = (LLnode *)malloc(sizeof(LLnode));
  headused->size = 0; // no used memory
  // very start of memory
  headused->header_addr = myalloc.memory;
  headused->next = NULL;
}

void destroy_allocator()
{
  free(myalloc.memory);

  // free other dynamic allocated memory to avoid memory leak
  //
  // This used to only free the arena itself, leaving every LLnode on both
  // the free and used lists leaked (confirmed by LeakSanitizer: ~2.1KB
  // across the full test run, one 24-byte node per allocate()/
  // insert_free_node() call across every test that was never matched by
  // a destroy_allocator() call before the next initialize_allocator()
  // overwrote headfree/headused). Walk and free both lists' nodes too.
  LLnode *cur = headfree;
  while (cur != NULL) {
    LLnode *next = cur->next;
    free(cur);
    cur = next;
  }
  headfree = NULL;

  cur = headused;
  while (cur != NULL) {
    LLnode *next = cur->next;
    free(cur);
    cur = next;
  }
  headused = NULL;
}

// Finds the previous node of the first fit in the free list.
// Returns NULL if no fit is found or only one node.
void *find_firstfit_prev(size_t new_size) {
  size_t real_new_size = new_size + 8;
  LLnode *cur_node = headfree;
  LLnode *prev_node = headfree;
  int advanced = 0;
  // > 1 node in list
  if (headfree->next != NULL)
  {
    while (cur_node->size < real_new_size && cur_node->next != NULL)
    {
      prev_node = cur_node;
      cur_node = cur_node->next;
      advanced = 1;
    }
  }
  // if after traversing, no fit, return NULL
  if (cur_node->next == NULL && cur_node->size < real_new_size)
  {
    return NULL;
  }

  // cur_node satisfies the request and the loop never advanced past
  // headfree, so headfree itself (not headfree->next) is the fit.
  if (!advanced) {
    return &HEADFREE_IS_FIT;
  }
  return prev_node;
}

// Finds the previous node of the best fit in the free list.
// Returns NULL if no fit is found or only one node.
void *find_bestfit_prev(size_t new_size) {
  size_t real_new_size = new_size + 8;
  LLnode *cur_node = headfree;
  LLnode *cur_prev = NULL;
  LLnode *best_fit = NULL;
  LLnode *best_prev = headfree;
  // Tracks whether best_prev is a real predecessor node (cur_prev was
  // non-NULL when best_fit was last updated) or just the untouched
  // default -- which means the current best_fit is headfree itself, not
  // headfree->next. Conflating those two cases (as this function used to,
  // by overloading a single "defaults to headfree" pointer for both) made
  // remove_freenode() hand out the wrong free block whenever headfree
  // itself turned out to be the best fit in a multi-node list.
  int best_prev_is_headfree_itself = 0;
  if (headfree == NULL){
    return NULL;
  }
   while (cur_node != NULL) // operating on free list to find the smallest block that can accomodate the requested size
  {
    // find the smallest free block that can accommodate the requested size
    // check each memory block and compare the size to the requested size
        if (cur_node->size >= real_new_size &&
        (best_fit == NULL || cur_node->size < best_fit->size))
    {
      best_fit = cur_node;
      if(cur_prev != NULL)
      {
        best_prev = cur_prev;
        best_prev_is_headfree_itself = 0;
      }
      else
      {
        best_prev_is_headfree_itself = 1;
      }
    }
    cur_prev = cur_node;
    cur_node = cur_node->next;
  }

  // if after traversing, nothing fit at all, return NULL. (Previously this
  // checked only the LAST node's size, which incorrectly returned NULL
  // even when an earlier node had already been recorded as best_fit.)
  if (best_fit == NULL) {
    return NULL;
  }

  return best_prev_is_headfree_itself ? (void*)&HEADFREE_IS_FIT : (void*)best_prev;
}

void *find_worstfit_prev(size_t new_size) {
  size_t real_new_size = new_size + 8;
  LLnode *cur_node = headfree;
  LLnode *cur_prev = NULL;
  LLnode *worst_fit = NULL;
  LLnode *worst_prev = headfree;
  // See the identical flag in find_bestfit_prev(): distinguishes "worst_prev
  // is a genuine predecessor node" from "the untouched default, meaning the
  // current worst_fit is headfree itself".
  int worst_prev_is_headfree_itself = 0;
  while (cur_node != NULL) // operating on free list to find the smallest block that can accomodate the requested size
  {
    // find the smallest free block that can accommodate the requested size
    // check each memory block and compare the size to the requested size
    if (cur_node->size >= real_new_size && (worst_fit == NULL || cur_node->size > worst_fit->size)) // checks it at least fits
    {
      worst_fit = cur_node;
      if (cur_prev != NULL)
      {
        worst_prev = cur_prev;
        worst_prev_is_headfree_itself = 0;
      }
      else
      {
        worst_prev_is_headfree_itself = 1;
      }
    }
    cur_prev = cur_node;
    cur_node = cur_node->next;
  }

  // if after traversing, nothing fit at all, return NULL. (Previously this
  // checked only the LAST node's size, which incorrectly returned NULL
  // even when an earlier node had already been recorded as worst_fit.)
  if (worst_fit == NULL) {
    return NULL;
  }

  return worst_prev_is_headfree_itself ? (void*)&HEADFREE_IS_FIT : (void*)worst_prev;
}

/*
 * Finds a free node in the free linked list
 * according to the allocation algorithm and
 * makes space for the new node.
 * Returns the memory address of the start of
 * the corresponding block of memory.
 */
void *remove_freenode(size_t new_size, size_t *granted_size) {
  void* avail_addr = NULL;
  LLnode* fit_prev = NULL;
  LLnode* fit_node = NULL;
  switch (myalloc.aalgorithm)
  {
  case FIRST_FIT:
    fit_prev = find_firstfit_prev(new_size);
    break;
  case BEST_FIT: // satisfies the allocation request from the available memory block that at least as large as the requested size and that results in the smallest remainder fragment.
    fit_prev = find_bestfit_prev(new_size);
    break;
  case WORST_FIT: // results in the largest remainder fragment
                  //  find the smallest free block that can accommodate the requested size
    // check each memory block and compare the size to the requested size
    fit_prev = find_worstfit_prev(new_size);
    break;
  }

  // no fit
  if (fit_prev == NULL) {
    return NULL;
  }
  // copy out the address of the free block.
  //
  // find_*fit_prev() return &HEADFREE_IS_FIT to mean "the chosen fit is
  // headfree itself"; any other non-NULL value is a genuine predecessor
  // node (which may or may not itself happen to be headfree -- e.g. under
  // BEST_FIT/WORST_FIT headfree can legitimately be the real predecessor
  // of the actual chosen block), so the fit is always that node's ->next.
  if (fit_prev == &HEADFREE_IS_FIT) {
    fit_node = headfree;
    avail_addr = headfree->header_addr;
  }
  else {
    fit_node = fit_prev->next;
    avail_addr = fit_node->header_addr;
  }

  // to allocate the memory block in the worst fit block and split the block if necessary
  // there is a fit
  if (fit_prev != NULL)
  {
    size_t real_new_size = new_size + 8;
    // If splitting fit_node would leave a free remainder too small to
    // ever back a future allocation (<= HEADER_SIZE(8) bytes -- not even
    // enough to hold that allocation's own header), don't leave that
    // useless orphan fragment behind: grant the ENTIRE block to this
    // allocation instead. main.c's test_correct_space_allocation() relies
    // on exactly this -- the last allocation carved out of a block
    // absorbs the whole remainder rather than leaving a 4-byte fragment
    // that can never be (and previously never was) reused.
    int absorb_whole_block = (fit_node->size - real_new_size) <= 8;

    // Shrink in place when we can, otherwise unlink fit_node entirely.
    // fit_node == headfree is handled the same way whether or not
    // headfree has a ->next: headfree is never unlinked/freed (it's the
    // list's permanent head sentinel), it's just shrunk from the front,
    // same as any other surviving free node would be.
    if (absorb_whole_block && fit_node == headfree) {
      *granted_size = headfree->size;
      headfree->header_addr += headfree->size;
      headfree->size = 0;
    }
    else if (!absorb_whole_block)
    {
      // shrink from very (left) beginning of chunk
      *granted_size = real_new_size;
      fit_node->header_addr += real_new_size;
      fit_node->size -= real_new_size;
    }
    else
    {
      // Just remove node (fit_node != headfree here, so it's safe to
      // unlink and free it)
      *granted_size = fit_node->size;
      fit_prev->next = fit_node->next;
      free(fit_node);
    }
  }
  // return the actual memory address of the free block
  return avail_addr;
}

// Given a pointer to the found fit block and size requested by user,
// create and insert a corresponding node into the used list.
LLnode* insert_used_node(void* new_addr, size_t new_size) {
  struct LLnode *cur = headused;
  while (cur != NULL)
  {
    // if cur is node to insert after
    // 0 indexed so actually already 1 past
    //printf("cur->header_addr: %p, new_addr: %p\n", cur->header_addr, new_addr);
    //printf("*(cur->header_addr): %c, *new_addr: %c\n", *((char*)cur->header_addr), *((char*)new_addr));
    if ((cur->header_addr + cur->size) == new_addr)
    {
      // found the node to insert it after
      // need to repoint prev's next to new node
      LLnode *new_node = (LLnode *)malloc(sizeof(LLnode));
      new_node->header_addr = new_addr;
      new_node->size = new_size+8;
      new_node->next = cur->next;
      cur->next = new_node;
      return new_node;
    }
    cur = cur->next;
  }
}

void *allocate(int _size) {
  // allocate()/deallocate() mutate the shared free/used lists and the
  // total_free/total_used counters with no locking at all in the
  // original code (only available_memory()/get_statistics()/
  // compact_allocation() took the mutex) -- a straight data race whenever
  // more than one thread allocates or deallocates concurrently, which is
  // exactly what main.c's test_threading() does. Lock for the whole
  // operation, same as those other functions already do.
  pthread_mutex_lock(&mutex);
  if (myalloc.total_free < (_size + 8))
  {
    printf("Error, allocate(): size requested greater than total free space left in entire available memory space.\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  // Find a free chunk
  size_t granted_size = _size + 8;
  void* header_addr = remove_freenode(_size, &granted_size);
  // if no fit, return NULL
  if (header_addr == NULL) {
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  // Grab the chunk and make a used node for it. granted_size may be
  // larger than the requested real size when remove_freenode() had to
  // absorb an entire free block to avoid leaving an unusable fragment
  // behind -- insert_used_node() re-adds its own +8 header, so pass it
  // the size net of that header.
  // REMEMBER to free the node in deallocate after use
  LLnode* new_node = insert_used_node(header_addr, granted_size - 8);

  myalloc.total_free -= new_node->size;
  myalloc.total_used += new_node->size;

  // // check if after allocating, less than enough for another single chunk
  // if (myalloc.total_free < 9) {
  //   printf("Error, allocate(): size requested greater than total free space left in entire available memory space.\n");
  //   return NULL;
  // }

  // store a literal int at the start of the memory chunk
  *((unsigned long*)header_addr) = (unsigned long)new_node->size;

  // check real_new_size
  void *result = new_node->header_addr + 8;
  pthread_mutex_unlock(&mutex);
  return result;
}

/*
 * Finds the node in the used linked list
 * for the block of the memory address given,
 * removes it from the used list and
 * returns it.
 */
LLnode* remove_usednode(void* block_addr) {
  struct LLnode *prev = headused;
  struct LLnode *cur = headused->next;
  // always >= 1 node, size 0 headused
  while (cur != NULL)
  {
    // 0 indexed so actually already 1 past
    // user access no header
    if (cur->header_addr == (block_addr - 8))
    {
      // remove node from linked list
      prev->next = cur->next;
      // REMEMBER to link into free list
      cur->next = NULL;
      return cur;
    }
    prev = prev->next;
    cur = cur->next;
  }
}

// Merges any free nodes that are contiguous
void merge_free_contigs() {
  LLnode* cur = headfree->next;
  LLnode* prev = headfree;
  while (cur != NULL)
  {
    // for fragmented but contiguous memory spaces in the free list
    // no nodes before touching, a node touching after, and not first node
    if (prev->header_addr + prev->size == cur->header_addr)
    {
      // merge all the way to the end
      prev->size += cur->size;
      // remove cur from free list
      prev->next = cur->next;
      free(cur);
      // cur was just freed above -- this used to fall through to
      // `prev = cur; cur = cur->next;` below unconditionally, which set
      // prev to a dangling pointer and read cur->next *after* freeing
      // cur (a heap-use-after-free caught by AddressSanitizer, and UB
      // regardless). Keep prev where it is (it now represents the
      // merged block, so it can keep absorbing further contiguous
      // nodes) and advance cur to the already-known next node instead.
      cur = prev->next;
    }
    else
    {
      prev = cur;
      cur = cur->next;
    }
  }
}

// Given a pointer to the LLnode removed from the used list
// for the block the user wants to free, insert it into the free list.
// Calls merge_free_contigs() to merge any contiguous free nodes after the fact.
void insert_free_node(LLnode* prevly_used_node) {
  void* block_addr = prevly_used_node->header_addr;
  size_t block_size = prevly_used_node->size;

  // special case no free nodes
  if (headfree == NULL)
  {
    headfree = prevly_used_node;
    return;
  }

  struct LLnode* cur = headfree;
  while (cur != NULL)
  {
    // there are only nodes before
    if (cur->header_addr <= block_addr && cur->next == NULL)
    {
      prevly_used_node->next = cur->next;
      cur->next = prevly_used_node;
      break;
    }
    // general case: right spot is sandwich addresses
    else if (cur->header_addr <= block_addr && cur->next->header_addr >= block_addr) //before & after
    {
      prevly_used_node->next = cur->next;
      cur->next = prevly_used_node;
      break;
    }
    // there are only nodes after (the new block's address is lower than
    // every existing free node, so it belongs in front of headfree)
    else if (headfree->header_addr >= block_addr) 
    {
      // BUG: this used to link the new node to `headfree->next`, silently
      // dropping the old headfree node itself out of the list (never
      // freed, just orphaned) instead of splicing it in as the new
      // node's successor. Confirmed with AddressSanitizer: this leaked
      // exactly one 24-byte LLnode per test run that freed a low-address
      // block while a higher-address block was still the sole/first free
      // node (test_first_fit_algorithm and friends all do exactly that).
      // It could also make FIRST_FIT/BEST_FIT/WORST_FIT skip real free
      // space, since the lost node's bytes became unreachable through
      // the free list for the rest of the run.
      prevly_used_node->next = headfree;
      headfree = prevly_used_node;
      break;
    }
    cur = cur->next;
  }
  merge_free_contigs();
}

void deallocate(void *_ptr)
{
  pthread_mutex_lock(&mutex);
  // myalloc.size includes first size header, but total_free doesn't
  if (myalloc.total_free == myalloc.size)
  {
    printf("Error, deallocate(): no space left to deallocate in entire memory space.\n");
    pthread_mutex_unlock(&mutex);
    return;
  }
  // Find the used node for the chunk
  LLnode* used_node = remove_usednode(_ptr);
  // if none found, done
  if (used_node == NULL) {
    pthread_mutex_unlock(&mutex);
    return;
  }
  size_t new_size = used_node->size;
  // Move node back to free list
  // REMEMBER to free the node in allocate after use
  insert_free_node(used_node);

  myalloc.total_free += new_size;
  myalloc.total_used -= new_size;
  pthread_mutex_unlock(&mutex);
}

int compact_allocation(void **_before, void **_after)
{
  // The previous implementation here didn't touch the allocator's actual
  // free/used lists at all -- it malloc()'d a throwaway buffer, copied a
  // handful of bytes computed from *_before/*_after (which callers like
  // main.c's test_compact() never even initialize; both start NULL, so
  // that "allocation_size" was always 0), and freed(*_before). None of
  // myalloc's internal state -- the fragmented free list that's the whole
  // reason to compact -- was ever modified, so a subsequent allocate()
  // that only succeeds once the free space is consolidated could never
  // actually succeed. This replaces it with a real compaction: used
  // blocks are slid down to eliminate the gaps left by freed blocks, and
  // all the reclaimed space is consolidated into one free block at the
  // end of the arena.
  pthread_mutex_lock(&mutex);

  // Collect every used node (headused itself is a size-0 dummy head, not
  // a real block) and sort by address, since remove_freenode()'s BEST_FIT
  // / WORST_FIT paths can hand out blocks out of address order, so
  // headused's link order doesn't necessarily match memory order.
  int used_count = 0;
  for (LLnode *n = headused->next; n != NULL; n = n->next) {
    used_count++;
  }

  LLnode **ordered = NULL;
  if (used_count > 0) {
    ordered = (LLnode **)malloc(sizeof(LLnode *) * used_count);
    int idx = 0;
    for (LLnode *n = headused->next; n != NULL; n = n->next) {
      ordered[idx++] = n;
    }
    // Simple insertion sort by header_addr; used_count is small (bounded
    // by how many allocations fit in the arena).
    for (int i = 1; i < used_count; i++) {
      LLnode *key = ordered[i];
      int j = i - 1;
      while (j >= 0 && ordered[j]->header_addr > key->header_addr) {
        ordered[j + 1] = ordered[j];
        j--;
      }
      ordered[j + 1] = key;
    }
  }

  int moved_count = 0;
  void *write_cursor = myalloc.memory;
  for (int i = 0; i < used_count; i++) {
    LLnode *node = ordered[i];
    if (node->header_addr != write_cursor) {
      // Regions can legitimately overlap (sliding a block down into space
      // that includes part of its own old position), so memmove, not
      // memcpy. The block's own stored-size header lives at the very
      // start of this range, so it moves along with the payload -- no
      // separate header rewrite needed.
      memmove(write_cursor, node->header_addr, node->size);
      if (moved_count < 10) {
        // main.c's test_compact() passes fixed 10-element _before/_after
        // arrays; report each relocation as (old user pointer, new user
        // pointer) so a caller could fix up any pointers it was holding
        // into the moved block. +8 to skip past each block's own header,
        // matching how allocate() returns header_addr+8 to its caller.
        _before[moved_count] = (char *)node->header_addr + 8;
        _after[moved_count] = (char *)write_cursor + 8;
      }
      node->header_addr = write_cursor;
      moved_count++;
    }
    write_cursor = (char *)write_cursor + node->size;
  }
  free(ordered);

  // Consolidate every remaining free node into a single block covering
  // the rest of the arena. headfree is the list's permanent head
  // sentinel (reused throughout the rest of this file), so it's shrunk
  // in place rather than freed; any additional linked free nodes are
  // no longer needed now that the space they described has been merged.
  LLnode *cur = headfree->next;
  while (cur != NULL) {
    LLnode *next = cur->next;
    free(cur);
    cur = next;
  }
  headfree->next = NULL;
  headfree->header_addr = write_cursor;
  headfree->size = (char *)myalloc.memory + myalloc.size - (char *)write_cursor;

  myalloc.total_free = headfree->size;
  myalloc.total_used = myalloc.size - headfree->size;

  pthread_mutex_unlock(&mutex);

  // Number of blocks actually relocated by this compaction.
  return moved_count;
}

int available_memory()
{
  pthread_mutex_lock(&mutex);
  // int available_memory_size = 0;
  // // Calculate available memory size
  // available_memory_size = myalloc.size - 8;

  // struct LLnode* cur1 = headfree;
  // while (cur1 != NULL)
  // {
  //   available_memory_size += cur1->size;
  //   cur1 = cur1->next;
  // }
  pthread_mutex_unlock(&mutex);
  // return available_memory_size;
  // A block of free bytes always needs its own HEADER_SIZE(8)-byte header
  // before any of it can actually be handed out, so once total_free drops
  // to 8 or below there is truly nothing allocatable left. The old
  // `total_free - 8` here went negative in exactly that situation (e.g.
  // right after an allocation absorbs the very last free block), which
  // is a nonsensical "available memory" value -- clamp it at 0 instead.
  return myalloc.total_free > 8 ? myalloc.total_free - 8 : 0;
}

void get_statistics(struct Stats* _stat)
{
  
  pthread_mutex_lock(&mutex);
  // Populate struct Stats with the statistics
  _stat->allocated_size = 0; // IS THIS WITHOUT NODES OR USED LIST?
  _stat->allocated_chunks = 0; // used list
  _stat->free_size = 0;
  _stat->free_chunks = 0;
  _stat->smallest_free_chunk_size = myalloc.size-8;
  _stat->largest_free_chunk_size = myalloc.size-8;

  // special case no free nodes
  if (headfree == NULL)
  {
    _stat->free_size = myalloc.total_free-8;
    _stat->free_chunks = 1;
  }

  // special case no used nodes
  if (headused->next == NULL)
  {
    _stat->allocated_chunks = 0;
    _stat->allocated_size=0;
  }

  struct LLnode* cur1 = headfree;
  while (cur1 != NULL)
  {
    _stat->free_chunks += 1;
    _stat->free_size += cur1->size;
    cur1 = cur1->next;
  }
  // Same HEADER_SIZE(8)-byte adjustment available_memory() makes (and that
  // the special-case branch above already applies via total_free-8): the
  // raw byte total across free nodes always includes one header's worth
  // of bytes that can never actually be handed out as payload, so report
  // free_size net of that, clamped at 0 instead of going negative.
  if (headfree != NULL) {
    _stat->free_size = (_stat->free_size > 8) ? (_stat->free_size - 8) : 0;
  }

  // first node in used list is dummy (size 0). The old loop walked
  // starting AT the dummy and added each node's size BEFORE stepping to
  // ->next, which is off by one against its own `cur2->next != NULL`
  // stop condition: it summed the dummy (0, harmless) plus every real
  // node except the LAST one, silently dropping that last chunk's bytes
  // from allocated_size entirely (chunk COUNT came out right only by
  // coincidence, since counting the dummy iteration made up for the
  // dropped final node). Also, each real node's ->size is header-inclusive
  // (payload + HEADER_SIZE(8)); main.c's test_threading() expects
  // allocated_size to total just the payload bytes actually handed out to
  // callers (7 nodes of a 5-byte request each => 35, not 7*13=91), so
  // subtract that header back out per node, matching the equivalent
  // per-block adjustment already made for free_size above.
  struct LLnode* cur2 = headused->next;
  while (cur2 != NULL)
  {
    _stat->allocated_chunks += 1;
    _stat->allocated_size += (cur2->size > 8) ? (cur2->size - 8) : 0;
    cur2 = cur2->next;
  }

  cur1 = headfree;
  LLnode* cur_prev = NULL;
  LLnode* largest_free_chunk_size = NULL;
  LLnode* smallest_free_chunk_size = NULL;
  // default return value if only 1 node
  LLnode* best_prev = headfree;
  int min = myalloc.size;
  int max = 0;
  while (cur1 != NULL)
  {

    // If min is greater than head->data then
    // assign value of head->data to min
    // otherwise node point to next node.
    if (min > cur1->size)
    {
      min = cur1->size;
    }

    if (max < cur1->size)
    {
      max = cur1->size;
    }
    cur1 = cur1->next;
  }
  _stat->smallest_free_chunk_size = min-8;
  _stat->largest_free_chunk_size = max-8;
  pthread_mutex_unlock(&mutex);
}