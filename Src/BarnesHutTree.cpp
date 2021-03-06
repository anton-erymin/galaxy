#include "BarnesHutTree.h"

#include "Galaxy.h"
#include "Math.h"

static constexpr uint32_t cMaxTreeLevel = 50;

BarnesHutTree::BarnesHutTree(const float3 &point, float length)
    : point(point),
    length(length),
    isLeaf(true),
    particle_(nullptr),
    totalMass(0.0f)
{
    oppositePoint = point + float3{ length };
}

void BarnesHutTree::Reset()
{
    isLeaf = true;
    particle_ = nullptr;
}

void BarnesHutTree::Insert(const Particle &p, uint32_t level)
{
    if (!Contains(p))
    {
        return;
    }

    if (level > cMaxTreeLevel)
    {
        return;
    }

    if (isLeaf)
    {
        // Если узел - лист
        if (!particle_)
        {
            // И пустой, то вставляем в него частицу
            particle_ = &p;
            return;
        }
        else
        {
            // Если лист непустой он становится внутренним узлом			
            isLeaf = false;

            if (!children[0])
            {
                // Размеры потомков в половину меньше
                float nl = 0.5f * length;

                float x = point.m_x + nl;
                float y = point.m_y + nl;

                float3 np1(x, point.m_y, 0.0f);
                float3 np2(x, y, 0.0f);
                float3 np3(point.m_x, y, 0.0f);

                children[0] = std::make_unique<BarnesHutTree>(point, nl);
                children[1] = std::make_unique<BarnesHutTree>(np1, nl);
                children[2] = std::make_unique<BarnesHutTree>(np2, nl);
                children[3] = std::make_unique<BarnesHutTree>(np3, nl);
            }
            else
            {
                // Иначе сбрасываем их
                children[0]->Reset();
                children[1]->Reset();
                children[2]->Reset();
                children[3]->Reset();
            }

            // Далее вставляем в нужный потомок частицу которая была в текущем узле
            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(*particle_))
                {
                    children[i]->Insert(*particle_, level + 1);
                    break;
                }
            }

            // И новую частицу
            for (int i = 0; i < 4; i++)
            {
                if (children[i]->Contains(p))
                {
                    children[i]->Insert(p, level + 1);
                    break;
                }
            }

            // Суммарная масса узла
            totalMass = particle_->mass + p.mass;

            // Центр тяжести
            massCenter = p.position.scaleR(p.mass);
            massCenter.addScaled(particle_->position, particle_->mass);
            massCenter *= 1.0f / totalMass;
        }
    }
    else
    {
        // Если это внутренний узел

        // Обновляем суммарную массу добавлением к ней массы новой частицы
        float total = totalMass + p.mass;

        // Также обновляем центр масс
        massCenter *= totalMass;
        massCenter.addScaled(p.position, p.mass);
        massCenter *= 1.0f / total;
        totalMass = total;

        // Рекурсивно вставляем в нужный потомок частицу
        for (int i = 0; i < 4; i++)
        {
            if (children[i]->Contains(p))
            {
                children[i]->Insert(p, level + 1);
                break;
            }
        }
    }
}

bool BarnesHutTree::Contains(const Particle &p) const
{
    float3 v = p.position;
    if (v.m_x >= point.m_x && v.m_x <= oppositePoint.m_x &&
        v.m_y >= point.m_y && v.m_y <= oppositePoint.m_y)
        return true;

    return false;
}

float3 BarnesHutTree::ComputeAcceleration(const Particle &particle, float softFactor) const
{
    float3 acceleration = {};

    if (isLeaf && particle_)
    {
        if (particle_ != &particle)
        {
            acceleration = GravityAcceleration(particle_->position - particle.position, particle_->mass, softFactor);
        }
    }
    else if (!isLeaf)
    {
        // Если это внутренний узел

        // Находим расстояние от частицы до центра масс этого узла
        float3 vec = massCenter - particle.position;
        float r = vec.norm();

        // Находим соотношение размера узла к расстоянию
        float theta = length / r;

        if (theta < 0.7f)
        {
            acceleration = GravityAcceleration(vec, totalMass, softFactor, r);
        }
        else
        {
            // Если частица близко к узлу рекурсивно считаем силу с потомками
            for (int i = 0; i < 4; i++)
            {
                acceleration += children[i]->ComputeAcceleration(particle, softFactor);
            }
        }
    }

    return acceleration;
}
