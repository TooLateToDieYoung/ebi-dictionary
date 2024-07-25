#ifndef __EBI_DICTIONARY_H
#define __EBI_DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include "ebi-memory-pool.h"

typedef struct dict_s dict_s;

typedef void (* dictFree_pf)(void *);

dict_s * 
dictMake(
    pool_s * const cpsPool,
    const dictFree_pf cpfFree
);

void
dictFree(
    void * pvThis
);

void *
dictAccess(
    dict_s * const cpsThis,
    const char * const cpcnKey
);

dict_s *
dictInsert(
    dict_s * const cpsThis,
    const char * const cpcnKey,
    void * const cpvVal
);

dict_s *
dictRemove(
    dict_s * const cpsThis,
    const char * const cpcnKey
);

dict_s *
dictChange(
    dict_s * const cpsThis,
    const char * const cpcnKey,
    void * const cpvVal
);

size_t
dictLength(
    const dict_s * const cpcsThis
);

const char *
dictGetRootKey(
    const dict_s * const cpcsThis
);

const char *
dictGetNextKey(
    const dict_s * const cpcsThis,
    const char * const cpcnCurrKey
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EBI_DICTIONARY_H */
