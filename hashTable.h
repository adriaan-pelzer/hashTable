#include <stdlib.h>
#include <stdint.h>

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

enum hashTable_rc {
    HT_FAILURE = -1,
    HT_SUCCESS = 0,
    HT_EXISTS = 1,
    HT_NOTEXISTS = 2
};

struct hashTableEntry {
    uint32_t k;
    void *v;
};

struct hashTable {
    size_t size;
    struct hashTableEntry *entries;
};

enum hashTable_rc hashTable_add_entry ( struct hashTable *hashTable, const char *key, void *value );
struct hashTableEntry *hashTable_find_entry ( struct hashTable *hashTable, const char *key );
enum hashTable_rc hashTable_remove_entry ( struct hashTable *hashTable, const char *key );
enum hashTable_rc hashTable_clear_entries ( struct hashTable *hashTable );
enum hashTable_rc hashTable_free ( struct hashTable *hashTable );

#endif
