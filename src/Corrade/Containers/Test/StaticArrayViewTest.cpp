/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <sstream>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StaticArrayViewTest: TestSuite::Tester {
    explicit StaticArrayViewTest();

    void constructDefault();
    void constructNullptr();
    void construct();
    void constructFixedSize();
    void constructDerived();

    void convertBool();
    void convertPointer();
    void convertConst();
    void convertVoid();

    void access();
    void accessConst();
    void rangeBasedFor();

    void slice();
    void sliceToStatic();

    void cast();
    void size();
};

typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
template<std::size_t size> using StaticArrayView = Containers::StaticArrayView<size, int>;
template<std::size_t size> using ConstStaticArrayView = Containers::StaticArrayView<size, const int>;
typedef Containers::ArrayView<const void> VoidArrayView;

StaticArrayViewTest::StaticArrayViewTest() {
    addTests({&StaticArrayViewTest::constructDefault,
              &StaticArrayViewTest::constructNullptr,
              &StaticArrayViewTest::construct,
              &StaticArrayViewTest::constructFixedSize,
              &StaticArrayViewTest::constructDerived,

              &StaticArrayViewTest::convertBool,
              &StaticArrayViewTest::convertPointer,
              &StaticArrayViewTest::convertConst,
              &StaticArrayViewTest::convertVoid,

              &StaticArrayViewTest::access,
              &StaticArrayViewTest::accessConst,
              &StaticArrayViewTest::rangeBasedFor,

              &StaticArrayViewTest::slice,
              &StaticArrayViewTest::sliceToStatic,

              &StaticArrayViewTest::cast,
              &StaticArrayViewTest::size});
}

void StaticArrayViewTest::constructDefault() {
    StaticArrayView<5> a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(!a.empty());
    CORRADE_COMPARE(a.size(), StaticArrayView<5>::Size);
    CORRADE_COMPARE(a.size(), 5);

    constexpr StaticArrayView<5> ca;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(!ca.empty());
    CORRADE_COMPARE(ca.size(), StaticArrayView<5>::Size);
    CORRADE_COMPARE(ca.size(), 5);
}

void StaticArrayViewTest::constructNullptr() {
    const StaticArrayView<5> a = nullptr;
    CORRADE_VERIFY(a == nullptr);

    constexpr StaticArrayView<5> ca = nullptr;
    CORRADE_VERIFY(ca == nullptr);
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array30[30]{};

void StaticArrayViewTest::construct() {
    int a[30];

    {
        const StaticArrayView<5> b{a};
        CORRADE_VERIFY(b == a);
    } {
        auto b = staticArrayView<5>(a);
        CORRADE_VERIFY((std::is_same<decltype(b), StaticArrayView<5>>::value));
        CORRADE_VERIFY(b == a);

        auto c = staticArrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), StaticArrayView<5>>::value));
        CORRADE_VERIFY(c == a);
    }

    {
        constexpr ConstStaticArrayView<5> b{Array30};
        CORRADE_VERIFY(b == Array30);
    } {
        constexpr auto b = staticArrayView<5>(Array30);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstStaticArrayView<5>>::value));
        CORRADE_VERIFY(b == Array30);

        constexpr auto c = staticArrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), const ConstStaticArrayView<5>>::value));
        CORRADE_VERIFY(c == Array30);
    }

    /* Implicit construction from pointer should not be allowed */
    CORRADE_VERIFY(!(std::is_convertible<int*, StaticArrayView<5>>::value));
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array13[13]{};

void StaticArrayViewTest::constructFixedSize() {
    int a[13];

    {
        StaticArrayView<13> b = a;
        CORRADE_VERIFY(b == a);
    } {
        auto b = staticArrayView(a);
        CORRADE_VERIFY((std::is_same<decltype(b), StaticArrayView<13>>::value));
        CORRADE_VERIFY(b == a);
    }

    {
        constexpr ConstStaticArrayView<13> b = Array13;
        CORRADE_VERIFY(b == Array13);
    } {
        constexpr auto b = staticArrayView(Array13);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstStaticArrayView<13>>::value));
        CORRADE_VERIFY(b == Array13);
    }
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(): i{} {}
    int i;
};
struct Derived: Base {
    constexpr Derived() {}
};
constexpr Derived DerivedArray[5]
    /* This missing makes MSVC2015 complain it's not constexpr, but if present
       then GCC 4.8 fails to build. Eh. ¯\_(ツ)_/¯ */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    {}
    #endif
    ;

void StaticArrayViewTest::constructDerived() {
    /* See ArrayViewTest for comments */

    Derived b[5];
    Containers::StaticArrayView<5, Derived> bv{b};
    Containers::StaticArrayView<5, Base> a{b};
    Containers::StaticArrayView<5, Base> av{bv};

    CORRADE_VERIFY(a == &b[0]);
    CORRADE_VERIFY(av == &b[0]);

    constexpr Containers::StaticArrayView<5, const Derived> cbv{DerivedArray};
    constexpr Containers::StaticArrayView<5, const Base> ca{DerivedArray};
    constexpr Containers::StaticArrayView<5, const Base> cav{cbv};

    CORRADE_VERIFY(ca == &DerivedArray[0]);
    CORRADE_VERIFY(cav == &DerivedArray[0]);
}

void StaticArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(StaticArrayView<5>{a});
    CORRADE_VERIFY(!StaticArrayView<5>{});

    constexpr ConstStaticArrayView<30> cb = Array30;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(boolCb);

    constexpr ConstStaticArrayView<30> cc;
    constexpr bool boolCc = !!cc;
    CORRADE_VERIFY(!boolCc);

    CORRADE_VERIFY(!(std::is_convertible<StaticArrayView<5>, int>::value));
}

void StaticArrayViewTest::convertPointer() {
    int a[7];
    StaticArrayView<7> b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const StaticArrayView<7> c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    constexpr ConstStaticArrayView<13> cc = Array13;
    constexpr const int* ccp = cc;
    CORRADE_COMPARE(ccp, static_cast<const int*>(Array13));

    /* Pointer arithmetic */
    const StaticArrayView<7> e = a;
    const int* ep = e + 2;
    CORRADE_COMPARE(ep, &e[2]);
}

void StaticArrayViewTest::convertConst() {
    int a[3];
    StaticArrayView<3> b = a;
    ConstArrayView c = b;
    CORRADE_VERIFY(c == a);
}

void StaticArrayViewTest::convertVoid() {
    int a[] = {3, 4, 7, 12, 0, -15};

    /** @todo C++14: test that all the operations are really constexpr (C++11 doesn't allow void conversions IMHO) */

    /* void reference to ArrayView */
    StaticArrayView<6> b = a;
    const StaticArrayView<6> cb = a;
    VoidArrayView c = b;
    VoidArrayView cc = cb;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == cb);
    CORRADE_COMPARE(c.size(), 6*sizeof(int));
    CORRADE_COMPARE(cc.size(), 6*sizeof(int));

    /* void reference to ArrayView */
    constexpr ConstStaticArrayView<13> ccb = Array13;
    VoidArrayView ccc = ccb;
    CORRADE_VERIFY(ccc == ccb);
    CORRADE_COMPARE(ccc.size(), 13*sizeof(int));
}

/* Needs to be here in order to use it in constexpr */
constexpr int OneToSeven[]{0, 1, 2, 3, 4, 5, 6};

void StaticArrayViewTest::access() {
    int a[7];
    StaticArrayView<7> b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), 7);
    CORRADE_COMPARE(b.cbegin(), b.begin());
    CORRADE_COMPARE(b.cend(), b.end());

    ConstStaticArrayView<7> c = a;
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstStaticArrayView<7> cb = OneToSeven;

    constexpr const int* data = cb.data();
    CORRADE_VERIFY(data == OneToSeven);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7);

    constexpr const int* begin = cb.begin();
    constexpr const int* cbegin = cb.cbegin();
    CORRADE_COMPARE(begin, OneToSeven);
    CORRADE_COMPARE(cbegin, OneToSeven);
}

void StaticArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const StaticArrayView<7> b = a;
    b.front() = 0;
    *(b.begin()+1) = 1;
    *(b.cbegin()+2) = 2;
    b[3] = 3;
    *(b.end()-3) = 4;
    *(b.cend()-2) = 5;
    b.back() = 6;

    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a[5], 5);
    CORRADE_COMPARE(a[6], 6);
}

void StaticArrayViewTest::rangeBasedFor() {
    int a[5];
    StaticArrayView<5> b = a;
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

void StaticArrayViewTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

    ArrayView b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayView c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);
}

void StaticArrayViewTest::sliceToStatic() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

    StaticArrayView<3> b = a.slice<3>(1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StaticArrayView<3> c = a.prefix<3>();
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);
}

void StaticArrayViewTest::cast() {
    std::uint32_t data[6]{};
    Containers::StaticArrayView<6, std::uint32_t> a = data;
    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto c = Containers::arrayCast<std::uint16_t>(a);
    auto d = Containers::arrayCast<std::uint16_t>(data);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::StaticArrayView<3, std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(c), Containers::StaticArrayView<12, std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(d), Containers::StaticArrayView<12, std::uint16_t>>::value));
    CORRADE_COMPARE(reinterpret_cast<void*>(b.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<void*>(c.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<void*>(d.begin()), reinterpret_cast<void*>(a.begin()));
}

void StaticArrayViewTest::size() {
    int a[6]{};
    StaticArrayView<3> b{a};

    CORRADE_COMPARE(Containers::arraySize(b), 3);

    constexpr ConstStaticArrayView<3> cb{Array13};
    constexpr std::size_t size = Containers::arraySize(cb);
    CORRADE_COMPARE(size, 3);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewTest)
