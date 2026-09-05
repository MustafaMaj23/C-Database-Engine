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

// Splits a full child node (which has reached max capacity of 3 keys)
static void btree_split_child(BTreeNode* parent, int i, BTreeNode* y) {
    BTreeNode* z = btree_create_node(y->is_leaf);
    
    // y has 3 keys (indices 0, 1, 2). Median is at index 1.
    // z gets the key after the median (index 2).
    z->num_keys = 1;
    z->keys[0] = y->keys[2];
    z->page_indices[0] = y->page_indices[2];
    z->row_offsets[0] = y->row_offsets[2];

    // If y is not a leaf, split its children as well (y keeps 0, 1; z gets 2, 3)
    if (!y->is_leaf) {
        z->children[0] = y->children[2];
        z->children[1] = y->children[3];
        y->children[2] = NULL;
        y->children[3] = NULL;
    }

    y->num_keys = 1; // y keeps only its first key (index 0)

    // Make room in parent for the new child pointer
    for (int j = parent->num_keys; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = z;

    // Make room in parent for the median key from y
    for (int j = parent->num_keys - 1; j >= i; j--) {
        parent->keys[j + 1] = parent->keys[j];
        parent->page_indices[j + 1] = parent->page_indices[j];
        parent->row_offsets[j + 1] = parent->row_offsets[j];
    }

    parent->keys[i] = y->keys[1];
    parent->page_indices[i] = y->page_indices[1];
    parent->row_offsets[i] = y->row_offsets[1];

    parent->num_keys++;
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
        
        // If the child we are about to descend into is full, split it first
        if (node->children[i]->num_keys == 3) {
            btree_split_child(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        btree_insert_non_full(node->children[i], key, page_idx, row_offset);
    }
}

void btree_insert(BTreeNode** root, uint32_t key, uint32_t page_idx, uint32_t row_offset) {
    BTreeNode* r = *root;
    if (r->num_keys == 3) {
        // Root is full; height grows by 1
        BTreeNode* s = btree_create_node(false);
        *root = s;
        s->children[0] = r;
        btree_split_child(s, 0, r);
        btree_insert_non_full(s, key, page_idx, row_offset);
    } else {
        btree_insert_non_full(r, key, page_idx, row_offset);
    }
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
