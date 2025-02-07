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
 *    File:       OSQueue.cpp
 *
 *    Contains:   implements OSQueue class
 *
 *
 */

#include "OSQueue.h"

OSQueue::OSQueue() : fLength(0)
{
	fSentinel.fNext = &fSentinel;
	fSentinel.fPrev = &fSentinel;
}

// 前插法
void OSQueue::EnQueue(OSQueueElem *elem)
{
	Assert(elem != NULL);

	if (elem->fQueue == this) {	// elem已经在queue中
		return;
	}

	Assert(elem->fQueue == NULL);
	elem->fNext = fSentinel.fNext;
	elem->fPrev = &fSentinel;
	elem->fQueue = this;

	fSentinel.fNext->fPrev = elem;
	fSentinel.fNext = elem;
	fLength++;
}

OSQueueElem *OSQueue::DeQueue()
{
	if (fLength > 0) {
		OSQueueElem *elem = fSentinel.fPrev;
		Assert(fSentinel.fPrev != &fSentinel);
		elem->fPrev->fNext = &fSentinel;
		fSentinel.fPrev = elem->fPrev;
		elem->fQueue = NULL;
		fLength--;
		return elem;
	} else {
		return NULL;
	}
}

void OSQueue::Remove(OSQueueElem *elem)
{
	Assert(elem != NULL);
	Assert(elem != &fSentinel);

	if (elem->fQueue == this) {
		elem->fNext->fPrev = elem->fPrev;
		elem->fPrev->fNext = elem->fNext;
		elem->fQueue = NULL;
		fLength--;
	}
}

#if OSQUEUETESTING
Bool16 OSQueue::Test()
{
	OSQueue         theVictim;
	void            *x = (void *)1;
	OSQueueElem     theElem1(x);

	x = (void *)2;
	OSQueueElem theElem2(x);
	x = (void *)3;
	OSQueueElem theElem3(x);

	if (theVictim.GetHead() != NULL) {
		return false;
	}

	if (theVictim.GetTail() != NULL) {
		return false;
	}

	theVictim.EnQueue(&theElem1);

	if (theVictim.GetHead() != &theElem1) {
		return false;
	}

	if (theVictim.GetTail() != &theElem1) {
		return false;
	}

	OSQueueElem *theElem = theVictim.DeQueue();

	if (theElem != &theElem1) {
		return false;
	}

	if (theVictim.GetHead() != NULL) {
		return false;
	}

	if (theVictim.GetTail() != NULL) {
		return false;
	}

	theVictim.EnQueue(&theElem1);
	theVictim.EnQueue(&theElem2);

	if (theVictim.GetHead() != &theElem1) {
		return false;
	}

	if (theVictim.GetTail() != &theElem2) {
		return false;
	}

	theElem = theVictim.DeQueue();

	if (theElem != &theElem1) {
		return false;
	}

	if (theVictim.GetHead() != &theElem2) {
		return false;
	}

	if (theVictim.GetTail() != &theElem2) {
		return false;
	}

	theElem = theVictim.DeQueue();

	if (theElem != &theElem2) {
		return false;
	}

	theVictim.EnQueue(&theElem1);
	theVictim.EnQueue(&theElem2);
	theVictim.EnQueue(&theElem3);

	if (theVictim.GetHead() != &theElem1) {
		return false;
	}

	if (theVictim.GetTail() != &theElem3) {
		return false;
	}

	theElem = theVictim.DeQueue();

	if (theElem != &theElem1) {
		return false;
	}

	if (theVictim.GetHead() != &theElem2) {
		return false;
	}

	if (theVictim.GetTail() != &theElem3) {
		return false;
	}

	theElem = theVictim.DeQueue();

	if (theElem != &theElem2) {
		return false;
	}

	if (theVictim.GetHead() != &theElem3) {
		return false;
	}

	if (theVictim.GetTail() != &theElem3) {
		return false;
	}

	theElem = theVictim.DeQueue();

	if (theElem != &theElem3) {
		return false;
	}

	theVictim.EnQueue(&theElem1);
	theVictim.EnQueue(&theElem2);
	theVictim.EnQueue(&theElem3);

	OSQueueIter theIterVictim(&theVictim);

	if (theIterVictim.IsDone()) {
		return false;
	}

	if (theIterVictim.GetCurrent() != &theElem3) {
		return false;
	}

	theIterVictim.Next();

	if (theIterVictim.IsDone()) {
		return false;
	}

	if (theIterVictim.GetCurrent() != &theElem2) {
		return false;
	}

	theIterVictim.Next();

	if (theIterVictim.IsDone()) {
		return false;
	}

	if (theIterVictim.GetCurrent() != &theElem1) {
		return false;
	}

	theIterVictim.Next();

	if (!theIterVictim.IsDone()) {
		return false;
	}

	if (theIterVictim.GetCurrent() != NULL) {
		return false;
	}

	theVictim.Remove(&theElem1);

	if (theVictim.GetHead() != &theElem2) {
		return false;
	}

	if (theVictim.GetTail() != &theElem3) {
		return false;
	}

	theVictim.Remove(&theElem1);

	if (theVictim.GetHead() != &theElem2) {
		return false;
	}

	if (theVictim.GetTail() != &theElem3) {
		return false;
	}

	theVictim.Remove(&theElem3);

	if (theVictim.GetHead() != &theElem2) {
		return false;
	}

	if (theVictim.GetTail() != &theElem2) {
		return false;
	}

	return true;
}
#endif	// if OSQUEUETESTING

void OSQueueIter::Next()
{
	if (fCurrentElemP == fQueueP->GetTail()) {
		fCurrentElemP = NULL;
	} else {
		fCurrentElemP = fCurrentElemP->Prev();
	}
}

/*******************OSQueue_Blocking 类实现*******************************/

OSQueueElem *OSQueue_Blocking::DeQueueBlocking(OSThread *inCurThread, SInt32 inTimeoutInMilSecs)
{
	OSMutexLocker theLocker(&fMutex);	// 获取BlockingQueue的互斥锁

#ifdef __Win32_
	if (fQueue.GetLength() == 0) {
		fCond.Wait(&fMutex, inTimeoutInMilSecs);
		return NULL;
	}

#else
	if (fQueue.GetLength() == 0) {
		fCond.Wait(&fMutex, inTimeoutInMilSecs);// ？？？？BlockingQueue要阻塞下（针对此情况）
	}
#endif

	OSQueueElem *retval = fQueue.DeQueue();
	return retval;
}

OSQueueElem *OSQueue_Blocking::DeQueue()
{
	OSMutexLocker   theLocker(&fMutex);
	OSQueueElem     *retval = fQueue.DeQueue();

	return retval;
}

void OSQueue_Blocking::EnQueue(OSQueueElem *obj)
{
	{
		OSMutexLocker theLocker(&fMutex);
		fQueue.EnQueue(obj);
	}
	// BlockQueue相当于生产者，EnQueue操作等价于产生了一个东西
	// 然后发个信号给正在阻塞的线程（如果有多个阻塞的，会发给优先级较高的），让其脱离阻塞状态
	fCond.Signal();
}

