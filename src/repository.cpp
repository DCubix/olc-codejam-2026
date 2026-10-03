#include "repository.h"

void FigureRepository::LoadFigure(const std::string &name) {
    auto it = m_figures.find(name);
    if (it != m_figures.end()) {
        return; // already loaded
    }
    Figure fig;
    fig.ParseFile(name);
    m_figures[name] = fig;
}

std::optional<Figure> FigureRepository::GetFigure(const std::string &name) {
    LoadFigure(name);
    auto it = m_figures.find(name);
    if (it != m_figures.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::unique_ptr<FigureRepository> FigureRepository::s_instance = nullptr;
FigureRepository &FigureRepository::Get() {
    if (!s_instance) {
        s_instance = std::make_unique<FigureRepository>();
    }
    return *s_instance;
}

olc::Image *ImageRepository::GetImage(
    const std::string &name,
    const olc::vi2d& origin,
    int tilesX, int tilesY
    )
{
    LoadImage(name);
    auto it = m_images.find(name);
    if (it != m_images.end()) {
        return &it->second;
    }
    return nullptr;
}

Sprite *ImageRepository::GetSprite(const std::string &name)
{
    auto it = m_sprites.find(name);
    if (it != m_sprites.end()) {
        return &it->second;
    }
    return nullptr;
}

std::unique_ptr<ImageRepository> ImageRepository::s_instance = nullptr;

ImageRepository &ImageRepository::Get(olc::PixelGameEngine *pge)
{
    if (!s_instance) {
        s_instance = std::make_unique<ImageRepository>();
    }

    if (pge) {
        s_instance->m_pge = pge;
    }
    return *s_instance;
}

void ImageRepository::LoadImage(
    const std::string &name,
    const olc::vi2d& origin,
    int tilesX, int tilesY
    )
{
    auto it = m_images.find(name);
    if (it != m_images.end()) {
        return; // already loaded
    }
    m_images[name] = olc::Image{};
    m_pge->CreateImageFromFile(m_images[name], name);

    m_sprites[name] = Sprite{ &m_images[name], origin, tilesX, tilesY };
}

void SoundRepository::LoadSound(const std::string &filePath)
{
    auto it = m_sounds.find(filePath);
    if (it != m_sounds.end()) {
        return; // already loaded
    }
    m_sounds[filePath] = ma::Sound{};
    m_audio->CreateSoundFromFile(m_sounds[filePath], filePath, 16);
}

ma::Sound *SoundRepository::GetSound(const std::string &name)
{
    LoadSound(name);
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        return &it->second;
    }
    return nullptr;
}

std::unique_ptr<SoundRepository> SoundRepository::s_instance = nullptr;
SoundRepository &SoundRepository::Get(ma::AudioEngine *audio)
{
    if (!s_instance) {
        s_instance = std::make_unique<SoundRepository>();
    }

    if (audio) {
        s_instance->m_audio = audio;
    }
    return *s_instance;
}
