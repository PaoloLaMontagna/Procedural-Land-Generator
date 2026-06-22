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
    std::vector<glm::vec3> normals; 

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
        calculate_normals();

    }
    void calculate_normals() {
        // Inizializza tutte le normali a (0,0,0)
        normals.assign(vertices.size(), glm::vec3(0.0f));

        // Somma le normali delle facce ai vertici che le compongono
        for (const auto& t : triangles) {
            glm::vec3 v0 = vertices[t.x];
            glm::vec3 v1 = vertices[t.y];
            glm::vec3 v2 = vertices[t.z];

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            
            // Il prodotto vettoriale dà la direzione perpendicolare alla faccia
            glm::vec3 face_normal = glm::cross(edge1, edge2);

            normals[t.x] += face_normal;
            normals[t.y] += face_normal;
            normals[t.z] += face_normal;
        }

        // Normalizza tutti i vettori (lunghezza uguale a 1)
        for (auto& n : normals) {
            n = glm::normalize(n);
        }
    }
    void pack4gpu (std::vector<float>& points, std::vector<unsigned int>& indices) {
        points.clear();
        for (size_t i = 0; i < vertices.size(); i++) {
            // Posizione X, Y, Z
            points.push_back (vertices[i].x);
            points.push_back (vertices[i].y);
            points.push_back (vertices[i].z);
            
            // Normale X, Y, Z (sostituisce il vecchio colore fittizio!)
            points.push_back (normals[i].x);
            points.push_back (normals[i].y);
            points.push_back (normals[i].z);
        }

        indices.clear();
        for (auto t : triangles) {
            indices.push_back(t.x);
            indices.push_back(t.y);
            indices.push_back(t.z);
        }
    }
};