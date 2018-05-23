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

template<typename A0, typename A1=void, typename A2=void, typename A3=void, typename A4=void, typename A5=void, typename A6=void, typename A7=void, typename A8=void, typename A9=void, typename A10=void>
struct AllArgs;

template<typename B0>
struct AllArgs<B0> {
    typedef AllArgs<B0> Self;
    B0 b0;

    template<typename T0>
    AllArgs(T0 b0=B0()): b0(b0) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0();
            s->~Self();
        }
    };

    typedef Operations<B0, void> ops;
};

template<typename B0, typename B1>
struct AllArgs<B0, B1> {
    typedef AllArgs<B0, B1> Self;
    B0 b0; B1 b1;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1()): b0(b0), b1(b1) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)()> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))();
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)() const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))();
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)() volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))();
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)() const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))();
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2>
struct AllArgs<B0, B1, B2> {
    typedef AllArgs<B0, B1, B2> Self;
    B0 b0; B1 b1; B2 b2;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2()): b0(b0), b1(b1), b2(b2) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3>
struct AllArgs<B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3> Self;
    B0 b0; B1 b1; B2 b2; B3 b3;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3()): b0(b0), b1(b1), b2(b2), b3(b3) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4>
struct AllArgs<B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5>
struct AllArgs<B0, B1, B2, B3, B4, B5> {
    typedef AllArgs<B0, B1, B2, B3, B4, B5> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5, typename B6>
struct AllArgs<B0, B1, B2, B3, B4, B5, B6> {
    typedef AllArgs<B0, B1, B2, B3, B4, B5, B6> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5, s->b6);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5, B6)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5, typename B6, typename B7>
struct AllArgs<B0, B1, B2, B3, B4, B5, B6, B7> {
    typedef AllArgs<B0, B1, B2, B3, B4, B5, B6, B7> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5, B6, B7)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5, typename B6, typename B7, typename B8>
struct AllArgs<B0, B1, B2, B3, B4, B5, B6, B7, B8> {
    typedef AllArgs<B0, B1, B2, B3, B4, B5, B6, B7, B8> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5, B6, B7, B8)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5, typename B6, typename B7, typename B8, typename B9>
struct AllArgs<B0, B1, B2, B3, B4, B5, B6, B7, B8, B9> {
    typedef AllArgs<B0, B1, B2, B3, B4, B5, B6, B7, B8, B9> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8; B9 b9;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8(), B9 b9=B9()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8), b9(b9) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};

template<typename B0, typename B1, typename B2, typename B3, typename B4, typename B5, typename B6, typename B7, typename B8, typename B9, typename B10>
struct AllArgs {
    typedef AllArgs<B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10> Self;
    B0 b0; B1 b1; B2 b2; B3 b3; B4 b4; B5 b5; B6 b6; B7 b7; B8 b8; B9 b9; B10 b10;

    template<typename T0, typename T1>
    AllArgs(T0 b0=B0(), T1 b1=B1(), B2 b2=B2(), B3 b3=B3(), B4 b4=B4(), B5 b5=B5(), B6 b6=B6(), B7 b7=B7(), B8 b8=B8(), B9 b9=B9(), B10 b10=B10()): b0(b0), b1(b1), b2(b2), b3(b3), b4(b4), b5(b5), b6(b6), b7(b7), b8(b8), b9(b9), b10(b10) {}

    template <typename T, typename _>
    struct Operations {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            s->b0(s->b1, s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9, s->b10);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T*, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9, B10)> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9, s->b10);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9, B10) const> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9, s->b10);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9, B10) volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9, s->b10);
            s->~Self();
        }
    };

    template <typename T, typename R, typename U>
    struct Operations<T, R (U::*)(B2, B3, B4, B5, B6, B7, B8, B9, B10) const volatile> {
        static void copy(void *_dest, void *_src)
        {
            new (_dest) Self(*(Self*)_src);
        }

        static void call(void *data) {
            Self *s = static_cast<Self*>(data);
            ((s->b0)->*(s->b1))(s->b2, s->b3, s->b4, s->b5, s->b6, s->b7, s->b8, s->b9, s->b10);
            s->~Self();
        }
    };

    typedef Operations<B0, B1> ops;
};





