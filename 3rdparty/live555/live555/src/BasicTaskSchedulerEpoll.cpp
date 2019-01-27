/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/


#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include <stdio.h>
#if defined(__WIN32__) || defined(_WIN32)
#include "wepoll.h"
#define EPOLL_INVALID NULL
#else
#include <sys/epoll.h>
#include <sys/prctl.h>
#define EPOLL_INVALID -1
#endif
#include "BasicTaskSchedulerEpoll.h"


////////// BasicTaskSchedulerEpoll //////////

BasicTaskSchedulerEpoll* BasicTaskSchedulerEpoll::createNew() {
	return new BasicTaskSchedulerEpoll();
}

BasicTaskSchedulerEpoll::BasicTaskSchedulerEpoll()
  : fEpollEventMax(BASIC_TS_EPOLL_EVENT_MAX) {
#if defined(__WIN32__) || defined(_WIN32)
  fEpollFd = epoll_create(fEpollEventMax + 1);
#else
  fEpollFd = epoll_create(fEpollEventMax + 1);
  if (fEpollFd == EPOLL_INVALID){
	  printf("live555 epoll_create error\n");
  }
#endif
}

BasicTaskSchedulerEpoll::~BasicTaskSchedulerEpoll() {
#ifndef _WIN32
	if (fEpollFd >= 0)
	{
		close(fEpollFd);
		fEpollFd = -1;
	}
#else
  if (fEpollFd !=EPOLL_INVALID)
	{
		epoll_close(fEpollFd);
	}
#endif
}

#ifndef MILLION
#define MILLION 1000000
#endif

void BasicTaskSchedulerEpoll::SingleStep(unsigned maxDelayTime) {
  struct epoll_event events[BASIC_TS_EPOLL_EVENT_MAX];
  int epollTimeout = 10;
  int eventIdx = -1;
  
  DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();
  struct timeval tv_timeToDelay;
  tv_timeToDelay.tv_sec = timeToDelay.seconds();
  tv_timeToDelay.tv_usec = timeToDelay.useconds();
  // Very large "tv_sec" values cause select() to fail.
  // Don't make it any larger than 1 million seconds (11.5 days)
  const long MAX_TV_SEC = MILLION;
  if (tv_timeToDelay.tv_sec > MAX_TV_SEC) {
	  tv_timeToDelay.tv_sec = MAX_TV_SEC;
  }
  // Also check our "maxDelayTime" parameter (if it's > 0):
  if (maxDelayTime > 0 &&
	  (tv_timeToDelay.tv_sec > (long)maxDelayTime / MILLION ||
	  (tv_timeToDelay.tv_sec == (long)maxDelayTime / MILLION &&
		  tv_timeToDelay.tv_usec > (long)maxDelayTime%MILLION))) {
	  tv_timeToDelay.tv_sec = maxDelayTime / MILLION;
	  tv_timeToDelay.tv_usec = maxDelayTime % MILLION;
  }

  epollTimeout = tv_timeToDelay.tv_sec * 1000 + tv_timeToDelay.tv_usec / 1000;
  if (epollTimeout > 1000)
  {
	  epollTimeout = 1000;
  }

  int ret = epoll_wait(fEpollFd, events, BASIC_TS_EPOLL_EVENT_MAX, epollTimeout);
  if (ret < 0) {
#ifdef _WIN32
	  printf("live555 epoll_wait error \n");
#else
	  printf("live555 epoll_wait error %s\n", strerror(errno));
#endif
	  return;
  }

#if 0
  // Call the handler function for one readable socket:
  HandlerIterator iter(*fHandlers);
  HandlerDescriptor* handler;
  iter.reset();

  while ((handler = iter.next()) != NULL) {
    int sock = handler->socketNum; // alias
    int resultConditionSet = 0;
	eventIdx = getIndexEventByFd(events, sock, ret);
	if (eventIdx != -1){
		/*
		if (events[eventIdx].events & EPOLLET){
			printf("%s-----%d sock %d\n", __FILE__, __LINE__, sock);
		}*/
#ifndef _WIN32
		if (events[eventIdx].events & EPOLLIN || events[eventIdx].events & EPOLLET){
			resultConditionSet |= SOCKET_READABLE;
		}
#else
		if (events[eventIdx].events & EPOLLIN){
			resultConditionSet |= SOCKET_READABLE;
		}
#endif
		if (events[eventIdx].events & EPOLLOUT){
			resultConditionSet |= SOCKET_WRITABLE;
		}
		if (events[eventIdx].events & EPOLLERR){
			resultConditionSet |= SOCKET_EXCEPTION;
		}	
	}
    if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) {
      fLastHandledSocketNum = sock;
          // Note: we set "fLastHandledSocketNum" before calling the handler,
          // in case the handler calls "doEventLoop()" reentrantly.
      (*handler->handlerProc)(handler->clientData, resultConditionSet);
      //break;
    }
  }
#else
  if (ret > 0) 
  {
    for (int i = 0; i < ret; i ++)
    {
      const HandlerDescriptor* handler =
        static_cast<const HandlerDescriptor*>(events[i].data.ptr);
      if (handler != NULL) 
      {
        int resultConditionSet = 0;
        if (events[i].events & EPOLLIN)  resultConditionSet |= SOCKET_READABLE;
        if (events[i].events & EPOLLOUT) resultConditionSet |= SOCKET_WRITABLE;
        if (events[i].events & EPOLLERR) resultConditionSet |= SOCKET_EXCEPTION;

        if ((resultConditionSet & handler->conditionSet) != 0) {
          (*handler->handlerProc)(handler->clientData, resultConditionSet);
        }
      }
    }
  }
