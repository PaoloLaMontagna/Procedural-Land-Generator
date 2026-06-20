#pragma once
#include<stdio.h>
#include<stdlib.h>
#include <vector>
#include <iostream>
#include <glm/ext/vector_uint3.hpp>
#include <glm/vec3.hpp> 
#include <glm/geometric.hpp> 
#include "perlin.hh"

class Mesh {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> triangles;

public:
    Mesh(int width, int depth, int octaves, float frequency, float amplitude,unsigned int seed) {
        PerlinNoise perlin(seed);
        // 1. GENERAZIONE DEI VERTICI (Griglia piatta Y=0)
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                
                float y = 0.0f;
                float current_freq = frequency;
                float current_amp = amplitude;

                // Loop delle ottave (Rumore Frattale)
                for(int i = 0; i < octaves; i++) {
                    // noise() restituisce valori da -1 a 1
                    y += perlin.noise(x * current_freq, z * current_freq) * current_amp;
                    
                    current_freq *= 2.0f; // La frequenza raddoppia (più dettagli)
                    current_amp *= 0.5f;  // L'ampiezza si dimezza (dettagli più piccoli)
                }

                vertices.emplace_back((float)x, y, (float)z);
            }
        }

        // 2. GENERAZIONE DEI TRIANGOLI (Indici)
        for (int z = 0; z < depth - 1; ++z) {
            for (int x = 0; x < width - 1; ++x) {
                unsigned int topLeft     = z * width + x;
                unsigned int topRight    = topLeft + 1;
                unsigned int bottomLeft  = (z + 1) * width + x;
                unsigned int bottomRight = bottomLeft + 1;

                triangles.push_back(glm::uvec3(topLeft, bottomLeft, topRight));
                triangles.push_back(glm::uvec3(topRight, bottomLeft, bottomRight));
            }
        }
    }

    void pack4gpu (std::vector<float>& points, std::vector<unsigned int>& indices) {
        points = {};
        for (auto v : vertices) {
            points.push_back (v.x);
            points.push_back (v.y);
            points.push_back (v.z);
            // Colore fittizio passato al buffer
            points.push_back (0.8f);
            points.push_back (0.6f);
            points.push_back (0.2f);
        }

        indices = {};
        for (auto t : triangles)
            for (unsigned i = 0; i < 3; i++)
                indices.push_back (t[i]);
    }
};