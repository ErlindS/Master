#pragma once
#include "kdtree.h"
#include "mesh.h"

inline AABB unit_box() {
    AABB b; 
    b.min = glm::vec3(-1,-1,-1); 
    b.max = glm::vec3(1,1,1); 
    return b;
}

inline Triangle xy_tri() {
    Triangle t;
    t.v0 = glm::vec3(0,0,0); t.v1 = glm::vec3(1,0,0); t.v2 = glm::vec3(0,1,0);
    t.n0 = t.n1 = t.n2 = glm::vec3(0,0,1);
    return t;
}
