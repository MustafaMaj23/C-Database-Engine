#include "db.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

Pager* pager_open(const char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd < 0) {
        perror("Unable to open database file");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->num_pages = (file_length / PAGE_SIZE);

    for (uint32_t i = 0; i < MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }
    return pager;
}

Page* get_page(Pager* pager, uint32_t page_num) {
    if (page_num >= MAX_PAGES) return NULL;
    if (pager->pages[page_num] == NULL) {
        Page* page = malloc(sizeof(Page));
        memset(page, 0, sizeof(Page));
        uint32_t num_pages = pager->file_length / PAGE_SIZE;

        if (pager->file_length % PAGE_SIZE) num_pages += 1;

        if (page_num < num_pages) {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            (void)bytes_read;
        }
        pager->pages[page_num] = page;
        if (page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }
    return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num) {
    if (pager->pages[page_num] == NULL) return;
    lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);
}

void pager_close(Pager* pager) {
    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i] != NULL) {
            pager_flush(pager, i);
            free(pager->pages[i]);
            pager->pages[i] = NULL;
        }
    }
    close(pager->file_descriptor);
    free(pager);
}