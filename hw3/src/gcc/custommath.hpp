#include <cmath>
#include <cfloat>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtc/epsilon.hpp>

#define EPSILON 0.00001f
#ifndef M_PI_2f32
#define M_PI_2f32 1.5707963267948966f
#endif
#ifndef M_PIf32
#define M_PIf32 3.14159265358979323f
#endif

glm::vec3 getButterflyCurve(const float &theta)
{
    // Butterfly Curve as described in https://en.wikipedia.org/wiki/Butterfly_curve_(transcendental)
    glm::vec3 pos;
    float t = glm::radians(theta);
    float sin_t = std::sin(t), sin_t_over_12 = std::sin(t / 12.0f);
    float cos_t = std::cos(t), cos_4t = std::cos(4 * t);
    float term = std::exp(cos_t) - 2 * cos_4t - std::pow(sin_t_over_12, 5);
    pos.x = 10.0f * sin_t * term;
    pos.y = 10.0f * cos_t * term;
    pos.z = 0.0f;

    return pos;
}

glm::vec3 getCircumcenter(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    // returns the circumcenter of triangle abc
    // if a,b,c are colinear, return (FLT_MAX, FLT_MAX, FLT_MAX)
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;

    if (glm::areCollinear(ab, ac, EPSILON))
    {
        return glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    }

    glm::vec3 abXac = glm::cross(ab, ac);
    return a + (glm::cross(abXac, ab) * glm::length2(ac) + glm::cross(ac, abXac) * glm::length2(ab)) / (2.f * glm::length2(abXac));
}

glm::vec2 getSteeringAngle(const glm::vec3 &circumcenter)
{
    // Calculate steering angle of two wheels
    // as described in http://datagenetics.com/blog/december12016/index.html
    const float L = 7.8f;
    const float T_half = 4.5f;
    float R = glm::distance(circumcenter, glm::vec3(3.9f, 0.0f, 0.0f));
    float left = std::atan(L / (R - T_half));
    float right = std::atan(L / (R + T_half));
    return glm::vec2(left, right);
}

int getOrientation(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    /*
    return -1 if the x-z projections of a, b, c are counter-clockwise oriented
           0 if a, b, c are colinear
           1 if the x-z projections of a, b, c are clockwise oriented
    */
    if (glm::areCollinear(b - a, c - a, EPSILON))
    {
        return 0;
    }
    return ((b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y) < 0) ? -1 : 1;
}

GLfloat normalizeAngle(const GLfloat angle)
{
    GLfloat return_angle = angle;
    while (return_angle < 0.0f)
    {
        return_angle += 2 * M_PIf32;
    }
    while (return_angle > 2 * M_PIf32)
    {
        return_angle -= 2 * M_PIf32;
    }
    if (glm::epsilonEqual(return_angle, 2 * M_PIf32, EPSILON) || glm::epsilonEqual(return_angle, 0.0f, EPSILON))
    {
        return_angle = 0.0f;
    }
    return return_angle;
}
