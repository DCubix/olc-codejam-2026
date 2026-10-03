#pragma once

#include "stickfigure.h"

#include "olcPGEX3_Miniaudio.h"

class FigureRepository {
public:
    std::optional<Figure> GetFigure(const std::string& name);

    static FigureRepository& Get();

private:
    std::map<std::string, Figure> m_figures;

    void LoadFigure(const std::string& name);

    static std::unique_ptr<FigureRepository> s_instance;
};

class ImageRepository {
public:
    olc::Image* GetImage(
        const std::string& name,
        const olc::vi2d& origin = olc::vi2d{},
        int tilesX = 1, int tilesY = 1
    );
    Sprite* GetSprite(const std::string& name);
    void LoadImage(
        const std::string& name,
        const olc::vi2d& origin = olc::vi2d{},
        int tilesX = 1, int tilesY = 1
    );

    static ImageRepository& Get(olc::PixelGameEngine* pge = nullptr);
private:
    olc::PixelGameEngine* m_pge{ nullptr };
    std::map<std::string, olc::Image> m_images;
    std::map<std::string, Sprite> m_sprites;

    static std::unique_ptr<ImageRepository> s_instance;
};

namespace ma = olc::ext::Miniaudio;

class SoundRepository {
public:
    void LoadSound(const std::string& filePath);
    ma::Sound* GetSound(const std::string& name);

    ma::AudioEngine& Audio() { return *m_audio; }

    static SoundRepository& Get(ma::AudioEngine* audio = nullptr);
private:
    ma::AudioEngine* m_audio{nullptr};
    std::map<std::string, ma::Sound> m_sounds;

    static std::unique_ptr<SoundRepository> s_instance;
};

#define IRG(x) ImageRepository::Get().GetSprite(x)
#define SRG(x) SoundRepository::Get().GetSound(x)

#define IRL(x) ImageRepository::Get().LoadImage(x)
#define SRL(x) SoundRepository::Get().LoadSound(x)
