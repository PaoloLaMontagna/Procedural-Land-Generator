#pragma once
#include <vector>
#include <iostream>
#include <glm/ext/vector_uint3.hpp>
#include <glm/vec2.hpp> 
#include <glm/vec3.hpp> 
#include <glm/geometric.hpp> 

#include "perlin.hh"
#include "dem.hh" 

class Mesh {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> triangles;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    
    // Dimensioni della mesh
    int m_width;
    int m_depth;

public:
    Mesh(int width, int depth, int octaves, float frequency, float amplitude, unsigned int seed, bool use_dem, std::string dem_file = "aletsch.asc") {
        //ghiacciaio non scalato - lag elevato
        // if (use_dem) {
        //     // ==========================================
        //     // DATI SATELLITARI (GHIACCIAIO)
        //     // ==========================================
        //     Dem dem(dem_file.c_str());
        //     ASC_Header asch = dem.header;
            
        //     m_width = asch.width;
        //     m_depth = asch.height;

        //     // Il punto più in basso a sinistra nel mondo reale
        //     Point bottom_left = asch.img2asc(Point(0, 0));
        //     double min_altitude = dem.min; 
            
        //     // Fattore di scala per far stare una montagna di 4000m dentro la nostra visuale
        //     float scale = 0.05f; 

        //     for (int z = 0; z < m_depth; ++z) {
        //         for (int x = 0; x < m_width; ++x) {
                    
        //             // Coord e Point sono classi fornite dal prof in grid.hh/dem.hh
        //             Point real_coords = asch.img2asc(Point(x, z));
        //             double real_y = dem(Coord(x, z));

        //             // Centriamo la montagna sullo 0,0,0 e la rimpiccioliamo
        //             float vx = (float)(real_coords.x - bottom_left.x) * scale;
        //             float vz = (float)(real_coords.y - bottom_left.y) * scale;
        //             float vy = (float)(real_y - min_altitude) * scale;

        //             vertices.emplace_back(vx, vy, vz);
        //         }
        //     }
        float uv_scale = 0.2f;
        if (use_dem) {
            Dem dem(dem_file.c_str());
            ASC_Header asch = dem.header;
            
            m_width = asch.width;
            m_depth = asch.height;

            // centro della mappa
            Point center_coords = asch.img2asc(Point(m_width / 2, m_depth / 2));
            double min_altitude = dem.min; 
            
            // scalato per la visualizzazione
            float scale = 0.005f; 

            for (int z = 0; z < m_depth; ++z) {
                for (int x = 0; x < m_width; ++x) {
                    
                    Point real_coords = asch.img2asc(Point(x, z));
                    double real_y = dem(Coord(x, z));

                    float vx = (float)(real_coords.x - center_coords.x) * scale;
                    float vz = (float)(real_coords.y - center_coords.y) * scale;
                    float vy = (float)(real_y - min_altitude) * scale;

                    vertices.emplace_back(vx, vy, vz);
                    uvs.emplace_back((float)x * uv_scale, (float)z * uv_scale);
                }
            }

        } else 
        {
            // ==========================================
            // MODALITÀ 1: PROCEDURALE (PERLIN NOISE)
            // ==========================================
            m_width = width;
            m_depth = depth;
            PerlinNoise perlin(seed); 

            for (int z = 0; z < m_depth; ++z) {
                for (int x = 0; x < m_width; ++x) {
                    float y = 0.0f;
                    float current_freq = frequency;
                    float current_amp = amplitude;
                    for(int i = 0; i < octaves; i++) {
                        y += perlin.noise(x * current_freq, z * current_freq) * current_amp;
                        current_freq *= 2.0f;
                        current_amp *= 0.5f;
                    }
                    vertices.emplace_back((float)x, y, (float)z);
                    uvs.emplace_back((float)x * uv_scale, (float)z * uv_scale);

                }
            }
        }

        // ==========================================
        // CREAZIONE DEI TRIANGOLI
        // ==========================================
        for (int z = 0; z < m_depth - 1; ++z) {
            for (int x = 0; x < m_width - 1; ++x) {
                unsigned int topLeft     = z * m_width + x;
                unsigned int topRight    = topLeft + 1;
                unsigned int bottomLeft  = (z + 1) * m_width + x;
                unsigned int bottomRight = bottomLeft + 1;

                triangles.push_back(glm::uvec3(topLeft, bottomLeft, topRight));
                triangles.push_back(glm::uvec3(topRight, bottomLeft, bottomRight));
            }
        }

        // Calcoliamo le luci su qualunque tipo di terreno abbiamo generato
        calculate_normals();
    }

    void calculate_normals() {
        normals.assign(vertices.size(), glm::vec3(0.0f));

        for (const auto& t : triangles) {
            glm::vec3 v0 = vertices[t.x];
            glm::vec3 v1 = vertices[t.y];
            glm::vec3 v2 = vertices[t.z];

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            
            glm::vec3 face_normal = glm::cross(edge1, edge2);

            normals[t.x] += face_normal;
            normals[t.y] += face_normal;
            normals[t.z] += face_normal;
        }

        for (auto& n : normals) {
            n = glm::normalize(n);
        }
    }

    void pack4gpu (std::vector<float>& points, std::vector<unsigned int>& indices) {
        points.clear();
        for (size_t i = 0; i < vertices.size(); i++) {
            points.push_back(vertices[i].x); points.push_back(vertices[i].y); points.push_back(vertices[i].z);
            points.push_back(normals[i].x);  points.push_back(normals[i].y);  points.push_back(normals[i].z);
            
            // Impacchettiamo anche le coordinate U e V
            points.push_back(uvs[i].x);      points.push_back(uvs[i].y);
        }
        indices.clear();
        for (auto t : triangles) {
            indices.push_back(t.x); indices.push_back(t.y); indices.push_back(t.z);
        }
    }
};