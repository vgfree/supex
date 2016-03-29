/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */

/*
 *    File:       OSHashTable.h
 *
 *    Contains:  Defines a template class for hash tables.
 *
 *
 *
 *
 */

#ifndef _OSHASHTABLE_H_
#define _OSHASHTABLE_H_

#include "MyAssert.h"
#include "OSHeaders.h"

/*
 *   T must have a fNextHashEntry field, and key(T) must returns the key of type K.
 *   K must have a method GetHashKey() that returns an UInt32 bit hash value.
 *   Will the hash table can contain duplicate keys, the Map function will return only the first one.
 */

template <class T, class K>
class OSHashTable {
public:
	OSHashTable(UInt32 size)
	{
		fHashTable = new(T *[size]);
		Assert(fHashTable);
		memset(fHashTable, 0, sizeof(T *) * size);
		fSize = size;

		// Determine whether the hash size is a power of 2
		// if not set the mask to zero, otherwise we can
		// use the mask which is faster for ComputeIndex
		// 就是说size为2的幂次方的时候，mask&size的值为0
		fMask = fSize - 1;

		if ((fMask & fSize) != 0) {	// size！=2的幂次方，置mask为0
			fMask = 0;
		}

		fNumEntries = 0;
	}
	~OSHashTable()
	{
		delete[] fHashTable;
	}

	void Add(T *entry)
	{
		Assert(entry->fNextHashEntry == NULL);
		K key(entry);

		UInt32 theIndex = ComputeIndex(key.GetHashKey());
		entry->fNextHashEntry = fHashTable[theIndex];
		fHashTable[theIndex] = entry;
		fNumEntries++;
	}

	void Remove(T *entry)
	{
		K key(entry);

		UInt32  theIndex = ComputeIndex(key.GetHashKey());
		T       *elem = fHashTable[theIndex];
		T       *last = NULL;

		while (elem && elem != entry) {
			last = elem;	// 记录上一个
			elem = elem->fNextHashEntry;
		}

		if (elem) {	// sometimes remove is called 2x ( swap, then un register )
			Assert(elem);

			if (last) {
				last->fNextHashEntry = elem->fNextHashEntry;
			} else {
				fHashTable[theIndex] = elem->fNextHashEntry;
			}

			elem->fNextHashEntry = NULL;
			fNumEntries--;
		}
	}

	T *Map(K *key)
	{
		UInt32  theIndex = ComputeIndex(key->GetHashKey());
		T       *elem = fHashTable[theIndex];

		while (elem) {
			K elemKey(elem);

			if (elemKey == *key) {	// 相等比较：两个K对象的string字段
				break;
			}

			elem = elem->fNextHashEntry;
		}

		return elem;
	}

	UInt64 GetNumEntries()
	{
		return fNumEntries;
	}

	UInt32 GetTableSize()
	{
		return fSize;
	}

	T *GetTableEntry(int i)
	{
		return fHashTable[i];
	}

private:
	T       **fHashTable;
	UInt32  fSize;
	UInt32  fMask;
	UInt64  fNumEntries;

	// 计算Map表的索引
	UInt32 ComputeIndex(UInt32 hashKey)
	{
		if (fMask) {			// size=2的幂次方
			return hashKey & fMask;	// 结果为hashKey值本身
		} else {			// size！=2的幂次方
			return hashKey % fSize;
		}
	}
};

template <class T, class K>
class OSHashTableIter {
public:
	OSHashTableIter(OSHashTable <T, K> *table)
	{
		fHashTable = table;
		First();
	}

	// 找到fHashTable容器中第一个有效的
	void First()
	{
		for (fIndex = 0; fIndex < fHashTable->GetTableSize(); fIndex++) {
			fCurrent = fHashTable->GetTableEntry(fIndex);

			if (fCurrent) {
				break;
			}
		}
	}

	void Next()
	{
		fCurrent = fCurrent->fNextHashEntry;

		if (!fCurrent) {
			for (fIndex = fIndex + 1; fIndex < fHashTable->GetTableSize(); fIndex++) {
				fCurrent = fHashTable->GetTableEntry(fIndex);

				if (fCurrent) {
					break;
				}
			}
		}
	}

	Bool16 IsDone()
	{
		return fCurrent == NULL;
	}

	T *GetCurrent()
	{
		return fCurrent;
	}

private:
	OSHashTable <T, K>      *fHashTable;
	T                       *fCurrent;
	UInt32                  fIndex;
};
#endif	// _OSHASHTABLE_H_

