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

#ifndef __BASIC_TASK_EPOLL_HH__
#define __BASIC_TASK_EPOLL_HH__

#ifndef _BASIC_USAGE_ENVIRONMENT0_HH
#include "BasicUsageEnvironment0.hh"
#endif
#include "HandlerSet.hh"

#define BASIC_TS_EPOLL_EVENT_MAX 1024

class BasicTaskSchedulerEpoll: public BasicTaskScheduler0 {
public:
  static BasicTaskSchedulerEpoll* createNew();
  virtual ~BasicTaskSchedulerEpoll();

protected:
  BasicTaskSchedulerEpoll();
      // called only by "createNew()"

protected:
  // Redefined virtual functions:
  virtual void SingleStep(unsigned maxDelayTime);

  virtual void setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData);
  virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

  const HandlerDescriptor* lookupHandlerDescriptor(int socketNum) const;

protected:
  // To implement background operations:
  int fEpollEventMax;
#if defined(__WIN32__) || defined(_WIN32)
  void* fEpollFd;
#else
  int fEpollFd;
#endif
};


#endif