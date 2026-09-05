#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void execute_insert(char* input, Pager* pager, BTreeNode** index) {
    Row row;
    int assigned = sscanf(input, "INSERT %u %31s %31s", &row.id, row.username, row.email);
    if (assigned < 3) {
        printf("Syntax error: INSERT <id> <username> <email>\n");
        return;
    }

    uint32_t page_idx = pager->num_pages == 0 ? 0 : pager->num_pages - 1;
    Page* page = get_page(pager, page_idx);
    if (page->num_rows >= MAX_RECORDS_PER_PAGE) {
        page_idx++;
        page = get_page(pager, page_idx);
    }

    uint32_t offset = page->num_rows;
    page->rows[offset] = row;
    page->num_rows++;

    btree_insert(index, row.id, page_idx, offset);
    printf("Inserted row (id: %u) at page %u, offset %u.\n", row.id, page_idx, offset);
}

void execute_select(char* input, Pager* pager, BTreeNode* index) {
    uint32_t target_id;
    if (sscanf(input, "SELECT WHERE id = %u", &target_id) == 1) {
        uint32_t page_idx, row_offset;
        if (btree_search(index, target_id, &page_idx, &row_offset)) {
            Page* page = get_page(pager, page_idx);
            Row r = page->rows[row_offset];
            printf("Found: ID=%u, Username=%s, Email=%s\n", r.id, r.username, r.email);
        } else {
            printf("Row with id %u not found.\n", target_id);
        }
    } else if (strncmp(input, "SELECT", 6) == 0) {
        for (uint32_t p = 0; p < pager->num_pages; p++) {
            Page* page = get_page(pager, p);
            for (uint32_t r = 0; r < page->num_rows; r++) {
                Row row = page->rows[r];
                printf("(%u, %s, %s)\n", row.id, row.username, row.email);
            }
        }
    }
}

int main() {
    Pager* pager = pager_open("database.db");
    BTreeNode* index = NULL;
    char buffer[256];

    printf("Custom C Database Engine Initialized. Type 'EXIT' to quit.\n");
    while (1) {
        printf("db > ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "EXIT") == 0) break;
        if (strncmp(buffer, "INSERT", 6) == 0) {
            execute_insert(buffer, pager, &index);
        } else if (strncmp(buffer, "SELECT", 6) == 0) {
            execute_select(buffer, pager, index);
        } else {
            printf("Unrecognized command.\n");
        }
    }

    pager_close(pager);
    return 0;
}