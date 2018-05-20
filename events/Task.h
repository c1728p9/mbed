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


template<typename F, typename B0=void, typename B1=void, typename B2=void,
        typename B3=void, typename B4=void, typename B5=void, typename B6=void,
        typename B7=void, typename B8=void, typename B9=void>
struct AllArgs {
    typedef AllArgs Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8; B9 b9;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8(), B9 b9=B9())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8), b9(b9) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
        s->~Self();
    }
};


template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5, typename B6,
        typename B7, typename B8>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, B8, void> {
    typedef AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, B8, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5, typename B6,
        typename B7>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, void, void> {
    typedef AllArgs<F, B0, B1, B2, B3, B4, B5, B6, B7, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5, typename B6>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, B6, void, void, void> {
    typedef AllArgs<F, B0, B1, B2, B3, B4, B5, B6, void, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5, s->b6);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4, typename B5>
struct AllArgs<F, B0, B1, B2, B3, B4, B5, void, void, void, void> {
    typedef AllArgs<F, B0, B1, B2, B3, B4, B5, void, void, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4, s->b5);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3, typename B4>
struct AllArgs<F, B0, B1, B2, B3, B4, void, void, void, void, void> {
    typedef AllArgs<F, B0, B1, B2, B3, B4, void, void, void, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3; B4 b4;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3), b4(b4) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3, s->b4);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2,
        typename B3>
struct AllArgs<F, B0, B1, B2, B3, void, void, void, void, void, void> {
    typedef AllArgs<F, B0, B1, B2, B3, void, void, void, void, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2; B3 b3;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2(), B3 b3=B3())
        : f(f), b0(b0), b1(b1), b2(b2), b3(b3) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2, s->b3);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1, typename B2>
struct AllArgs<F, B0, B1, B2, void, void, void, void, void, void, void> {
    typedef AllArgs<F, B0, B1, B2, void, void, void, void, void, void, void> Self;
    F f; B0 b0; B1 b1; B2 b2;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1(), B2 b2=B2())
        : f(f), b0(b0), b1(b1), b2(b2) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1, s->b2);
        s->~Self();
    }
};

template<typename F, typename B0, typename B1>
struct AllArgs<F, B0, B1, void, void, void, void, void, void, void, void> {
    typedef AllArgs<F, B0, B1, void, void, void, void, void, void, void, void> Self;
    F f; B0 b0; B1 b1;

    AllArgs(F f, B0 b0=B0(), B1 b1=B1())
        : f(f), b0(b0), b1(b1) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0, s->b1);
        s->~Self();
    }
};

template<typename F, typename B0>
struct AllArgs<F, B0, void, void, void, void, void, void, void, void, void> {
    typedef AllArgs<F, B0, void, void, void, void, void, void, void, void, void> Self;
    F f; B0 b0;

    AllArgs(F f, B0 b0=B0())
        : f(f), b0(b0) {}

    static void copy(void *dest, void *src) {
        new (dest) Self(*(Self*)src);
    }

    static void call_and_destroy(void *data) {
        Self *s = static_cast<Self*>(data);
        s->f(s->b0);
        s->~Self();
    }
};

