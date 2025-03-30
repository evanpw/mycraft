set -euxo pipefail
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebugInfo ..
cmake --build . -j8
cd -
