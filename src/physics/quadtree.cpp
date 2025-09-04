#include <cmath>
#include <ranges>
#include "physics/quadtree.hpp"




QuadTree::QuadTree(){};



cl_double distance(const cl_double2& a, const cl_double2& b)
{
    cl_double dx = a.s[0] - b.s[0];
    cl_double dy = a.s[1] - b.s[1];
    return std::sqrt(dx * dx + dy * dy);
}



void getCenterOfMass (std::vector<const cl_double2*>& positions, 
                         std::vector<const cl_double*>& masses,
                         cl_double2& position, cl_double& total_mass)
{

    cl_double2 pondered_positions = {0.0, 0.0};
    for (auto [pos, mass] : std::views::zip(positions, masses))
    {
        total_mass += *mass;
        pondered_positions.s[0] += pos->s[0] * (*mass);
        pondered_positions.s[1] += pos->s[1] * (*mass);
    }

    position.s[0] = pondered_positions.s[0] / total_mass;
    position.s[1] = pondered_positions.s[1] / total_mass;

}




void QuadTree::build(std::vector<const cl_double2*>& positions,
                     std::vector<const cl_double*>& masses,
                     quad_node& rootNode, cl_double4 domain)
{
    cl_double2 domainCenter = {(rootNode.domain.s[0] + rootNode.domain.s[1]) / 2,
                               (rootNode.domain.s[2] + rootNode.domain.s[3]) / 2};

    std::vector <std::vector< const cl_double2*>> quadrants(4);
    std::vector <std::vector< const cl_double*>> quadrantMasses(4);
    // separate current particles into quadrants
    // for (auto* pos : positions)
    for (auto [pos, mass] : std::views::zip(positions, masses))
    {
        if (pos->s[0] >= domainCenter.s[0])
        {
            if (pos->s[1] >= domainCenter.s[1])
            {
                quadrants[0].push_back(pos);
                quadrantMasses[0].push_back(mass);
            }
            else
            {
                quadrants[3].push_back(pos);
                quadrantMasses[3].push_back(mass);
            }
        }
        else
        {
            if (pos->s[1] >= domainCenter.s[1])
            {
                quadrants[1].push_back(pos);
                quadrantMasses[1].push_back(mass);
            }
            else
            {
                quadrants[2].push_back(pos);
                quadrantMasses[2].push_back(mass);
            }
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        cl_double4 quadrantDomain;
            
        switch (i)
        {
            case 0:
                quadrantDomain = {domainCenter.s[0], domain.s[1],
                                    domainCenter.s[1], domain.s[3]};
                break;
            case 1:
                quadrantDomain = {domain.s[0], domainCenter.s[0],
                                    domainCenter.s[1], domain.s[3]};
                break;
            case 2:
                quadrantDomain = {domain.s[0], domainCenter.s[0],
                                    domain.s[2], domainCenter.s[1]};
                break;
            case 3:
                quadrantDomain = {domainCenter.s[0], domain.s[1],
                                    domain.s[2], domainCenter.s[1]};
        }

        if(quadrants[i].size() > 1)
        {
            quad_node* branch = new quad_node;
            branch->domain = quadrantDomain;
            getCenterOfMass(quadrants[i], quadrantMasses[i], branch->position, branch->mass);
            rootNode.branches[i] = branch;

            build(quadrants[i], quadrantMasses[i],
                  *branch, quadrantDomain);
        }
        else if(quadrants[i].size() == 1)
        {
            quad_node* leaf = new quad_node;
            leaf->domain = quadrantDomain;
            leaf->isLeaf = true;
            leaf->position = *quadrants[i][0];
            leaf->mass = *quadrantMasses[i][0];

            rootNode.branches[i] = leaf;
        }
    }

}



std::vector<gpu_quad_node> QuadTree::getFlattenedTree()
{
    
}



QuadTree::~QuadTree(){};