template<typename F>
struct AllArgs<F, void, void, void, void, void, void, void, void, void, void> {
    typedef AllArgs<F, void, void, void, void, void, void, void, void, void, void> Self;
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


template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
struct PartialArgs;
template <typename R>
struct PartialArgs<R()> {
    typedef AllArgs<void(*)()> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set() {
    }
};
template <typename R, typename A0>
struct PartialArgs<R(A0)> {
    typedef AllArgs<void(*)(A0), A0> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0) {
        all.b0 = a0;
    }
};
template <typename R, typename A0, typename A1>
struct PartialArgs<R(A0, A1)> {
    typedef AllArgs<void(*)(A0, A1), A0, A1> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0, A1 a1) {
        all.b0 = a0;
        all.b1 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2>
struct PartialArgs<R(A0, A1, A2)> {
    typedef AllArgs<void(*)(A0, A1, A2), A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b0 = a0;
        all.b1 = a1;
        all.b2 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3>
struct PartialArgs<R(A0, A1, A2, A3)> {
    typedef AllArgs<void(*)(A0, A1, A2, A3), A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b0 = a0;
        all.b1 = a1;
        all.b2 = a2;
        all.b3 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct PartialArgs<R(A0, A1, A2, A3, A4)> {
    typedef AllArgs<void(*)(A0, A1, A2, A3, A4), A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F>
    PartialArgs(F f): all(f) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b0 = a0;
        all.b1 = a1;
        all.b2 = a2;
        all.b3 = a3;
        all.b4 = a4;
    }
};
template <typename R, typename B0>
struct PartialArgs<R(), B0> {
    typedef AllArgs<void(*)(B0), B0> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0>
struct PartialArgs<R(A0), B0> {
    typedef AllArgs<void(*)(B0, A0), B0, A0> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set(A0 a0) {
        all.b1 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0>
struct PartialArgs<R(A0, A1), B0> {
    typedef AllArgs<void(*)(B0, A0, A1), B0, A0, A1> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set(A0 a0, A1 a1) {
        all.b1 = a0;
        all.b2 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0>
struct PartialArgs<R(A0, A1, A2), B0> {
    typedef AllArgs<void(*)(B0, A0, A1, A2), B0, A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b1 = a0;
        all.b2 = a1;
        all.b3 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0>
struct PartialArgs<R(A0, A1, A2, A3), B0> {
    typedef AllArgs<void(*)(B0, A0, A1, A2, A3), B0, A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b1 = a0;
        all.b2 = a1;
        all.b3 = a2;
        all.b4 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0> {
    typedef AllArgs<void(*)(B0, A0, A1, A2, A3, A4), B0, A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F, typename C0>
    PartialArgs(F f, C0 c0): all(f, c0) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b1 = a0;
        all.b2 = a1;
        all.b3 = a2;
        all.b4 = a3;
        all.b5 = a4;
    }
};
template <typename R, typename B0, typename B1>
struct PartialArgs<R(), B0, B1> {
    typedef AllArgs<void(*)(B0, B1), B0, B1> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1>
struct PartialArgs<R(A0), B0, B1> {
    typedef AllArgs<void(*)(B0, B1, A0), B0, B1, A0> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set(A0 a0) {
        all.b2 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1>
struct PartialArgs<R(A0, A1), B0, B1> {
    typedef AllArgs<void(*)(B0, B1, A0, A1), B0, B1, A0, A1> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set(A0 a0, A1 a1) {
        all.b2 = a0;
        all.b3 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2), B0, B1> {
    typedef AllArgs<void(*)(B0, B1, A0, A1, A2), B0, B1, A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b2 = a0;
        all.b3 = a1;
        all.b4 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1> {
    typedef AllArgs<void(*)(B0, B1, A0, A1, A2, A3), B0, B1, A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b2 = a0;
        all.b3 = a1;
        all.b4 = a2;
        all.b5 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1> {
    typedef AllArgs<void(*)(B0, B1, A0, A1, A2, A3, A4), B0, B1, A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1>
    PartialArgs(F f, C0 c0, C1 c1): all(f, c0, c1) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b2 = a0;
        all.b3 = a1;
        all.b4 = a2;
        all.b5 = a3;
        all.b6 = a4;
    }
};
template <typename R, typename B0, typename B1, typename B2>
struct PartialArgs<R(), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2), B0, B1, B2> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2, A0), B0, B1, B2, A0> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set(A0 a0) {
        all.b3 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2, A0, A1), B0, B1, B2, A0, A1> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set(A0 a0, A1 a1) {
        all.b3 = a0;
        all.b4 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2, A0, A1, A2), B0, B1, B2, A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b3 = a0;
        all.b4 = a1;
        all.b5 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2, A0, A1, A2, A3), B0, B1, B2, A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b3 = a0;
        all.b4 = a1;
        all.b5 = a2;
        all.b6 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2> {
    typedef AllArgs<void(*)(B0, B1, B2, A0, A1, A2, A3, A4), B0, B1, B2, A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2): all(f, c0, c1, c2) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b3 = a0;
        all.b4 = a1;
        all.b5 = a2;
        all.b6 = a3;
        all.b7 = a4;
    }
};
template <typename R, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3), B0, B1, B2, B3> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, A0), B0, B1, B2, B3, A0> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set(A0 a0) {
        all.b4 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, A0, A1), B0, B1, B2, B3, A0, A1> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set(A0 a0, A1 a1) {
        all.b4 = a0;
        all.b5 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, A0, A1, A2), B0, B1, B2, B3, A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b4 = a0;
        all.b5 = a1;
        all.b6 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, A0, A1, A2, A3), B0, B1, B2, B3, A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b4 = a0;
        all.b5 = a1;
        all.b6 = a2;
        all.b7 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2, B3> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, A0, A1, A2, A3, A4), B0, B1, B2, B3, A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3): all(f, c0, c1, c2, c3) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b4 = a0;
        all.b5 = a1;
        all.b6 = a2;
        all.b7 = a3;
        all.b8 = a4;
    }
};
template <typename R, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4), B0, B1, B2, B3, B4> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4, A0), B0, B1, B2, B3, B4, A0> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set(A0 a0) {
        all.b5 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4, A0, A1), B0, B1, B2, B3, B4, A0, A1> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set(A0 a0, A1 a1) {
        all.b5 = a0;
        all.b6 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4, A0, A1, A2), B0, B1, B2, B3, B4, A0, A1, A2> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4, A0, A1, A2, A3), B0, B1, B2, B3, B4, A0, A1, A2, A3> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
        all.b8 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2, B3, B4> {
    typedef AllArgs<void(*)(B0, B1, B2, B3, B4, A0, A1, A2, A3, A4), B0, B1, B2, B3, B4, A0, A1, A2, A3, A4> AllArgs;
    AllArgs all;

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    PartialArgs(F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4): all(f, c0, c1, c2, c3, c4) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
        all.b8 = a3;
        all.b9 = a4;
    }
};

template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
class Task;

template <typename F, typename B0, typename B1, typename B2, typename B3, typename B4>
class Task<F, B0, B1, B2, B3, B4>: public TaskBase {
    typedef PartialArgs<F, B0, B1, B2, B3, B4> PartialArgs;
    typedef PartialArgs::AllArgs AllArgs;
    TaskQueue _queue;
    PartialArgs partial;

    template <typename F>
    Task(TaskQueue *q, F f)
        : TaskBase((void*)&partial.all, sizeof(partial.all), &AllArgs::copy, &AllArgs::call_and_destroy),
          _queue(q), partial(f) {
    }

    template <typename F, typename C0, typename C1, typename C2, typename C3, typename C4>
    Task(TaskQueue *q, F f, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4)
        : TaskBase((void*)&partial.all, sizeof(partial.all), &AllArgs::copy, &AllArgs::call_and_destroy),
          _queue(q), partial(f) {
    }

    void call() {
        //TODO - post
    }

    template <typename A0>
    void call(A0 a0) {
        partial.set(a0);
        //TODO - post
    }

    template <typename A0, typename A1>
    void call(A0 a0, A1 a1) {
        partial.set(a0, a1);
        //TODO - post
    }
};

}

/** @}*/

#endif
