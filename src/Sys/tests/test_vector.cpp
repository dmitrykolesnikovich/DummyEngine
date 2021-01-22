#include "test_common.h"

#include "../SmallVector.h"

void test_vector() {
    { // basic usage with trivial type
        Sys::SmallVector<int, 16> vec;

        for (int i = 0; i < 8; i++) {
            vec.push_back(i);
        }
        for (int i = 8; i < 16; i++) {
            vec.emplace_back(i);
        }

        require(vec.size() == 16);
        require(vec.capacity() == 16);
        require(vec.is_on_heap() == false);

        for (int i = 0; i < 16; i++) {
            require(vec[i] == i);
        }

        vec.push_back(42);

        require(vec.size() == 17);
        require(vec.is_on_heap() == true);

        for (int i = 0; i < 16; i++) {
            require(vec[i] == i);
        }
    }

    { // usage with custom type
        struct AAA {
            float data;

            AAA(float _data) : data(_data) {}

            AAA(const AAA &rhs) = delete;
            AAA(AAA &&rhs) = default;
            AAA &operator=(const AAA &rhs) = delete;
            AAA &operator=(AAA &&rhs) = default;
        };

        Sys::SmallVector<AAA, 16> vec;

        vec.push_back({42.0f});
        vec.emplace_back(13.0f);

        require(vec.size() == 2);
        require(vec.capacity() == 16);
    }
}
