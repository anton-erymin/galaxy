#include "integration.h"

namespace integration {

float rect(float a, float b, int n, std::function<float(float)>&& f) {
    float res, h, hh;

    h = (b - a) / n;
    hh = 0.5f * h;

    res = 0.0f;
    for (int i = 0; i < n; i++) {
        res += f(a + i * h + hh);
    }
    res *= h;

    return res;
}

float trap(float a, float b, int n, std::function<float(float)>&& f) {
    float res, h;

    h = (b - a) / n;

    res = 0.5f * (f(a) + f(b));

    for (int i = 1; i < n; i++) {
        res += f(a + i * h);
    }
    res *= h;

    return res;
}

}