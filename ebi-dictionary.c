#include "ebi-dictionary.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct node_s node_s;
struct node_s {
    char * pnKey;
    void * pvVal;

    node_s * psP;
    node_s * psL;
    node_s * psR;
    size_t zHeight;
};

struct dict_s {
    pool_s * psPool;
    dictFree_pf pfFree;

    node_s * psRoot;
    size_t zSize;
};

static 
size_t
_Height(
    node_s * const cpsThis
) {
    return ( NULL != cpsThis ) ? ( cpsThis->zHeight ) : ( 0 ) ;
}

static 
node_s *
_Access(
    node_s * const cpsThis,
    const char * const cpcnKey,
    node_s ** ppsPrev
) {
    int nRet = 0;
    node_s * psCurr = cpsThis;
    node_s * psPrev = NULL;

    if ( NULL == cpsThis || NULL == cpcnKey )
    {
        return NULL;
    }

    do {
        psPrev = psCurr->psP;
        nRet = strcmp(psCurr->pnKey, cpcnKey);
        if ( 0 == nRet )
        {
            break;
        }
        else if ( nRet > 0 )
        {
            psCurr = psCurr->psL;
        }
        else
        {
            psCurr = psCurr->psR;
        }
    } while ( NULL != psCurr );

    if ( NULL != ppsPrev )
    {
        *ppsPrev = psPrev;
    }
    return psCurr;
}

static
node_s *
_Organize(
    node_s * const cpsThis
) {
    node_s * psCurr = cpsThis;
    node_s * psPrev = NULL;
    node_s * psTemp = NULL;
    size_t zHL = 0;
    size_t zHR = 0;
    char nDiff = 0;

    while ( NULL != psCurr )
    {
        zHL = _Height(psCurr->psL);
        zHR = _Height(psCurr->psR);
        nDiff = (char)( zHL - zHR );
        switch ( nDiff )
        {
        case -2:
            psTemp = psCurr->psR;
            psPrev = psCurr->psP;

            /* temp <=> prev */
            psTemp->psP = psPrev;
            if ( NULL != psPrev )
            {
                if ( psPrev->psL == psCurr )
                {
                    psPrev->psL = psTemp;
                }
                else
                {
                    psPrev->psR = psTemp;
                }
            }
        
            /* temp <=> curr */
            psCurr->psP = psTemp;
            psTemp->psL = psCurr;

            /* temp <=> curr->r */
            psCurr->psR = psTemp->psL;
            if ( NULL != psTemp->psL )
            {
                psTemp->psL->psP = psCurr;
            }

            psCurr = psTemp->psP;
            break;

        case +2:
            psTemp = psCurr->psL;
            psPrev = psCurr->psP;

            /* temp <=> prev */
            psTemp->psP = psPrev;
            if ( NULL != psPrev )
            {
                if ( psPrev->psL == psCurr )
                {
                    psPrev->psL = psTemp;
                }
                else
                {
                    psPrev->psR = psTemp;
                }
            }
        
            /* temp <=> curr */
            psCurr->psP = psTemp;
            psTemp->psR = psCurr;

            /* temp <=> curr->l */
            psCurr->psL = psTemp->psR;
            if ( NULL != psTemp->psR )
            {
                psTemp->psR->psP = psCurr;
            }

            psCurr = psTemp->psP;
            break;

        default:
            psTemp = psCurr;
            psTemp->zHeight = ( zHL > zHR ) ? ( zHL + 1 ) : ( zHR + 1 ) ;
            psCurr = psTemp->psP;
            break;
        }
    }

    return psTemp; 
}

dict_s * 
dictMake(
    pool_s * const cpsPool,
    const dictFree_pf cpfFree
)  {
assert( NULL != cpfFree );

    dict_s * const cpsThis = ( NULL != cpsPool ) ? 
        (dict_s *)poolMalloc(cpsPool, sizeof(dict_s)) :
        (dict_s *)malloc(sizeof(dict_s)) ;

    if ( NULL != cpsThis )
    {
        cpsThis->psPool = cpsPool;
        cpsThis->pfFree = cpfFree;
        cpsThis->psRoot = NULL;
        cpsThis->zSize = 0;
    }

    return cpsThis;
}

