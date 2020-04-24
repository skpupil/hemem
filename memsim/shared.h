#ifndef SHARED_H
#define SHARED_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

// Handy size macros
#define KB(x)		(((uint64_t)x) * 1024)
#define MB(x)		(KB(x) * 1024)
#define GB(x)		(MB(x) * 1024)

// Page sizes
#define BASE_PAGE_SIZE	KB(4)
#define HUGE_PAGE_SIZE	MB(2)
#define GIGA_PAGE_SIZE	GB(1)

// Page offset masks
#define BASE_PAGE_MASK	(BASE_PAGE_SIZE - 1)
#define HUGE_PAGE_MASK	(HUGE_PAGE_SIZE - 1)
#define GIGA_PAGE_MASK	(GIGA_PAGE_SIZE - 1)

// Page frame number masks
#define BASE_PFN_MASK	(BASE_PAGE_MASK ^ UINT64_MAX)
#define HUGE_PFN_MASK	(HUGE_PAGE_MASK ^ UINT64_MAX)
#define GIGA_PFN_MASK	(GIGA_PAGE_MASK ^ UINT64_MAX)

// Physical memory sizes in bytes
#define FASTMEM_SIZE	GB(10)
#define SLOWMEM_SIZE	GB(100)
#define CACHELINE_SIZE	64

// Simulated execution times in ns
#define TIME_PAGEFAULT		2000		// pagefault interrupt
#define TIME_PAGEWALK		200		// 1 level of pagewalk
#define TIME_SLOWMOVE		2000		// Move to slow memory
#define TIME_FASTMOVE		1000		// Move to fast memory
#define TIME_TLBSHOOTDOWN	4000		// TLB shootdown

// From Intel memory latency checker
#define TIME_FASTMEM_READ	82
#define TIME_FASTMEM_WRITE	82

#define TIME_SLOWMEM_READ	1000		// From DCPMM QoS slides
#define TIME_SLOWMEM_WRITE	1000		// maybe worse?

// Fake offset for slowmem in physical memory
#define SLOWMEM_BIT	((uint64_t)1 << 63)
#define SLOWMEM_MASK	(((uint64_t)1 << 63) - 1)

#ifdef LOG_DEBUG
#	define LOG(str, ...)	fprintf(stderr, "%.2f " str, (double)runtime / 1000000000.0, ##__VA_ARGS__)
#else
#	define LOG(std, ...)	while(0) {}
#endif

// Memory access type
enum access_type {
  TYPE_READ,
  TYPE_WRITE,
};

enum memtypes {
  FASTMEM = 0,
  SLOWMEM = 1,
  NMEMTYPES,
};

enum pagetypes {
  GIGA = 0,
  HUGE,
  BASE,
  NPAGETYPES
};

// Page table entry
struct pte {
  uint64_t addr;		// Physical address
  struct pte *next;		// Next page table pointer

  // Hardware bits
  _Atomic bool present;
  bool readonly;
  bool accessed;
  bool modified;
  _Atomic bool pagemap;		// This PTE maps a page

  // OS bits
  _Atomic bool migration;	// Range is under migration
};

// readonly is set if the page was a write to a read-only
// page. Otherwise, it was any access to a non-present page.
void pagefault(uint64_t addr, bool readonly);
void tlb_shootdown(uint64_t addr);
void mmgr_init(void);
void add_runtime(size_t delta);
void memsim_nanosleep(size_t sleeptime);

// XXX: Debug
int listnum(struct pte *pte);

extern _Atomic size_t runtime;
extern struct pte *cr3;
extern _Atomic size_t memsim_timebound;
extern __thread bool memsim_timebound_thread;

static inline uint64_t page_size(enum pagetypes pt)
{
  switch(pt) {
  case GIGA: return GIGA_PAGE_SIZE;
  case HUGE: return HUGE_PAGE_SIZE;
  case BASE: return BASE_PAGE_SIZE;
  default: assert(!"Unknown page type");
  }
}

static inline uint64_t pfn_mask(enum pagetypes pt)
{
  switch(pt) {
  case GIGA: return GIGA_PFN_MASK;
  case HUGE: return HUGE_PFN_MASK;
  case BASE: return BASE_PFN_MASK;
  default: assert(!"Unknown page type");
  }
}

#endif
