/* events
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EVENT_H
#define EVENT_H

#include "events/EventQueue.h"
#include "events/TaskBase.h"
#include "platform/mbed_assert.h"

namespace events {
/** \addtogroup events */

/** Task
 *
 *  Representation of a postable task
 * @ingroup events
 */



//template <typename C>
//class _Task: public TaskBase {
//public:
//
//    _Task(TaskQueue *q, C *c): TaskBase(reinterpret_cast<uint8_t *>(&c), sizeof(C), C::thunk), _q(q) {
//
//    }
//
//    void set(TaskQueue *q)
//    {
//        _q = q;
//    }
//
//protected:
//    TaskQueue *_q;
//
//};

template<typename F, typename B0=void, typename B1=void, typename B2=void,
        typename B3=void, typename B4=void, typename B5=void, typename B6=void,
        typename B7=void, typename B8=void>
struct AllArgs {
    typedef AllArgs Self;
    F f; B0 e8; B1 e7; B2 e6; B3 e5; B4 e4; B5 e3; B6 e2; B7 e1; B8 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8())
        : f(f), e8(b0), e7(b1), e6(b2), e5(b3), e4(b4), e3(b5), e2(b6), e1(b7), e0(b8) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e8, s->e7, s->e6, s->e5, s->e4, s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5, typename B6,
        typename B7>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, void> {
    typedef AllArgs Self;
    F f; B0 e7; B1 e6; B2 e5; B3 e4; B4 e3; B5 e2; B6 e1; B7 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7())
        : f(f), e7(b0), e6(b1), e5(b2), e4(b3), e3(b4), e2(b5), e1(b6), e0(b7) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e7, s->e6, s->e5, s->e4, s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5, typename B6>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, void, void> {
    typedef AllArgs Self;
    F f; B0 e6; B1 e5; B2 e4; B3 e3; B4 e2; B5 e1; B6 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6())
        : f(f), e6(b0), e5(b1), e4(b2), e3(b3), e2(b4), e1(b5), e0(b6) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e6, s->e5, s->e4, s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e5; B1 e4; B2 e3; B3 e2; B4 e1; B5 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5())
        : f(f), e5(b0), e4(b1), e3(b2), e2(b3), e1(b4), e0(b5) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e5, s->e4, s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4>
struct AllArgs<F, B0, B1, B2, B3, B4, void, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e4; B1 e3; B2 e2; B3 e1; B4 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4())
        : f(f), e4(b0), e3(b1), e2(b2), e1(b3), e0(b4) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e4, s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3>
struct AllArgs<F, B0, B1, B2, B3, void, void, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e3; B1 e2; B2 e1; B3 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3())
        : f(f), e3(b0), e2(b1), e1(b2), e0(b3) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e3, s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2>
struct AllArgs<F, B0, B1, B2, void, void, void, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e2; B1 e1; B2 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2())
        : f(f), e2(b0), e1(b1), e0(b2) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e2, s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1>
struct AllArgs<F, B0, B1, void, void, void, void, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e1; B1 e0;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1())
        : f(f), e1(b0), e0(b1) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e1, s->e0);
        s->~Self();
    }
};

template<typename F, typename B0>
struct AllArgs<F, B0, void, void, void, void, void, void, void, void> {
    typedef AllArgs Self;
    F f; B0 e0;

    AllArgs(F f, B0 b0=B0())
        : f(f), e0(b0) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->e0);
        s->~Self();
    }
};

template<typename F>
struct AllArgs<F, void, void, void, void, void, void, void, void, void> {
    typedef AllArgs Self;
    F f;

    AllArgs(F f)
        : f(f) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f();
        s->~Self();
    }
};




//template<typename F, typename B0=void, typename B1=void, typename B2=void,
//        typename B3=void, typename B4=void, typename B5=void, typename B6=void,
//        typename B7=void, typename B8=void>
//struct AllArgs;
//
//template<typename F, typename B0, typename B1, typename B2,
//        typename B3, typename B4, typename B5, typename B6,
//        typename B7, typename B8>
//struct AllArgs<F, B0, B1, void, void, void, void, void, void, void> {
//    typedef AllArgs<F, B0, B1, void, void, void, void, void, void, void> Self;
//    F f; B0 b0; B1 b1;
//
//    static void copy(void *dest, void *src) {
//        new (dest) Self(*(Self*)src);
//    }
//
//    static void call_and_destroy(void *data) {
//        Self *self = static_cast<Self*>(data);
//        self->f(self->b0, self->b1);
//        self->~Self();
//    }
//};
//
//template<typename F, typename B0, typename B1, typename B2,
//        typename B3, typename B4, typename B5, typename B6,
//        typename B7, typename B8>
//struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, B8> {
//    typedef AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, B8> Self;
//    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8;
//
//    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8())
//        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8) {}
//
//    static void copy(void *dest, void *src) {
//        new (dest) Self(*(Self*)src);
//    }
//
//    static void call_and_destroy(void *data) {
//        Self *s = static_cast<Self*>(data);
//        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
//        s->~Self();
//    }
//};
//


template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
struct PartialArgs;

template <typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<void(A0, A1, A2, A3), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(A0, A1, A2, A3), B0, B1, B2, B3, B4, A0, A1, A2, A3> AllArgs;

    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.e3 = a0;
        all.e2 = a1;
        all.e1 = a2;
        all.e0 = a3;
    }
};

template <typename A0, typename A1, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<void(A0, A1), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0, A1> AllArgs;

    AllArgs all;

    void set(A0 a0, A1 a1) {
        all.e1 = a0;
        all.e0 = a1;
    }

};

template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
class Task;

