#include "Resource.h"

namespace CgEngine {
    bool Resource::isLoaded() const {
        return loaded;
    }

    void Resource::setLoaded() {
        this->loaded = true;
    }
}
