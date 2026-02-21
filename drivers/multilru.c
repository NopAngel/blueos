
#include <include/printk.h>
#include <include/colors.h>
#include <include/multilru.h>
#include <stddef.h>
#define PAGE_ACTIVE   1
#define PAGE_INACTIVE 0


lru_list_t active_list = {0, 0, 0};
lru_list_t inactive_list = {0, 0, 0};

void lru_init() {
    active_list.head = active_list.tail = NULL;
    active_list.count = 0;
    inactive_list.head = inactive_list.tail = NULL;
    inactive_list.count = 0;
    printk(YELLOW, "LRU: Multi-Level LRU initialized (Active/Inactive pools)\n");
}
void list_add_head(lru_list_t *list, page_t *page) {
    page->next = list->head;
    page->prev = NULL;

    if (list->head != NULL) {
        list->head->prev = page;
    }
    list->head = page;

    if (list->tail == NULL) {
        list->tail = page;
    }
    list->count++;
}

void list_remove(lru_list_t *list, page_t *page) {
    if (page->prev != NULL) {
        page->prev->next = page->next;
    } else {
        list->head = page->next;
    }

    if (page->next != NULL) {
        page->next->prev = page->prev;
    } else {
        list->tail = page->prev;
    }

    page->next = NULL;
    page->prev = NULL;
    list->count--;
}

void demote_active_pages() {
    if (active_list.tail != NULL) {
        page_t *page = active_list.tail;
        list_remove(&active_list, page);
        
        page->status = PAGE_INACTIVE;
        page->referenced = 0;
        list_add_head(&inactive_list, page);
        
        printk(YELLOW, "LRU: Demoting page %d to Inactive list (overflow)\n", page->frame_number);
    }
}

void list_move_to_head(lru_list_t *list, page_t *page) {
    list_remove(list, page);
    list_add_head(list, page);
}


void lru_add_page(page_t *page) {
    page->status = PAGE_INACTIVE;
    page->referenced = 0;

    list_add_head(&inactive_list, page);
    
    printk(GRAY, "LRU: Page %d added to Inactive list.\n", page->frame_number);
}

void lru_touch_page(page_t *page) {
    if (page->status == PAGE_INACTIVE) {
        if (page->referenced) {
            list_remove(&inactive_list, page);
            page->status = PAGE_ACTIVE;
            page->referenced = 0;
            list_add_head(&active_list, page);
            printk(GREEN, "LRU: Page %d promoted to Active!\n", page->frame_number);
        } else {
            page->referenced = 1;
        }
    } else {
       
        list_move_to_head(&active_list, page);
    }
}

page_t* lru_evict_page() {
    if (inactive_list.count == 0) {
        demote_active_pages();
    }

    page_t *victim = inactive_list.tail;
    if (victim) {
        list_remove(&inactive_list, victim);
        printk(RED, "LRU: Evicting page %d from RAM\n", victim->frame_number);
        return victim;
    }
    return NULL; // Kernel Panic: Out of Memory
}