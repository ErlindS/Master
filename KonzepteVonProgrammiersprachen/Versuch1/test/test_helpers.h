#pragma once
#include "aabb.h"
#include "triangle.h"

inline AABB unit_box() {
    AABB b; 
    b.min = {-1,-1,-1}; 
    b.max = {1,1,1}; 
    return b;
}

inline Triangle xy_tri() {
    Triangle t;
    t.v0 = {0,0,0}; t.v1 = {1,0,0}; t.v2 = {0,1,0};
    t.n0 = t.n1 = t.n2 = {0,0,1};
    t.mat_id = 0;
    return t;
}