template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void, typename A0=void, typename A1=void, typename A2=void, typename A3=void, typename A4=void>
struct PartialArgs;


template <typename R, typename A0>
struct PartialArgs<R(A0)> {
    typedef AllArgs<A0> All;
    All all;

    PartialArgs(): all() { }

    void set(A0 a0) {
        all.b0 = a0;
    }
};
template <typename R, typename A0, typename A1>
struct PartialArgs<R(A0, A1)> {
    typedef AllArgs<A0, A1> All;
    All all;

    PartialArgs(): all() { }

    void set(A0 a0, A1 a1) {
        all.b0 = a0;
        all.b1 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2>
struct PartialArgs<R(A0, A1, A2)> {
    typedef AllArgs<A0, A1, A2> All;
    All all;

    PartialArgs(): all() { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b0 = a0;
        all.b1 = a1;
        all.b2 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3>
struct PartialArgs<R(A0, A1, A2, A3)> {
    typedef AllArgs<A0, A1, A2, A3> All;
    All all;

    PartialArgs(): all() { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b0 = a0;
        all.b1 = a1;
        all.b2 = a2;
        all.b3 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct PartialArgs<R(A0, A1, A2, A3, A4)> {
    typedef AllArgs<A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(): all() { }

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
    typedef AllArgs<B0> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0>
struct PartialArgs<R(A0), B0> {
    typedef AllArgs<B0, A0> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

    void set(A0 a0) {
        all.b1 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0>
struct PartialArgs<R(A0, A1), B0> {
    typedef AllArgs<B0, A0, A1> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

    void set(A0 a0, A1 a1) {
        all.b1 = a0;
        all.b2 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0>
struct PartialArgs<R(A0, A1, A2), B0> {
    typedef AllArgs<B0, A0, A1, A2> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b1 = a0;
        all.b2 = a1;
        all.b3 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0>
struct PartialArgs<R(A0, A1, A2, A3), B0> {
    typedef AllArgs<B0, A0, A1, A2, A3> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b1 = a0;
        all.b2 = a1;
        all.b3 = a2;
        all.b4 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0> {
    typedef AllArgs<B0, A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(B0 b0): all(b0) { }

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
    typedef AllArgs<B0, B1> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1>
struct PartialArgs<R(A0), B0, B1> {
    typedef AllArgs<B0, B1, A0> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

    void set(A0 a0) {
        all.b2 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1>
struct PartialArgs<R(A0, A1), B0, B1> {
    typedef AllArgs<B0, B1, A0, A1> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

    void set(A0 a0, A1 a1) {
        all.b2 = a0;
        all.b3 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2), B0, B1> {
    typedef AllArgs<B0, B1, A0, A1, A2> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b2 = a0;
        all.b3 = a1;
        all.b4 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1> {
    typedef AllArgs<B0, B1, A0, A1, A2, A3> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b2 = a0;
        all.b3 = a1;
        all.b4 = a2;
        all.b5 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1> {
    typedef AllArgs<B0, B1, A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(B0 b0, B1 b1): all(b0, b1) { }

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
    typedef AllArgs<B0, B1, B2> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0), B0, B1, B2> {
    typedef AllArgs<B0, B1, B2, A0> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

    void set(A0 a0) {
        all.b3 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1), B0, B1, B2> {
    typedef AllArgs<B0, B1, B2, A0, A1> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

    void set(A0 a0, A1 a1) {
        all.b3 = a0;
        all.b4 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2> {
    typedef AllArgs<B0, B1, B2, A0, A1, A2> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b3 = a0;
        all.b4 = a1;
        all.b5 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2> {
    typedef AllArgs<B0, B1, B2, A0, A1, A2, A3> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b3 = a0;
        all.b4 = a1;
        all.b5 = a2;
        all.b6 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2> {
    typedef AllArgs<B0, B1, B2, A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2): all(b0, b1, b2) { }

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
    typedef AllArgs<B0, B1, B2, B3> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0), B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3, A0> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

    void set(A0 a0) {
        all.b4 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1), B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3, A0, A1> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

    void set(A0 a0, A1 a1) {
        all.b4 = a0;
        all.b5 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3, A0, A1, A2> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b4 = a0;
        all.b5 = a1;
        all.b6 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3, A0, A1, A2, A3> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b4 = a0;
        all.b5 = a1;
        all.b6 = a2;
        all.b7 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2, typename B3>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2, B3> {
    typedef AllArgs<B0, B1, B2, B3, A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3): all(b0, b1, b2, b3) { }

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
    typedef AllArgs<B0, B1, B2, B3, B4> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set() {
    }
};
template <typename R, typename A0, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set(A0 a0) {
        all.b5 = a0;
    }
};
template <typename R, typename A0, typename A1, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0, A1> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set(A0 a0, A1 a1) {
        all.b5 = a0;
        all.b6 = a1;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0, A1, A2> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set(A0 a0, A1 a1, A2 a2) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2, A3), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0, A1, A2, A3> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
        all.b8 = a3;
    }
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename B0, typename B1, typename B2, typename B3, typename B4>
struct PartialArgs<R(A0, A1, A2, A3, A4), B0, B1, B2, B3, B4> {
    typedef AllArgs<B0, B1, B2, B3, B4, A0, A1, A2, A3, A4> All;
    All all;

    PartialArgs(B0 b0, B1 b1, B2 b2, B3 b3, B4 b4): all(b0, b1, b2, b3, b4) { }

    void set(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        all.b5 = a0;
        all.b6 = a1;
        all.b7 = a2;
        all.b8 = a3;
        all.b9 = a4;
    }
};


template <typename F, typename B0=void, typename B1=void, typename B2=void, typename B3=void, typename B4=void>
class Task: public TaskBase {
public:

    Task(TaskQueue *q)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial() {
    }

    template <typename C0>
    Task(TaskQueue *q, C0 c0)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial(c0) {
    }

    template <typename C0, typename C1>
    Task(TaskQueue *q, C0 c0, C1 c1)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial(c0, c1) {
    }

    template <typename C0, typename C1, typename C2>
    Task(TaskQueue *q, C0 c0, C1 c1, C2 c2)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial(c0, c1, c2) {
    }

    template <typename C0, typename C1, typename C2, typename C3>
    Task(TaskQueue *q, C0 c0, C1 c1, C2 c2, C3 c3)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial(c0, c1, c2, c3) {
    }

    template <typename C0, typename C1, typename C2, typename C3, typename C4>
    Task(TaskQueue *q, C0 c0, C1 c1, C2 c2, C3 c3, C4 c4)
        : TaskBase((void*)&_partial.all, sizeof(_partial.all), &All::ops::copy, &All::ops::call),
          _queue(q), _partial(c0, c1, c2, c3, c4) {
    }

    void call() {
        _queue->post(this);
    }

    template <typename A0>
    void call(A0 a0) {
        _partial.set(a0);
        _queue->post(this);
    }

    template <typename A0, typename A1>
    void call(A0 a0, A1 a1) {
        _partial.set(a0, a1);
        _queue->post(this);
    }

    template <typename A0, typename A1, typename A2>
    void call(A0 a0, A1 a1, A2 a2) {
        _partial.set(a0, a1, a2);
        _queue->post(this);
    }

    template <typename A0, typename A1, typename A2, typename A3>
    void call(A0 a0, A1 a1, A2 a2, A3 a3) {
        _partial.set(a0, a1, a2, a3);
        _queue->post(this);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    void call(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
        _partial.set(a0, a1, a2, a3, a4);
        _queue->post(this);
    }

private:
    typedef PartialArgs<F, B0, B1, B2, B3, B4> Partial;
    typedef typename Partial::All All;
    TaskQueue *_queue;
    Partial _partial;
};

}

/** @}*/

#endif
