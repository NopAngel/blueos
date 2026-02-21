#ifndef LRU_H
#define LRU_H


#define PAGE_INACTIVE 0
#define PAGE_ACTIVE   1


typedef struct page {
    unsigned int frame_number;
    int status;                
    int referenced;            
    struct page *next;
    struct page *prev;
} page_t;


typedef struct {
    page_t *head;
    page_t *tail;
    int count;
} lru_list_t;

extern lru_list_t active_list;
extern lru_list_t inactive_list;

void list_add_head(lru_list_t *list, page_t *page);
void list_remove(lru_list_t *list, page_t *page);
void list_move_to_head(lru_list_t *list, page_t *page);


void lru_init();

void lru_add_page(page_t *page);

void lru_touch_page(page_t *page);

page_t* lru_evict_page();


void lru_demote_active();

#endif