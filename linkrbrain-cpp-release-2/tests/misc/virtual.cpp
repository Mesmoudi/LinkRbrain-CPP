#include "Exceptions/Exception.hpp"
#include "Logging/Logger.hpp"
#include "LinkRbrain/Models/Point.hpp"
auto& logger = Logging::get_logger();


class A {
public:
    A(int a) : _a(a) {}
protected:
    int _a;
};

class B : public A {
public:
    using A::A;
    virtual int f() { return _a; }
};

class C : public B {
public:
    using B::B;
    virtual int f() { return 2 * _a; }
};


int main(int argc, char const *argv[]) {
    logger.debug(sizeof(A));
    logger.debug(sizeof(B));
    logger.debug(sizeof(C));
    logger.notice("Showed size of A, B, C");
    logger.debug(sizeof(LinkRbrain::Models::WeightedPoint));
    logger.debug(4*sizeof(double));
    logger.notice("Showed size of WeightedPoint, 4*double");
    return 0;
}
