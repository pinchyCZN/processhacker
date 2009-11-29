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

#include <refp.h>

/* The object type type. */
PPH_OBJECT_TYPE PhObjectTypeObject = NULL;

/* Whether the object manager is destroying all objects. */
BOOLEAN PhObjectDeinitializing = FALSE;
/* The next object to delete. */
PPH_OBJECT_HEADER PhObjectNextToFree = NULL;

/* PhInitializeRef
 * 
 * Initializes the PH object manager.
 */
NTSTATUS PhInitializeRef()
{
    NTSTATUS status = STATUS_SUCCESS;
    
    /* Create the fundamental object type. */
    status = PhCreateObjectType(
        &PhObjectTypeObject,
        0,
        NULL
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    /* Now that the fundamental object type exists, fix it up. */
    PhObjectToObjectHeader(PhObjectTypeObject)->Type = PhObjectTypeObject;
    PhObjectTypeObject->NumberOfObjects = 1;
    
    return status;
}

/* PhCreateObject
 * 
 * Allocates a object.
 * 
 * Object: A variable which receives a pointer to the newly allocated object.
 * ObjectSize: The size of the object.
 * Flags: A combination of flags specifying how the object is to be allocated.
 *   * PHOBJ_RAISE_ON_FAIL: An exception will be raised if the object could 
 *     not be allocated.
 * ObjectType: The type of the object.
 * AdditionalReferences: The number of references to add to the object. The 
 * object will have a reference count of 1 + AdditionalReferences.
 */
NTSTATUS PhCreateObject(
    __out PVOID *Object,
    __in SIZE_T ObjectSize,
    __in ULONG Flags,
    __in_opt PPH_OBJECT_TYPE ObjectType,
    __in_opt LONG AdditionalReferences
    )
{
    PPH_OBJECT_HEADER objectHeader;
    
    /* Check the flags. */
    if ((Flags & PHOBJ_VALID_FLAGS) != Flags) /* Valid flag mask */
        return STATUS_INVALID_PARAMETER_3;
    /* The object type is only optional if the fundamental object type 
     * hasn't been created. */
    if (!ObjectType && PhObjectTypeObject)
        return STATUS_INVALID_PARAMETER_4;
    /* Make sure the additional reference count isn't negative. */
    if (AdditionalReferences < 0)
        return STATUS_INVALID_PARAMETER_5;
    
    /* Allocate storage for the object. Note that this includes 
     * the object header followed by the object body. */
    objectHeader = PhpAllocateObject(ObjectSize);
    
    if (!objectHeader)
    {
        if (Flags & PHOBJ_RAISE_ON_FAIL)
            PhRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        else
            return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    /* Object type statistics. */
    if (ObjectType)
    {
        InterlockedIncrement(&ObjectType->NumberOfObjects);
    }
    
    /* Initialize the object header. */
    objectHeader->RefCount = 1 + AdditionalReferences;
    objectHeader->Flags = Flags;
    objectHeader->Size = ObjectSize;
    objectHeader->Type = ObjectType;
    
    /* Pass a pointer to the object body back to the caller. */
    *Object = PhObjectHeaderToObject(objectHeader);
    
    return STATUS_SUCCESS;
}

/* PhCreateObjectType
 * 
 * Creates an object type.
 */
NTSTATUS PhCreateObjectType(
    __out PPH_OBJECT_TYPE *ObjectType,
    __in ULONG Flags,
    __in PPH_TYPE_DELETE_PROCEDURE DeleteProcedure
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PPH_OBJECT_TYPE objectType;
    
    /* Check the flags. */
    if ((Flags & PHOBJTYPE_VALID_FLAGS) != Flags) /* Valid flag mask */
        return STATUS_INVALID_PARAMETER_3;
    
    /* Create the type object. */
    status = PhCreateObject(
        &objectType,
        sizeof(PH_OBJECT_TYPE),
        0,
        PhObjectTypeObject,
        0
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    /* Initialize the type object. */
    objectType->Flags = Flags;
    objectType->DeleteProcedure = DeleteProcedure;
    objectType->NumberOfObjects = 0;
    
    *ObjectType = objectType;
    
    return status;
}

/* PhDereferenceObject
 * 
 * Dereferences the specified object. The object will be freed if 
 * its reference count reaches 0.
 * 
 * Object: A pointer to the object to dereference.
 * 
 * Return value: TRUE if the object was freed, otherwise FALSE.
 */
BOOLEAN PhDereferenceObject(
    __in PVOID Object
    )
{
    return PhDereferenceObjectEx(Object, 1, FALSE) == 0;
}

/* PhDereferenceObjectDeferDelete
 * 
 * Dereferences the specified object. The object will be freed in 
 * a worker thread if its reference count reaches 0.
 * 
 * Object: A pointer to the object to dereference.
 * 
 * Return value: TRUE if the object was freed, otherwise FALSE.
 */
BOOLEAN PhDereferenceObjectDeferDelete(
    __in PVOID Object
    )
{
    return PhDereferenceObjectEx(Object, 1, TRUE) == 0;
}

/* PhDereferenceObjectEx
 * 
 * Dereferences the specified object. The object will be freed if 
 * its reference count reaches 0.
 * 
 * Object: A pointer to the object to dereference.
 * RefCount: The number of references to remove.
 * 
 * Return value: The new reference count of the object.
 */
LONG PhDereferenceObjectEx(
    __in PVOID Object,
    __in LONG RefCount,
    __in BOOLEAN DeferDelete
    )
{
    PPH_OBJECT_HEADER objectHeader;
    LONG oldRefCount;
    
    /* Make sure we're not subtracting a negative reference count. */
    if (RefCount < 0)
        PhRaiseStatus(STATUS_INVALID_PARAMETER_2);
    
    objectHeader = PhObjectToObjectHeader(Object);
    
    /* Decrease the reference count. */
    oldRefCount = InterlockedExchangeAdd(&objectHeader->RefCount, -RefCount);
    
    /* Free the object if it has 0 references. */
    if (oldRefCount - RefCount == 0)
    {
        if (DeferDelete)
        {
            PhpDeferDeleteObject(objectHeader);
        }
        else
        {
            /* Free the object. */
            PhpFreeObject(objectHeader);
        }
    }
    
    return oldRefCount - RefCount;
}

/* PhGetObjectType
 * 
 * Gets an object's type.
 */
PPH_OBJECT_TYPE PhGetObjectType(
    __in PVOID Object
    )
{
    return PhObjectToObjectHeader(Object)->Type;
}

/* PhReferenceObject
 * 
 * References the specified object.
 * 
 * Object: A pointer to the object to reference.
 */
VOID PhReferenceObject(
    __in PVOID Object
    )
{
    PPH_OBJECT_HEADER objectHeader;
    
    objectHeader = PhObjectToObjectHeader(Object);
    /* Increment the reference count. */
    InterlockedIncrement(&objectHeader->RefCount);
}

/* PhReferenceObjectEx
 * 
 * References the specified object.
 * 
 * Object: A pointer to the object to reference.
 * RefCount: The number of references to add.
 * 
 * Return value: The new reference count of the object.
 */
LONG PhReferenceObjectEx(
    __in PVOID Object,
    __in LONG RefCount
    )
{
    PPH_OBJECT_HEADER objectHeader;
    LONG oldRefCount;
    
    /* Make sure we're not adding a negative reference count. */
    if (RefCount < 0)
        PhRaiseStatus(STATUS_INVALID_PARAMETER_2);
    
    objectHeader = PhObjectToObjectHeader(Object);
    /* Increase the reference count. */
    oldRefCount = InterlockedExchangeAdd(&objectHeader->RefCount, RefCount);
    
    return oldRefCount + RefCount;
}

/* PhReferenceObjectSafe
 * 
 * Attempts to reference an object and fails if it is being 
 * destroyed.
 * 
 * Object: The object to reference if it is not being deleted.
 * 
 * Return value: TRUE if the object was referenced, FALSE if 
 * it was being deleted and was not referenced.
 * 
 * Remarks:
 * This function is useful if a reference to an object is 
 * held, protected by a mutex, and the delete procedure of 
 * the object's type attempts to acquire the mutex. If this 
 * function is called while the mutex is owned, you can 
 * avoid referencing an object that is being destroyed.
 */
BOOLEAN PhReferenceObjectSafe(
    __in PVOID Object
    )
{
    PPH_OBJECT_HEADER objectHeader;
    BOOLEAN result;
    
    objectHeader = PhObjectToObjectHeader(Object);
    /* Increase the reference count only if it isn't 0 (atomically). */
    result = PhpInterlockedIncrementSafe(&objectHeader->RefCount);
    
    return result;
}

/* PhpAllocateObject
 * 
 * Allocates storage for an object.
 * 
 * ObjectSize: The size of the object, excluding the header.
 */
PPH_OBJECT_HEADER PhpAllocateObject(
    __in SIZE_T ObjectSize
    )
{
    return PhAllocate(PhpAddObjectHeaderSize(ObjectSize));
}

/* PhpDeferDeleteObject
 * 
 * Queues an object for deletion.
 */
VOID PhpDeferDeleteObject(
    __in PPH_OBJECT_HEADER ObjectHeader
    )
{
    PPH_OBJECT_HEADER nextToFree;
    
    /* Add the object to the list while saving the old value, atomically.
     * Note that it is first-in, last-out.
     */
    while (TRUE)
    {
        nextToFree = PhObjectNextToFree;
        ObjectHeader->NextToFree = nextToFree;
        
        /* Attempt to set the global next-to-free variable. */
        if (InterlockedCompareExchangePointer(
            &PhObjectNextToFree,
            ObjectHeader,
            nextToFree
            ) == nextToFree)
        {
            /* Success. */
            break;
        }
        
        /* Someone else changed the next-to-free variable. 
         * Go back and try again.
         */
    }
    
    /* Was the to-free list empty before? If so, we need to queue 
     * a work item.
     */
    if (!nextToFree)
    {
        QueueUserWorkItem(PhpDeferDeleteObjectRoutine, NULL, 0);
    }
}

/* PhpDeferDeleteObjectRoutine
 * 
 * Removes and frees objects from the to-free list.
 */
NTSTATUS PhpDeferDeleteObjectRoutine(
    __in PVOID Parameter
    )
{
    PPH_OBJECT_HEADER objectHeader = NULL;
    
    while (TRUE)
    {
        /* Get the next object to free while replacing the global variable with 
         * what we needed to free next.
         */
        objectHeader = InterlockedExchangePointer(&PhObjectNextToFree, objectHeader);
        
        /* If we have an object to free, free it and move on to the 
         * next object. Otherwise, stop.
         */
        if (objectHeader)
        {
            PhpFreeObject(objectHeader);
            objectHeader = objectHeader->NextToFree;
        }
        else
        {
            break;
        }
    }

    return STATUS_SUCCESS;
}

/* PhpFreeObject
 * 
 * Calls the delete procedure for an object and frees its 
 * allocated storage.
 * 
 * ObjectHeader: A pointer to the object header of an allocated object.
 */
VOID PhpFreeObject(
    __in PPH_OBJECT_HEADER ObjectHeader
    )
{
    /* Object type statistics. */
    InterlockedDecrement(&ObjectHeader->Type->NumberOfObjects);
    
    /* Call the delete procedure if we have one. */
    if (ObjectHeader->Type->DeleteProcedure)
    {
        ObjectHeader->Type->DeleteProcedure(
            PhObjectHeaderToObject(ObjectHeader),
            ObjectHeader->Flags
            );
    }
    
    PhFree(ObjectHeader);
}