#endif

  // Also handle any newly-triggered event (Note that we do this *after* calling a socket handler,
  // in case the triggered event handler modifies The set of readable sockets.)
  if (fTriggersAwaitingHandling != 0) {
	  if (fTriggersAwaitingHandling == fLastUsedTriggerMask) {
		  // Common-case optimization for a single event trigger:
		  fTriggersAwaitingHandling &= ~fLastUsedTriggerMask;
		  if (fTriggeredEventHandlers[fLastUsedTriggerNum] != NULL) {
			  (*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
		  }
	  }
	  else {
		  // Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
		  unsigned i = fLastUsedTriggerNum;
		  EventTriggerId mask = fLastUsedTriggerMask;

		  do {
			  i = (i + 1) % MAX_NUM_EVENT_TRIGGERS;
			  mask >>= 1;
			  if (mask == 0) mask = 0x80000000;

			  if ((fTriggersAwaitingHandling&mask) != 0) {
				  fTriggersAwaitingHandling &= ~mask;
				  if (fTriggeredEventHandlers[i] != NULL) {
					  (*fTriggeredEventHandlers[i])(fTriggeredEventClientDatas[i]);
				  }

				  fLastUsedTriggerMask = mask;
				  fLastUsedTriggerNum = i;
				  break;
			  }
		  } while (i != fLastUsedTriggerNum);
	  }
  }

  // Also handle any delayed event that may have come due.
  fDelayQueue.handleAlarm();
}

void BasicTaskSchedulerEpoll
  ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData) {
  struct epoll_event ev;
  
  if (socketNum < 0 || fEpollFd == EPOLL_INVALID) return;
  
  memset(&ev, 0, sizeof(struct epoll_event));
  
  const HandlerDescriptor* handler = lookupHandlerDescriptor(socketNum);
  if (handler != NULL) {
	  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev);
  }
  
  if (conditionSet == 0) {
    fHandlers->clearHandler(socketNum);
  } else {
#if 0
    fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);

    if (conditionSet&SOCKET_READABLE){
	    ev.data.fd = socketNum;
		  //ev.events = EPOLLIN | EPOLLET;
		  ev.events = EPOLLIN;
		 if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			printf("live555 epoll_ctl  EPOLL_CTL_ADD error\n");
		 }
	}
    if (conditionSet&SOCKET_WRITABLE){
	    ev.data.fd = socketNum;
		  ev.events = EPOLLOUT;
		  if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			printf("live555 epoll_ctl  EPOLL_CTL_ADD error\n");
		 }
	}
    if (conditionSet&SOCKET_EXCEPTION){
	    ev.data.fd = socketNum;
		ev.events = EPOLLERR;
		 if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			printf("live555 epoll_ctl  EPOLL_CTL_ADD error\n");
		 }
	}
#else
    fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);

    handler = lookupHandlerDescriptor(socketNum);
    ev.data.ptr = const_cast<HandlerDescriptor*>(handler);

    if (conditionSet&SOCKET_READABLE) ev.events |= EPOLLIN;
    if (conditionSet&SOCKET_WRITABLE) ev.events |= EPOLLOUT;

    epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev);
#endif
  }
}

void BasicTaskSchedulerEpoll::moveSocketHandling(int oldSocketNum, int newSocketNum) {
#if 0
  struct epoll_event ev;
  
  if (oldSocketNum < 0 || newSocketNum < 0 || fEpollFd == EPOLL_INVALID) return; // sanity check
  
  memset(&ev, 0, sizeof(struct epoll_event));
  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, oldSocketNum, &ev);
  ev.data.fd = newSocketNum;
#ifndef _WIN32
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
#else
  ev.events = EPOLLIN | EPOLLOUT;
#endif
  epoll_ctl(fEpollFd, EPOLL_CTL_ADD, newSocketNum, &ev);
  fHandlers->moveHandler(oldSocketNum, newSocketNum);
#else
  if (oldSocketNum < 0 || newSocketNum < 0) return; // sanity check

  const HandlerDescriptor* handler = lookupHandlerDescriptor(oldSocketNum);
  if (handler == NULL) {
    return;
  }

  epoll_event ev;
  memset(&ev, 0, sizeof(ev));

  if (handler->conditionSet&SOCKET_READABLE) ev.events |= EPOLLIN;
  if (handler->conditionSet&SOCKET_WRITABLE) ev.events |= EPOLLOUT;

  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, oldSocketNum, &ev);
  epoll_ctl(fEpollFd, EPOLL_CTL_ADD, newSocketNum, &ev);

  fHandlers->moveHandler(oldSocketNum, newSocketNum);
#endif
}

const HandlerDescriptor* BasicTaskSchedulerEpoll::lookupHandlerDescriptor(int socketNum) const {
  HandlerDescriptor* handler;
  HandlerIterator iter(*fHandlers);
  while ((handler = iter.next()) != NULL) {
    if (handler->socketNum == socketNum) break;
  }
  return handler;
}
