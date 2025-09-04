#pragma once
#include <vector>
#include <CL/cl.h>

struct quad_node {
    cl_double4 domain;
    std::vector<quad_node*> branches={nullptr, nullptr, nullptr, nullptr};
    
    bool isLeaf = false;
    cl_double2 position;
    cl_double mass;
};

struct gpu_quad_node {
    cl_double4 x, y, width, height; // 32 bytes
    cl_double2 position; //16 bytes
    cl_int * childrenIndices[4]; // 
    int particleCount;
    int firstParticleIndex;
};

class QuadTree {
private:
    quad_node* root;

    void getCenterOfMass(std::vector<const cl_double2*>& positions,
                            std::vector<const cl_double*>& masses,
                            cl_double2& position, cl_double& total_mass);

    cl_double distance(const cl_double2& a, const cl_double2& b);

public:
    QuadTree();
    ~QuadTree();
    void build(std::vector<const cl_double2*>& positions,
                std::vector<const cl_double*>& masses,
                quad_node& rootNode, cl_double4 domain);
    
    
    std::vector<gpu_quad_node> getFlattenedTree();
};


