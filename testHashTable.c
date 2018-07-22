#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <stdint.h>
#include <hashTable.h>

#define DAEMON_NAME "hashTable"
#define BUF_SIZE 1024

int main ( int argc, char **argv ) {
    int rc = EXIT_FAILURE;
    char *filename = NULL;
    FILE *infile = NULL;
    char buffer[BUF_SIZE];
    char *word;
    uint16_t chunkStart = 0;
    uint16_t readBytes = 0;
    struct hashTable hashTable;
    struct hashTableEntry *entry = NULL;

    memset ( &hashTable, 0, sizeof ( struct hashTable ) );

    if ( argc < 2 ) {
        fprintf ( stderr, "Usage: %s word [debug]\n", argv[0] );
        goto over;
    }

    if ( argc > 2 && ! strncmp ( argv[2], "debug", 5 ) ) {
        setlogmask ( LOG_UPTO ( LOG_DEBUG ) );
        openlog ( DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER );
    } else {
        setlogmask ( LOG_UPTO ( LOG_INFO ) );
        openlog ( DAEMON_NAME, LOG_CONS, LOG_USER );
    }

    filename = argv[1];

    if ( ( infile = fopen ( filename, "r" ) ) == NULL ) {
        syslog ( LOG_ERR, "Cannot open file %s for reading: %s", filename, strerror ( errno ) );
        goto over;
    }

    while ( ( readBytes = fread ( (void *)((uint64_t) &buffer + chunkStart), 1, BUF_SIZE - chunkStart, infile ) ) > 0 ) {
        uint16_t charIndex = chunkStart, i, wordStart = 0;

        while ( charIndex < chunkStart + readBytes ) {
            if ( buffer[charIndex] == '\n' ) {
                buffer[charIndex] = '\0';
                word = buffer + wordStart;
                hashTable_add_entry ( &hashTable, word, word );
                printf ( "%s\n", word );
                wordStart = charIndex + 1;
            }

            charIndex++;
        }

        if ( readBytes == BUF_SIZE - chunkStart ) {
            for ( i = wordStart; i < BUF_SIZE; i++ )
                buffer[i - wordStart] = buffer[i];

            chunkStart = i - wordStart;
        } else {
            break;
        }
    }

    if ( ( hashTable_add_entry ( &hashTable, word, word ) ) != HT_EXISTS ) {
        syslog ( LOG_ERR, "Adding the same word twice doesn't return HT_EXISTS" );
        goto over;
    }

    rewind ( infile );
    chunkStart = 0;
    
    while ( ( readBytes = fread ( (void *)((uint64_t) &buffer + chunkStart), 1, BUF_SIZE - chunkStart, infile ) ) > 0 ) {
        uint16_t charIndex = chunkStart, i, wordStart = 0;

        while ( charIndex < chunkStart + readBytes ) {
            if ( buffer[charIndex] == '\n' ) {
                buffer[charIndex] = '\0';
                word = buffer + wordStart;
                if ( ( entry = hashTable_find_entry ( &hashTable, word ) ) == NULL ) {
                    syslog ( LOG_ERR, "Cannot find entry: %s", word );
                    goto over;
                }

                printf ( "%s\n", (char *) entry->v );
                wordStart = charIndex + 1;
            }

            charIndex++;
        }

        if ( readBytes == BUF_SIZE - chunkStart ) {
            for ( i = wordStart; i < BUF_SIZE; i++ )
                buffer[i - wordStart] = buffer[i];

            chunkStart = i - wordStart;
        } else {
            break;
        }
    }

    rewind ( infile );
    chunkStart = 0;
    
    while ( ( readBytes = fread ( (void *)((uint64_t) &buffer + chunkStart), 1, BUF_SIZE - chunkStart, infile ) ) > 0 ) {
        uint16_t charIndex = chunkStart, i, wordStart = 0;

        while ( charIndex < chunkStart + readBytes ) {
            if ( buffer[charIndex] == '\n' ) {
                buffer[charIndex] = '\0';
                word = buffer + wordStart;
                if ( ( hashTable_remove_entry ( &hashTable, word ) ) != HT_SUCCESS ) {
                    syslog ( LOG_ERR, "Cannot remove entry: %s", word );
                    goto over;
                }
                wordStart = charIndex + 1;
            }

            charIndex++;
        }

        if ( readBytes == BUF_SIZE - chunkStart ) {
            for ( i = wordStart; i < BUF_SIZE; i++ )
                buffer[i - wordStart] = buffer[i];

            chunkStart = i - wordStart;
        } else {
            break;
        }
    }

    if ( ( hashTable_remove_entry ( &hashTable, word ) ) != HT_NOTEXISTS ) {
        syslog ( LOG_ERR, "Removing the same word twice doesn't return HT_NOTEXISTS" );
        goto over;
    }

    rc = EXIT_SUCCESS;
over:
    if ( hashTable.entries )
        free ( hashTable.entries );
    if ( infile )
        fclose ( infile );
    closelog ();

    return rc;
}
