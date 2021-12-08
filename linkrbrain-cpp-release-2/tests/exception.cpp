#include "Exceptions/Exception.hpp"


class Thrower {
public:
    void test() {
        except();
    }
};

int thrower() {
    except("voici le message de l'exception");
    return 4;
}


int main(int argc, char const *argv[]) {
    try {
        Thrower().test();
    } catch (Exception e) {
        thrower();
    }
    return 0;
}
