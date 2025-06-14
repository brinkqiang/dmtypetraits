#include "dmtypetraits.h"

int main(int argc, char* argv[]) {
    Idmtypetraits* module = dmtypetraitsGetModule();
    if (module) {
        module->Release();
    }
    return 0;
}