void
dictFree(
    void * pvThis
) {
    dict_s * const cpsThis = (dict_s *)( pvThis );

    while ( dictLength(cpsThis) > 0 )
    {
        (void)dictRemove(cpsThis, dictGetRootKey(cpsThis));
    }

    if ( NULL != cpsThis->psPool )
    {
        poolReturn(cpsThis);
    }
    else
    {
        free(cpsThis);
    }
}

void *
dictAccess(
    dict_s * const cpsThis,
    const char * const cpcnKey
) {
    node_s * psCurr = NULL;

    if ( NULL == cpsThis || NULL == cpcnKey )
    {
        return NULL;
    }

    psCurr = _Access(cpsThis->psRoot, cpcnKey, NULL);

    return ( NULL != psCurr ) ? ( psCurr->pvVal ) : ( NULL ) ;
}

dict_s *
dictInsert(
    dict_s * const cpsThis,
    const char * const cpcnKey,
    void * const cpvVal
) {
    node_s * psCurr = NULL;
    node_s * psPrev = NULL;
    size_t zKeyLen = 0;

    if ( NULL == cpsThis || NULL == cpcnKey )
    {
        return NULL;
    }

    psCurr = _Access(cpsThis->psRoot, cpcnKey, &psPrev);

    if ( NULL != psCurr )
    {
        return dictChange(cpsThis, cpcnKey, cpvVal);
    }

    psCurr = ( NULL != cpsThis->psPool ) ? 
        (node_s *)poolMalloc(cpsThis->psPool, sizeof(node_s)) :
        (node_s *)malloc(sizeof(node_s)) ;
    
    if ( NULL == psCurr )
    {
        return NULL;
    }

    zKeyLen = 1 + strlen(cpcnKey);

    psCurr->pnKey = ( NULL != cpsThis->psPool ) ? 
        (node_s *)poolMalloc(cpsThis->psPool, zKeyLen) :
        (node_s *)malloc(zKeyLen) ;
    
    if ( NULL == psCurr->pnKey )
    {
        if ( NULL != cpsThis->psPool )
        {
            poolReturn(psCurr);
        }
        else
        {
            free(psCurr);
        }
        return NULL;
    }

    (void)memcpy(psCurr->pnKey, cpcnKey, zKeyLen);
    psCurr->pvVal = cpcnKey;

    if ( NULL == psPrev )
    {
        cpsThis->psRoot = psCurr;
        psCurr->psL = NULL;
        psCurr->psR = NULL;
        psCurr->psP = NULL;
    }
    else
    {
        psCurr->psL = NULL;
        psCurr->psR = NULL;
        psCurr->psP = psPrev;
        if ( strcmp(psPrev->pnKey, psCurr->pnKey) > 0 )
        {
            psPrev->psL = psCurr;
        }
        else
        {
            psPrev->psR = psCurr;
        }
    }

    cpsThis->psRoot = _Organize(psCurr);

    cpsThis->zSize++;

    return cpsThis;
}

