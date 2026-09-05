#include "db.h"
#include <stdlib.h>

BTreeNode* btree_create_node(bool is_leaf) {
    BTreeNode* node = malloc(sizeof(BTreeNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    for (int i = 0; i < 4; i++) {
        node->children[i] = NULL;
    }
    return node;
}

static void btree_insert_non_full(BTreeNode* node, uint32_t key, uint32_t page_idx, uint32_t row_offset) {
    int i = node->num_keys - 1;
    if (node->is_leaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            node->page_indices[i + 1] = node->page_indices[i];
            node->row_offsets[i + 1] = node->row_offsets[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->page_indices[i + 1] = page_idx;
        node->row_offsets[i + 1] = row_offset;
        node->num_keys++;
    } else {
        while (i >= 0 && node->keys[i] > key) i--;
        i++;
        btree_insert_non_full(node->children[i], key, page_idx, row_offset);
    }
}

void btree_insert(BTreeNode** root, uint32_t key, uint32_t page_idx, uint32_t row_offset) {
    if (*root == NULL) {
        *root = btree_create_node(true);
    }
    btree_insert_non_full(*root, key, page_idx, row_offset);
}

bool btree_search(BTreeNode* root, uint32_t key, uint32_t* page_idx, uint32_t* row_offset) {
    if (root == NULL) return false;
    int i = 0;
    while (i < root->num_keys && key > root->keys[i]) i++;
    if (i < root->num_keys && key == root->keys[i]) {
        *page_idx = root->page_indices[i];
        *row_offset = root->row_offsets[i];
        return true;
    }
    if (root->is_leaf) return false;
    return btree_search(root->children[i], key, page_idx, row_offset);
}