#include "FingerprintModule.h"
#include <iostream>
extern "C" Module* create_module() { return new FingerprintModule(); }
