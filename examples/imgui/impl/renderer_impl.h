#pragma once

#include "shared.h"

namespace impl {

    namespace d3d9 {
        void init();
        void shutdown();
    }

    namespace d3d10 {
        void init();
        void shutdown();
    }

    namespace d3d11 {
        void init();
        void shutdown();
    }

    namespace d3d12 {
        void init();
        void shutdown();
    }

    namespace opengl {
        void init();
        void shutdown();
    }
}