template <typename F, typename B0, typename B1, typename B2, typename B3, typename B4>
class Task<F, B0, B1, B2, B3, B4>: public TaskBase {
    typedef PartialArgs<F, B0, B1, B2, B3, B4> PartialArgs;
    typedef PartialArgs::AllArgs AllArgs;
    PartialArgs partial;

    template <typename F>
    Task(F f): TaskBase((void*)&partial.all, sizeof(partial.all), &AllArgs::copy, &AllArgs::call_and_destroy), _c(f) {

    }

    template <typename F, typename B0>
    Task(F f, B0 b0): TaskBase((void*)&partial.all, sizeof(partial.all), &AllArgs::copy, &AllArgs::call_and_destroy), _c(f, b0) {

    }

    template <typename F, typename B0, typename B1>
    Task(F f, B0 b0, B1 b1): TaskBase((void*)&partial.all, sizeof(partial.all), &AllArgs::copy, &AllArgs::call_and_destroy), _c(f, b0, b1) {

    }

    void call() {
        //TODO - post
    }

    template <typename A0>
    void call(A0 a0) {
        _c.set(a0);
        //TODO - post
    }

    template <typename A0, typename A1>
    void call(A0 a0, A1 a1) {
        _c.set(a0, a1);
        //TODO - post
    }
};

//template <typename F>
//struct ContextF;
//
//template <typename A0, typename A1>
//struct ArgContext<void(A0, A1)> {
//    A0 a0;
//    A1 a1;
//};
//
//template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
//struct ContextF0;
//
//template <typename F, typename B0>
//struct ContextF0<F, B0, void, void, void> {
//    typedef ContextF0<F, B0, void, void, void> Self;
//    F call;
//    ArgContext<F> args;
//    B0 b0;
//
//    ContextF0(F f, B0 b0)
//        : f(f), b0(b0) {}
//
//    static void copy(void *dest, void *src) {
//        new (dest) Self(*(Self*)src);
//    }
//
//    static void call_and_destroy(void *data) {
//        Self *self = static_cast<Self*>(data);
//        self->f(self->b0);
//        self->~Self();
//    }
//};
//



//template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
//class Task;
//
//template <typename F>
//class Task: public TaskBase {
//    typedef EventQueue::contextF0<F, B0, B1, B2, B3, B4> Context;
//    Context _c;
//
//    template <typename F>
//    Task(F f): _c(f) {
//
//    }
//
//    template <typename F, typename C0>
//    Task(F f, C0 c0): _c(f, c0) {
//
//    }
//
//    template <typename F, typename C0, typename C1>
//    Task(F f, C0 c0, C1 c1): _c(f, c0, c1) {
//
//    }
//
//    void call() {
//        _c.f();
//    }
//
//    template <typename A0>
//    void call(A0 a0) {
//        _c.f(a0);
//    }
//
//    template <typename A0, typename A1>
//    void call(A0 a0, A1 a1) {
//        _c.f(a0, a1);
//        //post
//    }
//};

//template <typename F, typename A0>
//class Task: public TaskBase {
//    typedef EventQueue::contextF1<F, A0> Context;
//
//    /** Create an event
//     *  @param q                Event queue to dispatch on
//     *  @param f                Function to execute when the event is dispatched
//     *  @param b0,b1            Arguments to bind to the callback, these arguments are
//     *                          copied to the task. Must be type-compatible with b0..b1, the
//     *                          arguments to the underlying callback.
//     */
//    template<typename B0, typename B1>
//    Task(F f, B0 b0, B1 b1): _c(f, b0, b1) {
//
//    }
//
//
//    Context _c;
//    void call(A0 a0) {
//        _c(a0);
//    }
//}

template <typename F, typename A0, typename A1>
class Task;

template <typename F, typename A0, typename A1, typename A2>
class Task;

template <typename F, typename A0, typename A1, typename A2, typename A3>
class Task;

template <>
class Task<void()>;

template <typename A0>
class Task<void(A0)>;

template <typename A0, typename A1>
class Task<void(A0, A1)>;

template <typename A0, typename A1, typename A2>
class Task<void(A0, A1, A2)>;

template <typename A0>
class Task<void(A0)>: public TaskBase {
public:
    typedef EventQueue::context10<A0> Context;

    Task(TaskQueue *q): _Task< Task<void()> >(q, this) {
        Context::thunk;
    }

private:


};

///** Task
// *
// *  Representation of a postable task
// * @ingroup events
// */
//class Task: public _Task< {
//public:
//    /** Create a task
//     *
//     *  Constructs a task. The specified callback acts as the target
//     *  for the task and is executed in the context of the task queue's
//     *  dispatch loop once posted.
//     *
//     *  @param q                Event queue to dispatch on
//     *  @param f                Function to execute when the event is dispatched
//     */
//    Task(TaskQueue *q=NULL): TaskBase(reinterpret_cast<uint8_t *>(&_callback), sizeof(_callback), task_thunk), _q(q) {
//
//    }
//
//    void set(TaskQueue *q)
//    {
//        _q = q;
//    }
//
//    void call()
//    {
//        cancel();
//        _q->post(this);
//    }
//
//    template <typename A1>
//    void call(A1 a1)
//    {
//        cancel();
//        _callback = mbed::Callback<void()>(a1);
//        _q->post(this);
//    }
//
//    template <typename A1, typename A2>
//    void call(A1 a1, A2 a2)
//    {
//        cancel();
//        _callback = mbed::Callback<void()>(a1, a2);
//        _q->post(this);
//    }
//
//private:
//    mbed::Callback<void()> _callback;
//
//    static void task_thunk(void *data) {
//        (*(mbed::Callback<void()>*)data)();
//    }
//};

}

/** @}*/

#endif
