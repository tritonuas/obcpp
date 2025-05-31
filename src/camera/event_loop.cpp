/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * event_loop.cpp - Event loop based on cam
 */

#include "event_loop.h"

#include <assert.h>
#include <event2/event.h>
#include <event2/thread.h>

EventLoop *EventLoop::instance_ = nullptr;

EventLoop::EventLoop()
{
	assert(!instance_);

	evthread_use_pthreads();
	event_ = event_base_new();
	instance_ = this;
}

EventLoop::~EventLoop()
{
	instance_ = nullptr;

	event_base_free(event_);
	libevent_global_shutdown();
}

int EventLoop::exec()
{
	exitCode_ = -1;
	exit_.store(false, std::memory_order_release);

	while (!exit_.load(std::memory_order_acquire)) {
		dispatchCalls();
		event_base_loop(event_, EVLOOP_NO_EXIT_ON_EMPTY);
	}

	return exitCode_;
}

void EventLoop::exit(int code) {
	exitCode_ = code;
	exit_.store(true, std::memory_order_release);
	interrupt();
}

void EventLoop::interrupt()
{
	event_base_loopbreak(event_);
}

void EventLoop::timeoutTriggered(int fd, short event, void *arg) {
	EventLoop *self = static_cast<EventLoop *>(arg);
	self->exit();
}

void EventLoop::timeout(unsigned int sec) {
	struct event *ev;
	struct timeval tv;

	tv.tv_sec = sec;
	tv.tv_usec = 0;
	ev = evtimer_new(event_, &timeoutTriggered, this);
	evtimer_add(ev, &tv);
}

void EventLoop::callLater(const std::function<void()> &func)
{
	{
		std::unique_lock<std::mutex> locker(lock_);
		calls_.push_back(func);
	}

	interrupt();
}

void EventLoop::dispatchCalls()
{
	std::unique_lock<std::mutex> locker(lock_);

	for (auto iter = calls_.begin(); iter != calls_.end(); ) {
		std::function<void()> call = std::move(*iter);

		iter = calls_.erase(iter);

		locker.unlock();
		call();
		locker.lock();
	}
}