dict_s *
dictRemove(
    dict_s * const cpsThis,
    const char * const cpcnKey
) {
    node_s * psCurr = NULL;
    node_s * psTemp = NULL;
    node_s * psDrop = NULL;

    if ( NULL == cpsThis || NULL == cpcnKey )
    {
        return NULL;
    }

    psCurr = _Access(cpsThis->psRoot, cpcnKey, NULL);

    if ( NULL == psCurr )
    {
        return cpsThis; // ? Not found
    }
    else
    {
        psDrop = psCurr;
    }

    if ( NULL == psDrop->psL && NULL == psDrop->psR ) // ? is the tail
    {
        if ( NULL == psDrop->psP )
        {
            cpsThis->psRoot = NULL;
        }
        else if ( psDrop->psP->psL == psDrop )
        {
            psDrop->psP->psL = NULL;
        }
        else
        {
            psDrop->psP->psR = NULL;
        }
    }
    else 
    {
        if ( _Height(psDrop->psL) > _Height(psDrop->psR) )
        {
            // ? In the left subtree, find the largest node to replace the data to be dropped.
            for ( psDrop = psDrop->psL; NULL != psDrop->psR; psDrop = psDrop->psR ) { }

            if ( psDrop->psP->psL == psDrop )
            {
                psDrop->psP->psL = psDrop->psL;
            }
            else
            {
                psDrop->psP->psR = psDrop->psL;
            }

            if ( NULL != psDrop->psL )
            {
                psDrop->psL->psP = psDrop->psP;
            }
        }
        else
        {
            // ? In the right subtree, find the smallest node to replace the data to be dropped.
            for ( psDrop = psDrop->psR; NULL != psDrop->psL; psDrop = psDrop->psL ) { }

            if ( psDrop->psP->psR == psDrop )
            {
                psDrop->psP->psR = psDrop->psR;
            }
            else
            {
                psDrop->psP->psL = psDrop->psR;
            }

            if ( NULL != psDrop->psR )
            {
                psDrop->psR->psP = psDrop->psP;
            }
        }

        /*  ? Exchange data:
            - The link relationship of psCurr is still useful, but the data in psCurr is not.
            - On the other hand, the data in the psDrop node is useful, but the node will be removed from the tree. */
        psTemp->pnKey = psDrop->pnKey;
        psTemp->pvVal = psDrop->pvVal;
        psDrop->pnKey = psCurr->pnKey;
        psDrop->pvVal = psCurr->pvVal;
        psCurr->pnKey = psTemp->pnKey;
        psCurr->pvVal = psTemp->pvVal;
    }

    if ( NULL != cpsThis->psRoot )
    {
        cpsThis->psRoot = _Organize(psDrop->psP);
    }

    cpsThis->pfFree(psDrop->pvVal);
    if ( NULL != cpsThis->psPool )
    {
        poolReturn(psDrop->pnKey);
        poolReturn(psDrop);
    }
    else
    {
        free(psDrop->pnKey);
        free(psDrop);
    }

    cpsThis->zSize--;

    return cpsThis;
}

dict_s *
dictChange(
    dict_s * const cpsThis,
    const char * const cpcnKey,
    void * const cpvVal
) {
    node_s * psCurr = NULL;

    if ( NULL == cpsThis || NULL == cpcnKey )
    {
        return NULL;
    }

    psCurr = _Access(cpsThis->psRoot, cpcnKey, NULL);

    if ( NULL == psCurr )
    {
        return dictInsert(cpsThis, cpcnKey, cpvVal);
    }

    cpsThis->pfFree(psCurr->pvVal);
    psCurr->pvVal = cpvVal;

    return cpsThis;
}

size_t
dictLength(
    const dict_s * const cpcsThis
) {
    return ( NULL != cpcsThis ) ? ( cpcsThis->zSize ) : ( 0 ) ; 
}

const char *
dictGetRootKey(
    const dict_s * const cpcsThis
) {
    return ( NULL != cpcsThis ) ? ( cpcsThis->psRoot->pnKey ) : ( NULL ) ; 
}

const char *
dictGetNextKey(
    const dict_s * const cpcsThis,
    const char * const cpcnCurrKey
) {
    node_s * psCurr = NULL;

    if ( NULL == cpcsThis || NULL == cpcnCurrKey )
    {
        return NULL;
    }

    psCurr = _Access(cpcsThis->psRoot, cpcnCurrKey, NULL);

    if ( NULL == psCurr )
    {
        return NULL;
    }

    if ( NULL != psCurr->psL )
    {
        return psCurr->psL->pnKey;
    }
    
    if ( NULL != psCurr->psR )
    {
        return psCurr->psR->pnKey;
    }

    for ( ; NULL != psCurr->psP; psCurr = psCurr->psP )
    {
        if ( 
            psCurr->psP->psL == psCurr &&
            psCurr->psP->psR != NULL
        ) {
            return psCurr->psP->psR->pnKey;
        }
    }

    return NULL;
}
