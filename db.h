#ifndef DB_H
#define DB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 100
#define COLUMN_NAME_LEN 32
#define MAX_RECORDS_PER_PAGE 32

typedef struct {
    uint32_t id;
    char username[COLUMN_NAME_LEN];
    char email[COLUMN_NAME_LEN];
} Row;

typedef struct {
    uint32_t num_rows;
    Row rows[MAX_RECORDS_PER_PAGE];
} Page;

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    uint32_t num_pages;
    Page* pages[MAX_PAGES];
} Pager;

typedef struct BTreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[3];
    uint32_t page_indices[3];
    uint32_t row_offsets[3];
    struct BTreeNode* children[4];
} BTreeNode;

// Buffer & storage manager
Pager* pager_open(const char* filename);
Page* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num);
void pager_close(Pager* pager);

// B-tree indexing
BTreeNode* btree_create_node(bool is_leaf);
void btree_insert(BTreeNode** root, uint32_t key, uint32_t page_idx, uint32_t row_offset);
bool btree_search(BTreeNode* root, uint32_t key, uint32_t* page_idx, uint32_t* row_offset);

#endif