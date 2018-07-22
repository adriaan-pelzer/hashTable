#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "hashTable.h"

char *str_lowercase ( const char *string ) {
    char *rc = NULL, *_rc = NULL;
    uint16_t i = 0;

    if ( ( _rc = strndup ( string, strlen ( string ) ) ) == NULL ) {
        goto over;
    }

    for ( i = 0; i < strlen ( _rc ); i++ )
        _rc[i] = (char) tolower ( (int) _rc[i] );

    rc = _rc;
over:
    if ( rc != _rc && _rc != NULL )
        free ( _rc );

    return rc;
}

uint32_t murmur3_32 ( const uint8_t* key, size_t len, uint32_t seed ) {
    uint32_t h = seed;

    if (len > 3) {
        const uint32_t* key_x4 = (const uint32_t*) key;
        size_t i = len >> 2;
        do {
            uint32_t k = *key_x4++;
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = (h * 5) + 0xe6546b64;
        } while (--i);
        key = (const uint8_t*) key_x4;
    }

    if (len & 3) {
        size_t i = len & 3;
        uint32_t k = 0;
        key = &key[i - 1];
        do {
            k <<= 8;
            k |= *key--;
        } while (--i);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t get_native_key ( const char *key ) {
    char *lowerKey = str_lowercase ( key );
    uint32_t nativeKey = murmur3_32 ( ( const uint8_t * ) lowerKey, strlen ( lowerKey ), 0 );
    free ( lowerKey );
    return nativeKey;
};

size_t find_new_slot ( struct hashTableEntry *entries, uint32_t nativeKey, size_t bottom, size_t top ) {
    size_t middle = bottom + ( top - bottom ) / 2;

    if ( entries == NULL ) return 0;

    if ( entries[middle].k == nativeKey ) return middle;
    if ( entries[top].k == nativeKey ) return top;
    if ( entries[bottom].k == nativeKey ) return bottom;

    if ( top - bottom == 1 || top - bottom == 0 ) return top;

    if ( entries[middle].k < nativeKey ) return find_new_slot ( entries, nativeKey, middle, top );
    if ( entries[middle].k > nativeKey ) return find_new_slot ( entries, nativeKey, bottom, middle );

    fprintf ( stderr, "find_new_slot falls through, which should never happen!" );

    return middle;
}

enum hashTable_rc hashTable_add_entry ( struct hashTable *hashTable, const char *key, void *value ) {
    uint32_t nativeKey = get_native_key ( key );
    size_t i, newSlot = find_new_slot ( hashTable->entries, nativeKey, 0, hashTable->size - 1 );

    if ( hashTable->entries && hashTable->entries[newSlot].k == nativeKey )
        return HT_EXISTS;

    if ( ( hashTable->entries = realloc ( hashTable->entries, sizeof ( struct hashTableEntry ) * ( hashTable->size + 1 ) ) ) == NULL )
        return HT_FAILURE;

    for ( i = hashTable->size; i > newSlot; i-- ) {
        hashTable->entries[i].k = hashTable->entries[i-1].k;
        hashTable->entries[i].v = hashTable->entries[i-1].v;
    }

    hashTable->entries[newSlot].k = nativeKey;
    hashTable->entries[newSlot].v = value;
    hashTable->size++;
    return HT_SUCCESS;
}

struct hashTableEntry *hashTable_find_entry ( struct hashTable *hashTable, const char *key ) {
    uint32_t nativeKey = get_native_key ( key );
    size_t slotOfInterest = find_new_slot ( hashTable->entries, nativeKey, 0, hashTable->size - 1 );
    
    if ( hashTable->entries && hashTable->entries[slotOfInterest].k == nativeKey )
        return &hashTable->entries[slotOfInterest];

    return NULL;
}

enum hashTable_rc remove_entry ( struct hashTable *hashTable, size_t slot ) {
    size_t i = 0;

    for ( i = slot; i < hashTable->size - 1; i++ ) {
        hashTable->entries[i].k = hashTable->entries[i+1].k;
        hashTable->entries[i].v = hashTable->entries[i+1].v;
    }

    if ( ( hashTable->entries = realloc ( hashTable->entries, sizeof ( struct hashTableEntry ) * ( hashTable->size - 1 ) ) ) == NULL )
        return HT_FAILURE;

    return HT_SUCCESS;
}

enum hashTable_rc hashTable_remove_entry ( struct hashTable *hashTable, const char *key ) {
    uint32_t nativeKey = get_native_key ( key );
    size_t slotOfInterest = find_new_slot ( hashTable->entries, nativeKey, 0, hashTable->size - 1 );

    if ( hashTable->entries[slotOfInterest].k == nativeKey )
        return remove_entry ( hashTable, slotOfInterest );

    return HT_NOTEXISTS;
}

enum hashTable_rc hashTable_clear_entries ( struct hashTable *hashTable ) {
    if ( hashTable->entries ) free ( hashTable->entries );
    hashTable->entries = NULL;
    return HT_SUCCESS;
}

enum hashTable_rc hashTable_free ( struct hashTable *hashTable ) {
    if ( hashTable ) {
        if ( hashTable_clear_entries ( hashTable ) != HT_SUCCESS )
            return HT_FAILURE;
        free ( hashTable );
    }
    return HT_SUCCESS;
}
