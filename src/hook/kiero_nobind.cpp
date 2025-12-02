#include <kiero.hpp>

namespace kiero::hook {

    Status init() {
        return Status::Success;
    }

    void shutdown() {}

    Status bind(void*, void**, void*) {
        return Status::NotImplementedError;
    }

    Status unbind(void*) {
        return Status::NotImplementedError;
    }
}
