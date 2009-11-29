/*
 * Process Hacker - 
 *   internal object manager
 * 
 * Copyright (C) 2009 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REFP_H
#define REFP_H

#define REF_PRIVATE
#include <ref.h>

#define PhObjectToObjectHeader(Object) ((PPH_OBJECT_HEADER)CONTAINING_RECORD((PCHAR)(Object), PH_OBJECT_HEADER, Body))
#define PhObjectHeaderToObject(ObjectHeader) (&((PPH_OBJECT_HEADER)(ObjectHeader))->Body)
#define PhpAddObjectHeaderSize(Size) ((Size) + FIELD_OFFSET(PH_OBJECT_HEADER, Body))

typedef struct _PH_OBJECT_HEADER *PPH_OBJECT_HEADER;
typedef struct _PH_OBJECT_TYPE *PPH_OBJECT_TYPE;

typedef struct _PH_OBJECT_HEADER
{
    /* The reference count of the object. */
    LONG RefCount;
    /* The flags that were used to create the object. */
    ULONG Flags;
    union
    {
        /* The size of the object, excluding the header. */
        SIZE_T Size;
        /* A pointer to the object header of the next object to free. */
        PPH_OBJECT_HEADER NextToFree;
    };
    /* The type of the object. */
    PPH_OBJECT_TYPE Type;
    
    /* The body of the object. For use by the KphObject(Header)ToObject(Header) macros. */
    QUAD Body;
} PH_OBJECT_HEADER, *PPH_OBJECT_HEADER;

typedef struct _PH_OBJECT_TYPE
{
    /* The flags that were used to create the object type. */
    ULONG Flags;
    /* An optional procedure called when objects of this type are freed. */
    PPH_TYPE_DELETE_PROCEDURE DeleteProcedure;
    
    /* The total number of objects of this type that are alive. */
    ULONG NumberOfObjects;
} PH_OBJECT_TYPE, *PPH_OBJECT_TYPE;

/* PhpInterlockedIncrementSafe
 * 
 * Increments a reference count, but will never increment 
 * from 0 to 1.
 */
FORCEINLINE BOOLEAN PhpInterlockedIncrementSafe(
    __inout PLONG RefCount
    )
{
    LONG refCount;
    
    /* Here we will attempt to increment the reference count, 
     * making sure that it is not 0.
     */
    
    while (TRUE)
    {
        refCount = *RefCount;
        
        /* Check if the reference count is 0. If it is, the 
         * object is being or about to be deleted.
         */
        if (refCount == 0)
            return FALSE;
        
        /* Try to increment the reference count. */
        if (InterlockedCompareExchange(
            RefCount,
            refCount + 1,
            refCount
            ) == refCount)
        {
            /* Success. */
            return TRUE;
        }
        
        /* Someone else changed the reference count before we did. 
         * Go back and try again.
         */
    }
}

PPH_OBJECT_HEADER PhpAllocateObject(
    __in SIZE_T ObjectSize
    );

VOID PhpDeferDeleteObject(
    __in PPH_OBJECT_HEADER ObjectHeader
    );

NTSTATUS PhpDeferDeleteObjectRoutine(
    __in PVOID Parameter
    );

VOID PhpFreeObject(
    __in PPH_OBJECT_HEADER ObjectHeader
    );

#endif