#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>
#include "threads/palloc.h"

/*project3*/
#include "include/lib/kernel/list.h"
#include "include/lib/kernel/hash.h"

enum vm_type {
	/* page not initialized */
	VM_UNINIT = 0,
	/* page not related to the file, aka anonymous page */
	VM_ANON = 1,
	/* page that realated to the file */
	VM_FILE = 2,
	/* page that hold the page cache, for project 4 */
	VM_PAGE_CACHE = 3,

	/* Bit flags to store state */

	/* Auxillary bit flag marker for store information. You can add more
	 * markers, until the value is fit in the int. */
	VM_MARKER_0 = (1 << 3),
	VM_MARKER_1 = (1 << 4),

	/* DO NOT EXCEED THIS VALUE. */
	VM_MARKER_END = (1 << 31),
};

#include "vm/uninit.h"
#include "vm/anon.h"
#include "vm/file.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type) & 7)

/* The representation of "page".
 * This is kind of "parent class", which has four "child class"es, which are
 * uninit_page, file_page, anon_page, and page cache (project4).
 * DO NOT REMOVE/MODIFY PREDEFINED MEMBER OF THIS STRUCTURE. */
 /*
 이 구조체에는 페이지 작업, 가상 주소 및 물리 프레임이 있음, 또한 연합필드도 있음
 */
struct page {
	const struct page_operations *operations;
	void *va;             /* 사용자 공간 주소*/
	struct frame *frame;   /* 프레임에 대한 역참조 */

	/* Your implementation */
	struct hash_elem hash_elem;
/*	 각 유형의 데이터가 연합(union)에 바인딩됩니다.
	 각 함수는 현재의 연합을 자동으로 감지합니다.
	 연합은 다른 유형의 데이터 메모리르 역역에 저장할 수 있는 특수한 데이터 유형,
	 여러 멤버가 있지만 한 번에 하나의 멤버만 값을 포함할 수 있음, 이것은 시스템
	 에서 페이지가 초기화 되지않은 페이지, 익명 페이지, 파일페이지, 또는 페이지 캐시중 하나
*/
	union {
		struct uninit_page uninit;
		struct anon_page anon; // 페이지가 익명 페이지인 경우 , 페이지 구조체에 struct anon_page anon 필드가 멤버 중 하나로 들어가게 됨, anon_page에는 익명 페이지에 필요한 정보가 포함
		struct file_page file;
#ifdef EFILESYS
		struct page_cache page_cache;
#endif
	};
};

/* The representation of "frame" */
/*frame 테이블
page를 가상 주소 공간에 할당하기위해 frame table을 가지고 있다.
프레임 테이블에는 비어있는 프레임, 사용중인 프레임, 총ㅇ 프레임과 같은 정보를 갖고있음
fram_table은 리스트로 구현한다.  frame은 전역적으로 관리하면서 갯수도 어마어마하게
많을 거기 때문에 linked list로 구현, 전역적으로 구현
*/
struct frame {
	void *kva;
	struct page *page;

	struct list_elem frame_elem;//리스트 형태로 구현하기떄문에 list_elem을 추가,frame_table을 순회하기 위한 용도
};

struct aux{

};

/* The function table for page operations.
 * This is one way of implementing "interface" in C.
 * Put the table of "method" into the struct's member, and
 * call it whenever you needed. */
struct page_operations {
	bool (*swap_in) (struct page *, void *);
	bool (*swap_out) (struct page *);
	void (*destroy) (struct page *); // page ->operation -> destroy
	enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in ((page), v)
#define swap_out(page) (page)->operations->swap_out (page)
#define destroy(page) \
	if ((page)->operations->destroy) (page)->operations->destroy (page)

/* Representation of current process's memory space.
 * We don't want to force you to obey any specific design for this struct.
 * All designs up to you for this. */
struct supplemental_page_table {
	struct hash spt_hash;//hash 테이블 사용
};

#include "threads/thread.h"
void supplemental_page_table_init (struct supplemental_page_table *spt);
bool supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src);
void supplemental_page_table_kill (struct supplemental_page_table *spt);
struct page *spt_find_page (struct supplemental_page_table *spt,
		void *va);
bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
void spt_remove_page (struct supplemental_page_table *spt, struct page *page);

void vm_init (void);
bool vm_try_handle_fault (struct intr_frame *f, void *addr, bool user,
		bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
	vm_alloc_page_with_initializer ((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage,
		bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page (struct page *page);
bool vm_claim_page (void *va);
enum vm_type page_get_type (struct page *page);
unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED);
bool page_less (const struct hash_elem *a_,const struct hash_elem *b_, void *aux UNUSED);
#endif  /* VM_VM_H */
