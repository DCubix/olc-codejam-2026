#include "logic.h"

void Entity::Update(olc::PixelGameEngine* pge, float fElapsedTime)
{
    if (m_destroyed) return;

    if (!m_initialized) {
        OnCreate(pge);
        m_initialized = true;
    }

    OnUpdate(pge, fElapsedTime);

    if (m_destroyTimeout > -1.0f) {
        m_destroyTimeout -= fElapsedTime;
        if (m_destroyTimeout <= 0.0f) {
            m_destroyed = true;
        }
    }

    if (m_destroyed) OnDestroy(pge);
}

void Entity::Destroy(float timeout)
{
    m_destroyTimeout = timeout;
}

int NextTypeId() { static int id = 0; return id++; }

void HUDElement::Update(olc::PixelGameEngine *pge, float fElapsedTime)
{
    if (!m_visible) return;
    OnUpdate(pge, fElapsedTime);
}

void HUDElement::Show()
{
    if (!m_visible) {
        m_visible = true;
        OnShow();
    }
}

void HUDElement::Hide()
{
    if (m_visible) {
        m_visible = false;
        OnHide();
    }
}
