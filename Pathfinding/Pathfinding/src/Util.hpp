#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "Common.h"
#include "glm/gtx/intersect.hpp"
#include <random>
#include <format>
#include <filesystem>
#include "Renderer/RendererCommon.h"

namespace Util {

    inline bool AreEqual(float a, float b, float epsilon = 1e-6f) {
        return std::fabs(a - b) < epsilon;
    }

    inline bool IsPointInRectangle(const glm::vec2& p, const glm::vec2& rectTopLeft, const glm::vec2& rectBottomRight) {
        return (p.x >= rectTopLeft.x && p.x <= rectBottomRight.x &&
            p.y >= rectTopLeft.y && p.y <= rectBottomRight.y);
    }

    inline bool DoLineSegmentsIntersect(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4) {
        auto cross = [](float ax, float ay, float bx, float by) -> float {
            return ax * by - ay * bx;
        };
        float dx1 = p2.x - p1.x;
        float dy1 = p2.y - p1.y;
        float dx2 = p4.x - p3.x;
        float dy2 = p4.y - p3.y;
        float denominator = cross(dx1, dy1, dx2, dy2);
        if (AreEqual(denominator, 0.0f)) {
            // Lines are parallel, check if they are collinear
            if (AreEqual(cross(p3.x - p1.x, p3.y - p1.y, dx1, dy1), 0.0f)) {
                // Collinear, check if they overlap
                float minX1 = std::min(p1.x, p2.x);
                float maxX1 = std::max(p1.x, p2.x);
                float minY1 = std::min(p1.y, p2.y);
                float maxY1 = std::max(p1.y, p2.y);
                float minX2 = std::min(p3.x, p4.x);
                float maxX2 = std::max(p3.x, p4.x);
                float minY2 = std::min(p3.y, p4.y);
                float maxY2 = std::max(p3.y, p4.y);
                return !(maxX2 < minX1 || maxX1 < minX2 || maxY2 < minY1 || maxY1 < minY2);
            }
            return false;
        }
        float s = cross(p1.x - p3.x, p1.y - p3.y, dx2, dy2) / denominator;
        float t = cross(p1.x - p3.x, p1.y - p3.y, dx1, dy1) / denominator;
        return s >= 0 && s <= 1 && t >= 0 && t <= 1;
    }

    inline bool DoesLineSegmentOverlapRectangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& rectTopLeft, const glm::vec2& rectBottomRight) {
        if (IsPointInRectangle(p1, rectTopLeft, rectBottomRight) || IsPointInRectangle(p2, rectTopLeft, rectBottomRight)) {
            return true;
        }
        glm::vec2 rectTopRight = { rectBottomRight.x, rectTopLeft.y };
        glm::vec2 rectBottomLeft = { rectTopLeft.x, rectBottomRight.y };
        return DoLineSegmentsIntersect(p1, p2, rectTopLeft, rectTopRight) ||
            DoLineSegmentsIntersect(p1, p2, rectTopRight, rectBottomRight) ||
            DoLineSegmentsIntersect(p1, p2, rectBottomRight, rectBottomLeft) ||
            DoLineSegmentsIntersect(p1, p2, rectBottomLeft, rectTopLeft);
    }

    inline bool GetLineSegmentIntersection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4, glm::vec2& intersection) {
        auto cross = [](float ax, float ay, float bx, float by) -> float {
            return ax * by - ay * bx;
        };

        float dx1 = p2.x - p1.x;
        float dy1 = p2.y - p1.y;
        float dx2 = p4.x - p3.x;
        float dy2 = p4.y - p3.y;

        float denominator = cross(dx1, dy1, dx2, dy2);
        if (AreEqual(denominator, 0.0f)) {
            return false;  // Parallel or collinear
        }

        float s = cross(p1.x - p3.x, p1.y - p3.y, dx2, dy2) / denominator;
        float t = cross(p1.x - p3.x, p1.y - p3.y, dx1, dy1) / denominator;

        if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
            intersection = glm::vec2(p1.x + s * dx1, p1.y + s * dy1);
            return true;
        }
        return false;  // No intersection
    }

    inline bool DoesLineSegmentOverlapRectangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& rectTopLeft, const glm::vec2& rectBottomRight, glm::vec2& closestIntersection) {
        bool foundIntersection = false;
        float minDistance = std::numeric_limits<float>::max();
        glm::vec2 intersection;

        // Define the four corners of the rectangle
        glm::vec2 rectTopRight = { rectBottomRight.x, rectTopLeft.y };
        glm::vec2 rectBottomLeft = { rectTopLeft.x, rectBottomRight.y };
        // List of rectangle edges
        std::vector<std::pair<glm::vec2, glm::vec2>> rectEdges = {
            {rectTopLeft, rectTopRight},
            {rectTopRight, rectBottomRight},
            {rectBottomRight, rectBottomLeft},
            {rectBottomLeft, rectTopLeft}
        };
        // Check if either endpoint is inside the rectangle
        if (IsPointInRectangle(p1, rectTopLeft, rectBottomRight)) {
            closestIntersection = p1;
            return true;
        }
        if (IsPointInRectangle(p2, rectTopLeft, rectBottomRight)) {
            closestIntersection = p2;
            return true;
        }
        // Check for intersections with the rectangle's edges
        for (const auto& edge : rectEdges) {
            if (GetLineSegmentIntersection(p1, p2, edge.first, edge.second, intersection)) {
                float distance = glm::distance(p1, intersection);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestIntersection = intersection;
                    foundIntersection = true;
                }
            }
        }
        return foundIntersection;
    }


    inline glm::vec3 ScreenToNDC(glm::vec2 screenPos, glm::vec2 viewportSize) {
        float ndcX = 2.0f * (static_cast<float>(screenPos.x) / viewportSize.x) - 1.0f;
        float ndcY = 1.0f - 2.0f * (static_cast<float>(screenPos.y) / viewportSize.y);
        return glm::vec3(ndcX, ndcY, 0);
    }

    inline glm::vec3 Vec3Min(const glm::vec3& a, const glm::vec3& b) {
        return glm::vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    }

    inline glm::vec3 Vec3Max(const glm::vec3& a, const glm::vec3& b) {
        return glm::vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
    }

    inline std::string Lowercase(std::string& str) {
        std::string result = "";
        for (auto& c : str) {
            result += std::tolower(c);
        }
        return result;
    }

    inline std::string Uppercase(std::string& str) {
        std::string result = "";
        for (auto& c : str) {
            result += std::toupper(c);
        }
        return result;
    }

    inline std::string DebugLineRenderModeToString(DebugLineRenderMode mode) {

        if (mode == DebugLineRenderMode::SHOW_NO_LINES) {
            return "NONE";
        }
        else if (mode == DebugLineRenderMode::PHYSX_ALL) {
            return "PHYSX_ALL";
        }
        else if (mode == DebugLineRenderMode::PHYSX_RAYCAST) {
            return "PHYSX_RAYCAST";
        }
        else if (mode == DebugLineRenderMode::PHYSX_COLLISION) {
            return "PHYSX_COLLISION";
        }
        else if (mode == DebugLineRenderMode::RAYTRACE_LAND) {
            return "RAYTRACE_LAND";
        }
        else if (mode == DebugLineRenderMode::PHYSX_EDITOR) {
            return "PHYSX_EDITOR";
        }
        else if (mode == DebugLineRenderMode::BOUNDING_BOXES) {
            return "BOUNDING_BOXES";
        }
        else if (mode == DebugLineRenderMode::RTX_LAND_AABBS) {
            return "RTX_LAND_AABBS";
        }
        else if (mode == DebugLineRenderMode::RTX_LAND_TRIS) {
            return "RTX_LAND_TRIS";
        }
        else if (mode == DebugLineRenderMode::RTX_LAND_TOP_LEVEL_ACCELERATION_STRUCTURE) {
            return "RTX_LAND_TOP_LEVEL_ACCELERATION_STRUCTURE";
        }
        else if (mode == DebugLineRenderMode::RTX_LAND_BOTTOM_LEVEL_ACCELERATION_STRUCTURES) {
            return "RTX_LAND_BOTTOM_LEVEL_ACCELERATION_STRUCTURES";
        }
        else if (mode == DebugLineRenderMode::RTX_LAND_TOP_AND_BOTTOM_LEVEL_ACCELERATION_STRUCTURES) {
            return "RTX_LAND_TOP_AND_BOTTOM_LEVEL_ACCELERATION_STRUCTURES";
        }
        else {
            return "UNDEFINED";
        }
    }

    inline std::string SplitscreenModeToString(SplitscreenMode mode) {
        if (mode == SplitscreenMode::NONE) {
            return "NONE";
        }
        else if (mode == SplitscreenMode::TWO_PLAYER) {
            return "TWO_PLAYER";
        }
        else if (mode == SplitscreenMode::FOUR_PLAYER) {
            return "FOUR_PLAYER";
        }
        else {
            return "UNDEFINED";
        }
    }

    inline std::string RenderModeToString(RenderMode mode) {
        if (mode == RenderMode::COMPOSITE) {
            return "COMPOSITE";
        }
        else if (mode == RenderMode::POINT_CLOUD) {
            return "POINT_CLOUD";
        }
        else if (mode == RenderMode::DIRECT_LIGHT) {
            return "DIRECT_LIGHT";
        }
        else if (mode == RenderMode::INDIRECT_LIGHT) {
            return "INDIRECT_LIGHT";
        }
        else if (mode == RenderMode::PROPAGATION_GRID) {
            return "PROPAGATION_GRID";
        }
        else if (mode == RenderMode::POINT_CLOUD_PROPAGATION_GRID) {
            return "POINT_CLOUD_PROPAGATION_GRID";
        }
        else {
            return "UNDEFINED";
        }
    }

    inline std::vector<Vertex> GetAABBVertices(AABB& aabb, glm::vec3 color) {
        std::vector<Vertex> vertices;
        glm::vec4 frontTopLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMax.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontTopRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMax.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontBottomLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMin.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontBottomRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMin.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 backTopLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMax.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backTopRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMax.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backBottomLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMin.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backBottomRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMin.y, aabb.boundsMin.z, 1.0f);
        vertices.push_back(Vertex(frontBottomLeft, color));
        vertices.push_back(Vertex(frontBottomRight, color));
        vertices.push_back(Vertex(frontTopLeft, color));
        vertices.push_back(Vertex(frontTopRight, color));
        vertices.push_back(Vertex(frontBottomLeft, color));
        vertices.push_back(Vertex(frontTopLeft, color));
        vertices.push_back(Vertex(frontBottomRight, color));
        vertices.push_back(Vertex(frontTopRight, color));
        vertices.push_back(Vertex(backBottomLeft, color));
        vertices.push_back(Vertex(backBottomRight, color));
        vertices.push_back(Vertex(backTopLeft, color));
        vertices.push_back(Vertex(backTopRight, color));
        vertices.push_back(Vertex(backBottomLeft, color));
        vertices.push_back(Vertex(backTopLeft, color));
        vertices.push_back(Vertex(backBottomRight, color));
        vertices.push_back(Vertex(backTopRight, color));
        vertices.push_back(Vertex(frontBottomLeft, color));
        vertices.push_back(Vertex(backBottomLeft, color));
        vertices.push_back(Vertex(frontBottomRight, color));
        vertices.push_back(Vertex(backBottomRight, color));
        vertices.push_back(Vertex(frontTopLeft, color));
        vertices.push_back(Vertex(backTopLeft, color));
        vertices.push_back(Vertex(frontTopRight, color));
        vertices.push_back(Vertex(backTopRight, color));
        return vertices;
    }

    inline float SquaredDistPointAABB(const glm::vec3 p, const AABB& aabb) {
        auto check = [&](
            const double pn,
            const double bmin,
            const double bmax) -> double {
            double out = 0;
            double v = pn;
            if (v < bmin) {
                double val = (bmin - v);
                out += val * val;
            }
            if (v > bmax) {
                double val = (v - bmax);
                out += val * val;
            }
            return out;
        };
        // Squared distance
        double sq = 0.0;
        sq += check(p.x, aabb.boundsMin.x, aabb.boundsMax.x);
        sq += check(p.y, aabb.boundsMin.y, aabb.boundsMax.y);
        sq += check(p.z, aabb.boundsMin.z, aabb.boundsMax.z);
        return (float)sq;
    }

    inline bool AABBInSphere(AABB& aabb, glm::vec3 spherePosition, float sphereRadius) {
        float squaredDistance = SquaredDistPointAABB(spherePosition, aabb);
        return squaredDistance <= (sphereRadius * sphereRadius);
    }

    inline std::string WeaponActionToString(WeaponAction weaponAction) {

        if (weaponAction == IDLE) {
            return "IDLE";
        }
        else if (weaponAction == FIRE) {
            return "FIRE";
        }
        else if (weaponAction == RELOAD) {
            return "RELOAD";
        }
        else if (weaponAction == RELOAD_FROM_EMPTY) {
            return "RELOAD_FROM_EMPTY";
        }
        else if (weaponAction == DRAW_BEGIN) {
            return "DRAW_BEGIN";
        }
        else if (weaponAction == DRAWING) {
            return "DRAWING";
        }
        else if (weaponAction == SPAWNING) {
            return "SPAWNING";
        }
        else if (weaponAction == RELOAD_SHOTGUN_BEGIN) {
            return "RELOAD_SHOTGUN_BEGIN";
        }
        else if (weaponAction == RELOAD_SHOTGUN_SINGLE_SHELL) {
            return "RELOAD_SHOTGUN_SINGLE_SHELL";
        }
        else if (weaponAction == RELOAD_SHOTGUN_DOUBLE_SHELL) {
            return "RELOAD_SHOTGUN_DOUBLE_SHELL";
        }
        else if (weaponAction == RELOAD_SHOTGUN_END) {
            return "RELOAD_SHOTGUN_END";
        }
        else if (weaponAction == ADS_IN) {
            return "ADS_IN";
        }
        else if (weaponAction == ADS_OUT) {
            return "ADS_OUT";
        }
        else if (weaponAction == ADS_IDLE) {
            return "ADS_IDLE";
        }
        else if (weaponAction == ADS_FIRE) {
            return "ADS_FIRE";
        }
        else {
            return "UNKNOWN WEAPON ACTION";
        }
    }


    inline glm::vec3 GetMouseRay(glm::mat4 projection, glm::mat4 view, int windowWidth, int windowHeight, int mouseX, int mouseY) {
		float x = (2.0f * mouseX) / (float)windowWidth - 1.0f;
		float y = 1.0f - (2.0f * mouseY) / (float)windowHeight;
		float z = 1.0f;
		glm::vec3 ray_nds = glm::vec3(x, y, z);
		glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, ray_nds.z, 1.0f);
		glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, ray_eye.z, 0.0f);
		glm::vec4 inv_ray_wor = (inverse(view) * ray_eye);
		glm::vec3 ray_wor = glm::vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
		ray_wor = normalize(ray_wor);
		return ray_wor;
    }


    inline void DrawFrontFacingPlane(int instanceCount) {
        static unsigned int frontFacingPlaneVAO = 0;
        float offset = 0.1f;
        if (frontFacingPlaneVAO == 0) {
            Vertex vert0, vert1, vert2, vert3;
            vert0.position = glm::vec3(-0.5, 0.5, offset);
            vert1.position = glm::vec3(0.5, 0.5f, offset);
            vert2.position = glm::vec3(0.5, -0.5, offset);
            vert3.position = glm::vec3(-0.5, -0.5, offset);
            vert0.uv = glm::vec2(0, 1);
            vert1.uv = glm::vec2(1, 1);
            vert2.uv = glm::vec2(1, 0);
            vert3.uv = glm::vec2(0, 0);
            vert0.normal = glm::vec3(0, 0, 1);
            vert1.normal = glm::vec3(0, 0, 1);
            vert2.normal = glm::vec3(0, 0, 1);
            vert3.normal = glm::vec3(0, 0, 1);
            vert0.tangent = glm::vec3(1, 0, 0);
            vert1.tangent = glm::vec3(1, 0, 0);
            vert2.tangent = glm::vec3(1, 0, 0);
            vert3.tangent = glm::vec3(1, 0, 0);
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            unsigned int i = (unsigned int)vertices.size();
            indices.push_back(i + 2);
            indices.push_back(i + 1);
            indices.push_back(i + 0);
            indices.push_back(i + 0);
            indices.push_back(i + 3);
            indices.push_back(i + 2);
            vertices.push_back(vert0);
            vertices.push_back(vert1);
            vertices.push_back(vert2);
            vertices.push_back(vert3);
            unsigned int VBO;
            unsigned int EBO;
            glGenVertexArrays(1, &frontFacingPlaneVAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            glBindVertexArray(frontFacingPlaneVAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        }
        glBindVertexArray(frontFacingPlaneVAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCount);
    }

    inline glm::vec3 ClosestPointOnLine(glm::vec3 point, glm::vec3 start, glm::vec3 end) {
        glm::vec2 p(point.x, point.z);
        glm::vec2 v(start.x, start.z);
        glm::vec2 w(end.x, end.z);
        const float l2 = ((v.x - w.x) * (v.x - w.x)) + ((v.y - w.y) * (v.y - w.y));
        if (l2 == 0.0)
            return glm::vec3(0);
        const float t = std::max(0.0f, std::min(1.0f, dot(p - v, w - v) / l2));
        const glm::vec2 projection = v + t * (w - v);
        return glm::vec3(projection.x, 0, projection.y);
    }

    inline float DistanceSquared(glm::vec3 A, glm::vec3 B) {
        glm::vec3 C = A - B;
        return glm::dot(C, C);
    }

    inline glm::vec3 Translate(glm::mat4& translation, glm::vec3 position) {
        return translation * glm::vec4(position, 1.0);
    }

    inline float RandomFloat(float min, float max) {
        return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
    }

    inline int RandomInt(int min, int max) {
        static std::random_device dev;
        static std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max);
        return dist6(rng);
    }

    inline std::string ReadTextFromFile(std::string path) {
        std::ifstream file(path);
        std::string str;
        std::string line;
        while (std::getline(file, line)) {
            str += line + "\n";
        }
        return str;
    }

    inline std::string Vec3ToString(glm::vec3 v) {
        return std::string("(" + std::format("{:.2f}", v.x) + ", " + std::format("{:.2f}", v.y) + ", " + std::format("{:.2f}", v.z) + ")");
    }
    inline std::string Vec3ToString10(glm::vec3 v) {
        return std::string("(" + std::format("{:.10f}", v.x) + ", " + std::format("{:.10f}", v.y) + ", " + std::format("{:.10f}", v.z) + ")");
    }

    inline std::string QuatToString(glm::quat q) {
        return std::string("(" + std::format("{:.2f}", q.x) + ", " + std::format("{:.2f}", q.y) + ", " + std::format("{:.2f}", q.z) + ", " + std::format("{:.2f}", q.w) + ")");
    }


    inline std::string Mat4ToString(glm::mat4 m) {
        std::string result;
       /* result += std::format("{:.2f}", m[0][0]) + ", " + std::format("{:.2f}", m[1][0]) + ", " + std::format("{:.2f}", m[2][0]) + ", " + std::format("{:.2f}", m[3][0]) + "\n";
        result += std::format("{:.2f}", m[0][1]) + ", " + std::format("{:.2f}", m[1][1]) + ", " + std::format("{:.2f}", m[2][1]) + ", " + std::format("{:.2f}", m[3][1]) + "\n";
        result += std::format("{:.2f}", m[0][2]) + ", " + std::format("{:.2f}", m[1][2]) + ", " + std::format("{:.2f}", m[2][2]) + ", " + std::format("{:.2f}", m[3][2]) + "\n";
        result += std::format("{:.2f}", m[0][3]) + ", " + std::format("{:.2f}", m[1][3]) + ", " + std::format("{:.2f}", m[2][3]) + ", " + std::format("{:.2f}", m[3][3]);
       */
        result += std::format("{:.2f}", m[0][0]) + ", " + std::format("{:.2f}", m[1][0]) + ", " + std::format("{:.2f}", m[2][0]) + ", " + std::format("{:.2f}", m[3][0]) + "\n";
        result += std::format("{:.2f}", m[0][1]) + ", " + std::format("{:.2f}", m[1][1]) + ", " + std::format("{:.2f}", m[2][1]) + ", " + std::format("{:.2f}", m[3][1]) + "\n";
        result += std::format("{:.2f}", m[0][2]) + ", " + std::format("{:.2f}", m[1][2]) + ", " + std::format("{:.2f}", m[2][2]) + ", " + std::format("{:.2f}", m[3][2]) + "\n";
        result += std::format("{:.2f}", m[0][3]) + ", " + std::format("{:.2f}", m[1][3]) + ", " + std::format("{:.2f}", m[2][3]) + ", " + std::format("{:.2f}", m[3][3]);
        return result;
    }

    inline bool FileExists(const std::string& name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    inline bool FileExists(const std::string_view name) {
        struct stat buffer;
        return (stat(name.data(), &buffer) == 0);
    }

    inline float FInterpTo(float current, float target, float deltaTime, float interpSpeed) {
        // If no interp speed, jump to target value
        if (interpSpeed <= 0.f)
            return target;
        // Distance to reach
        const float Dist = target - current;
        // If distance is too small, just set the desired location
        if (Dist * Dist < SMALL_NUMBER)
            return target;
        // Delta Move, Clamp so we do not over shoot.
        const float DeltaMove = Dist * glm::clamp(deltaTime * interpSpeed, 0.0f, 1.0f);
        return current + DeltaMove;
    }

    inline IntersectionResult RayTriangleIntersectTest(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 o, glm::vec3 n) {

        IntersectionResult result;
        result.found = glm::intersectRayTriangle(o, n, p1, p2, p3, result.baryPosition, result.distance);
        //result.distance *= -1;
        return result;
    }

    inline glm::vec3 NormalFromTriangle(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2) {
        return glm::normalize(glm::cross(pos1 - pos0, pos2 - pos0));
    }

    inline glm::vec3 NormalFromTriangle(Triangle triangle) {
        glm::vec3 pos0 = triangle.v0;
        glm::vec3 pos1 = triangle.v1;
        glm::vec3 pos2 = triangle.v2;
        return glm::normalize(glm::cross(pos1 - pos0, pos2 - pos0));
    }

    inline void SetNormalsAndTangentsFromVertices(Vertex* vert0, Vertex* vert1, Vertex* vert2) {
        // Shortcuts for UVs
        glm::vec3& v0 = vert0->position;
        glm::vec3& v1 = vert1->position;
        glm::vec3& v2 = vert2->position;
        glm::vec2& uv0 = vert0->uv;
        glm::vec2& uv1 = vert1->uv;
        glm::vec2& uv2 = vert2->uv;
        // Edges of the triangle : position delta. UV delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        glm::vec3 normal = NormalFromTriangle(vert0->position, vert1->position, vert2->position);
        vert0->normal = normal;
        vert1->normal = normal;
        vert2->normal = normal;
        vert0->tangent = tangent;
        vert1->tangent = tangent;
        vert2->tangent = tangent;
    }
    /*
    inline float GetMaxXPointOfTri(Triangle& tri) {
        return std::max(std::max(tri.v0.x, tri.p2.x), tri.p3.x);
    }
    inline float GetMaxYPointOfTri(Triangle& tri) {
        return std::max(std::max(tri.v0.y, tri.p2.y), tri.p3.y);
    }
    inline float GetMaxZPointOfTri(Triangle& tri) {
        return std::max(std::max(tri.v0.z, tri.p2.z), tri.p3.z);
    }
    inline float GetMinXPointOfTri(Triangle& tri) {
        return std::min(std::min(tri.v0.x, tri.p2.x), tri.p3.x);
    }
    inline float GetMinYPointOfTri(Triangle& tri) {
        return std::min(std::min(tri.v0.y, tri.p2.y), tri.p3.y);
    }
    inline float GetMinZPointOfTri(Triangle& tri) {
        return std::min(std::min(tri.v0.z, tri.p2.z), tri.p3.z);
    }*/

   /* namespace RayTracing {

        inline bool AnyHit(std::vector<Triangle>& triangles, glm::vec3 rayOrign, glm::vec3 rayDir, float minDist, float maxDist) {

            for (Triangle& tri : triangles) {

                // Skip back facing
                if (glm::dot(rayDir, tri.normal) < 0) {
                    continue;
                }
                // Cast ray
                IntersectionResult result = Util::RayTriangleIntersectTest(tri.p1, tri.p2, tri.p3, rayOrign, rayDir);
                if (result.found && result.distance < maxDist && result.distance > minDist) {
                    return true;
                }
            }
            // No hit found
            return false;
        }
    }*/

    inline float MapRange(float inValue, float minInRange, float maxInRange, float minOutRange, float maxOutRange) {
        float x = (inValue - minInRange) / (maxInRange - minInRange);
        return minOutRange + (maxOutRange - minOutRange) * x;
    }

    /*inline int MapRange(int inValue, int minInRange, int maxInRange, int minOutRange, int maxOutRange) {
        float x = (inValue - minInRange) / (float)(maxInRange - minInRange);
        return minOutRange + (maxOutRange - minOutRange) * x;
    }*/

    /*
    inline void AnyHit(glm::vec3 rayOrigin, glm::vec3 rayDirection, float maxDistance, std::vector<Triangle>& triangles, RayCastResult& out, bool ignoreBackFacing = true) {

        for (Triangle& triangle : triangles) {

            // Skip if triangle is facing away from light direction
            float vdotn = dot(rayDirection, triangle.normal);
            if (ignoreBackFacing && vdotn > 0) {
                continue;
            }

            IntersectionResult result = RayTriangleIntersectTest(triangle.p1, triangle.p2, triangle.p3, rayOrigin, rayDirection);
            float minDist = 0.01f;
            if (result.found && result.distance > minDist && result.distance < maxDistance) {
                if (result.distance < out.distanceToHit) {
                    out.triangle = triangle;
                    out.distanceToHit = result.distance;
                    out.baryPosition = result.baryPosition;
                    out.found = true;
                    out.rayCount++;
                    return;
                }
            }
            out.rayCount++;
        }
    }*/

    /*
    inline void EvaluateRaycasts(glm::vec3 rayOrigin, glm::vec3 rayDirection, float maxDistance, std::vector<Triangle>& triangles, RigidStaticType parentType, glm::mat4 parentMatrix, RayCastResult& out, void* parent, bool ignoreBackFacing = true) {
        glm::mat4 inverseTransform = glm::inverse(parentMatrix);

        glm::vec3 origin = inverseTransform * glm::vec4(rayOrigin, 1);
        glm::vec3 direction = inverseTransform * glm::vec4(rayDirection, 0);

        for (Triangle& triangle : triangles) {

            // Skip if triangle is facing away from light direction
            float vdotn = dot(rayDirection, triangle.normal);
            if (ignoreBackFacing && vdotn > 0) {
                continue;
            }

            IntersectionResult result = RayTriangleIntersectTest(triangle.p1, triangle.p2, triangle.p3, origin, direction);
            float minDist = 0.01f;
            if (result.found && result.distance > minDist && result.distance < maxDistance) {
                if (result.distance < out.distanceToHit) {
                    out.triangle = triangle;
                    out.distanceToHit = result.distance;
                    out.RigidStaticType = parentType;
                    out.triangeleModelMatrix = parentMatrix;
                    out.baryPosition = result.baryPosition;
                    out.found = true;
                    out.parent = parent;
                }
            }
            out.rayCount++;
        }
    }*/

    inline const std::string GetFilename(const std::string& filepath) {
        std::string result = filepath.substr(filepath.rfind("/") + 1);
        result = result.substr(0, result.length() - 4);
        return result;
    }

    inline FileInfo GetFileInfo(std::string filepath) {
        // isolate name
        std::string filename = filepath.substr(filepath.rfind("/") + 1);
        filename = filename.substr(0, filename.length() - 4);
        // isolate filetype
        std::string filetype = filepath.substr(filepath.length() - 3);
        // isolate directory
        std::string directory = filepath.substr(0, filepath.rfind("/") + 1);
        // material name
        std::string materialType = "NONE";
        if (filename.length() > 5) {
            std::string query = filename.substr(filename.length() - 3);
            if (query == "ALB" || query == "RMA" || query == "NRM")
                materialType = query;
        }
        // RETURN IT
        FileInfo info;
        info.fullpath = filepath;
        info.filename = filename;
        info.filetype = filetype;
        info.directory = directory;
        info.materialType = materialType;
        return info;
    }

    inline FileInfo GetFileInfo(const std::filesystem::directory_entry &filepath)
    {
        const auto &path{ filepath.path() };

        static const auto get_material_type{ [] (std::string_view filename) {
			if (filename.size() > 5) {
				filename.remove_prefix(filename.size() - 3);
				if (filename == "ALB" || filename == "RMA" || filename == "NRM") {
					return std::string{ filename };
				}
			}
            return std::string{ "NONE" };
        }};

		const auto stem{ path.has_stem() ? path.stem().string() : "" };

        std::string filetype = path.has_extension() ? path.extension().string().substr(1) : "";  // remove dot


        return FileInfo{
            path.string(),
            path.parent_path().string(),
            stem,
            Lowercase(filetype),
            get_material_type(stem)
        };
    }

    inline float YRotationBetweenTwoPoints(glm::vec3 a, glm::vec3 b) {
        float delta_x = b.x - a.x;
        float delta_y = b.z - a.z;
        float theta_radians = atan2(delta_y, delta_x);
        return -theta_radians;
    }

    inline glm::vec3 GetTranslationFromMatrix(glm::mat4 matrix) {
        return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    }

	inline void RemoveScaleFromMatrix(glm::mat4& matrix) {
		matrix[0][3] = 1.0f;
		matrix[1][3] = 1.0f;
		matrix[2][3] = 1.0f;
	}


    inline void InterpolateQuaternion(glm::quat& Out, const glm::quat& Start, const glm::quat& End, float pFactor) {
        // calc cosine theta
        float cosom = Start.x * End.x + Start.y * End.y + Start.z * End.z + Start.w * End.w;
        // adjust signs (if necessary)
        glm::quat end = End;
        if (cosom < static_cast<float>(0.0)) {
            cosom = -cosom;
            end.x = -end.x;   // Reverse all signs
            end.y = -end.y;
            end.z = -end.z;
            end.w = -end.w;
        }
        // Calculate coefficients
        float sclp, sclq;
        if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) // 0.0001 -> some epsillon
        {
            // Standard case (slerp)
            float omega, sinom;
            omega = std::acos(cosom); // extract theta from dot product's cos theta
            sinom = std::sin(omega);
            sclp = std::sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
            sclq = std::sin(pFactor * omega) / sinom;
        }
        else {
            // Very close, do linear interp (because it's faster)
            sclp = static_cast<float>(1.0) - pFactor;
            sclq = pFactor;
        }
        Out.x = sclp * Start.x + sclq * end.x;
        Out.y = sclp * Start.y + sclq * end.y;
        Out.z = sclp * Start.z + sclq * end.z;
        Out.w = sclp * Start.w + sclq * end.w;
    }

    inline glm::mat4 Mat4InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ) {
        /*	glm::mat4 m = glm::mat4(1);
            m[0][0] = ScaleX; m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
            m[1][0] = 0.0f;   m[1][1] = ScaleY; m[1][2] = 0.0f;   m[1][3] = 0.0f;
            m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = ScaleZ; m[2][3] = 0.0f;
            m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
            return m;*/

        return glm::scale(glm::mat4(1.0), glm::vec3(ScaleX, ScaleY, ScaleZ));
    }

    inline glm::mat4 Mat4InitRotateTransform(float RotateX, float RotateY, float RotateZ) {
        glm::mat4 rx = glm::mat4(1);
        glm::mat4 ry = glm::mat4(1);
        glm::mat4 rz = glm::mat4(1);

        const float x = ToRadian(RotateX);
        const float y = ToRadian(RotateY);
        const float z = ToRadian(RotateZ);

        rx[0][0] = 1.0f; rx[0][1] = 0.0f; rx[0][2] = 0.0f; rx[0][3] = 0.0f;
        rx[1][0] = 0.0f; rx[1][1] = cosf(x); rx[1][2] = -sinf(x); rx[1][3] = 0.0f;
        rx[2][0] = 0.0f; rx[2][1] = sinf(x); rx[2][2] = cosf(x); rx[2][3] = 0.0f;
        rx[3][0] = 0.0f; rx[3][1] = 0.0f; rx[3][2] = 0.0f; rx[3][3] = 1.0f;

        ry[0][0] = cosf(y); ry[0][1] = 0.0f; ry[0][2] = -sinf(y); ry[0][3] = 0.0f;
        ry[1][0] = 0.0f; ry[1][1] = 1.0f; ry[1][2] = 0.0f; ry[1][3] = 0.0f;
        ry[2][0] = sinf(y); ry[2][1] = 0.0f; ry[2][2] = cosf(y); ry[2][3] = 0.0f;
        ry[3][0] = 0.0f; ry[3][1] = 0.0f; ry[3][2] = 0.0f; ry[3][3] = 1.0f;

        rz[0][0] = cosf(z); rz[0][1] = -sinf(z); rz[0][2] = 0.0f; rz[0][3] = 0.0f;
        rz[1][0] = sinf(z); rz[1][1] = cosf(z); rz[1][2] = 0.0f; rz[1][3] = 0.0f;
        rz[2][0] = 0.0f; rz[2][1] = 0.0f; rz[2][2] = 1.0f; rz[2][3] = 0.0f;
        rz[3][0] = 0.0f; rz[3][1] = 0.0f; rz[3][2] = 0.0f; rz[3][3] = 1.0f;

        return rz * ry * rx;
    }

    inline glm::mat4 Mat4InitTranslationTransform(float x, float y, float z) {
        return  glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    }

    inline bool StrCmp(const char* queryA, const char* queryB) {
        if (strcmp(queryA, queryB) == 0)
            return true;
        else
            return false;
    }

    inline const char* CopyConstChar(const char* text) {
        char* b = new char[strlen(text) + 1] {};
        std::copy(text, text + strlen(text), b);
        return b;
    }

    inline bool LineIntersects(glm::vec2 begin_A, glm::vec2 end_A, glm::vec2 begin_B, glm::vec2 end_B, glm::vec2& result)
    {
        static const auto SameSign = [](float a, float b) -> bool {
            return ((a * b) >= 0);
        };
        // a
        float x1 = begin_A.x;
        float y1 = begin_A.y;
        float x2 = end_A.x;
        float y2 = end_A.y;
        // b
        float x3 = begin_B.x;
        float y3 = begin_B.y;
        float x4 = end_B.x;
        float y4 = end_B.y;
        float a1, a2, b1, b2, c1, c2;
        float r1, r2, r3, r4;
        float denom;
        a1 = y2 - y1;
        b1 = x1 - x2;
        c1 = (x2 * y1) - (x1 * y2);
        r3 = ((a1 * x3) + (b1 * y3) + c1);
        r4 = ((a1 * x4) + (b1 * y4) + c1);
        if ((r3 != 0) && (r4 != 0) && SameSign(r3, r4))
            return false;
        a2 = y4 - y3; // Compute a2, b2, c2
        b2 = x3 - x4;
        c2 = (x4 * y3) - (x3 * y4);
        r1 = (a2 * x1) + (b2 * y1) + c2; // Compute r1 and r2
        r2 = (a2 * x2) + (b2 * y2) + c2;
        if ((r1 != 0) && (r2 != 0) && (SameSign(r1, r2)))
            return false;
        denom = (a1 * b2) - (a2 * b1); //Line segments intersect: compute intersection point.
        if (denom == 0)
            return false;// COLLINEAR;
        // FIND THAT INTERSECTION POINT ALREADY
        {
            // Line AB represented as a1x + b1y = c1
            float a = y2 - y1;
            float b = x1 - x2;
            float c = a * (x1)+b * (y1);
            // Line CD represented as a2x + b2y = c2
            float aa1 = y4 - y3;
            float bb1 = x3 - x4;
            float cc1 = aa1 * (x3)+bb1 * (y3);
            float det = a * bb1 - aa1 * b;
            if (det == 0) {

                return false;
            }
            else {
                float x = (bb1 * c - b * cc1) / det;
                float y = (a * cc1 - aa1 * c) / det;
                result.x = x;
                result.y = y;
                return true;
            }
        }
    }

    inline bool LineIntersects(glm::vec3 begin_A, glm::vec3 end_A, glm::vec3 begin_B, glm::vec3 end_B, glm::vec3& result) {
        // wow this is ugly
        glm::vec2 temp;
        bool i = LineIntersects(glm::vec2(begin_A.x, begin_A.z), glm::vec2(end_A.x, end_A.z), glm::vec2(begin_B.x, begin_B.z), glm::vec2(end_B.x, end_B.z), temp);
        result.x = temp.x;
        result.y = begin_A.y;
        result.z = temp.y;
        return i;
    }

    inline float sign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    inline bool PointIn2DTriangle(glm::vec2 pt, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3) {
        float d1, d2, d3;
        bool has_neg, has_pos;
        d1 = sign(pt, v1, v2);
        d2 = sign(pt, v2, v3);
        d3 = sign(pt, v3, v1);
        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(has_neg && has_pos);
    }
}