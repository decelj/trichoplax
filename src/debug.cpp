#include "debug.h"

void print4x4(const glm::mat4 &m) {
    for (int i = 0; i < 4; i++)
        printf("%.2f, %.2f, %.2f, %.2f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
    printf("\n");
}

void printv3(const glm::vec3 &v) {
    printf("%.2f, %.2f, %.2f\n", v[0], v[1], v[2]);
}
