#include "MsgEffect.h"

#include "World.h"
#include "DebugLog.h"
#include "main/WinMain.h"
#include "render/Renderer.h"
#include "render3d/Device.h"
#include "res/ActRes.h"
#include "res/Sprite.h"
#include "res/Res.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

constexpr float kMsgEffectNearPlane = 10.0f;
constexpr float kMsgEffectSubmitNearPlane = 80.0f;
constexpr const char* kDamageNumberSpritePath = "data\\sprite\\\xC0\xCC\xC6\xD1\xC6\xAE\\\xBC\xFD\xC0\xDA.spr";
constexpr const char* kDamageNumberActPath = "data\\sprite\\\xC0\xCC\xC6\xD1\xC6\xAE\\\xBC\xFD\xC0\xDA.act";

struct QueuedMsgEffectDraw {
    int screenX = 0;
    int screenY = 0;
    int digit = 0;
    u32 colorArgb = 0xFFFFFFFFu;
    int alpha = 255;
    float zoom = 1.0f;
    int sprShift = 0;
};

std::vector<QueuedMsgEffectDraw> g_queuedMsgEffects;

bool ProjectMsgEffectPoint(const matrix& viewMatrix, const vector3d& point, tlvertex3d* outVertex)
{
    if (!outVertex) {
        return false;
    }

    const float clipZ = point.x * viewMatrix.m[0][2]
        + point.y * viewMatrix.m[1][2]
        + point.z * viewMatrix.m[2][2]
        + viewMatrix.m[3][2];
    if (!std::isfinite(clipZ) || clipZ <= kMsgEffectSubmitNearPlane) {
        return false;
    }

    const float oow = 1.0f / clipZ;
    const float projectedX = point.x * viewMatrix.m[0][0]
        + point.y * viewMatrix.m[1][0]
        + point.z * viewMatrix.m[2][0]
        + viewMatrix.m[3][0];
    const float projectedY = point.x * viewMatrix.m[0][1]
        + point.y * viewMatrix.m[1][1]
        + point.z * viewMatrix.m[2][1]
        + viewMatrix.m[3][1];

    outVertex->x = g_renderer.m_xoffset + projectedX * g_renderer.m_hpc * oow;
    outVertex->y = g_renderer.m_yoffset + projectedY * g_renderer.m_vpc * oow;
    outVertex->z = (1500.0f / (1500.0f - kMsgEffectNearPlane)) * ((1.0f / oow) - kMsgEffectNearPlane) * oow;
    outVertex->oow = oow;
    outVertex->specular = 0xFF000000u;
    return std::isfinite(outVertex->x) && std::isfinite(outVertex->y) && std::isfinite(outVertex->z);
}

CSprRes* GetDamageCountSprite()
{
    static CSprRes* sprite = nullptr;
    static bool attemptedLoad = false;
    if (!attemptedLoad) {
        attemptedLoad = true;
        sprite = g_resMgr.GetAs<CSprRes>(kDamageNumberSpritePath);
        if (!sprite) {
            DbgLog("[MsgEffect] Failed to load damage number sprite '%s'\n", kDamageNumberSpritePath);
        }
    }
    return sprite;
}

CActRes* GetDamageCountAct()
{
    static CActRes* act = nullptr;
    static bool attemptedLoad = false;
    if (!attemptedLoad) {
        attemptedLoad = true;
        act = g_resMgr.GetAs<CActRes>(kDamageNumberActPath);
        if (!act) {
            DbgLog("[MsgEffect] Failed to load damage number act '%s'\n", kDamageNumberActPath);
        }
    }
    return act;
}

float ResolveDigitScale(float zoom)
{
    const float scale = 1.8f + zoom * 0.6f;
    return (std::max)(1.8f, (std::min)(4.8f, scale));
}

int ResolveDigitShiftPixels(const QueuedMsgEffectDraw& draw)
{
    return draw.sprShift;
}

int CountDamageDigits(int value)
{
    if (value >= 100000) {
        return 6;
    }
    if (value >= 10000) {
        return 5;
    }
    if (value >= 1000) {
        return 4;
    }
    if (value >= 100) {
        return 3;
    }
    if (value >= 10) {
        return 2;
    }
    return 1;
}

int ResolveDigitAtIndex(int value, int index)
{
    for (int i = 0; i < index; ++i) {
        value /= 10;
    }
    return value % 10;
}

bool ResolveDigitClipBox(int digit, RECT* outClipBox)
{
    if (!outClipBox) {
        return false;
    }

    CSprRes* sprite = GetDamageCountSprite();
    CActRes* act = GetDamageCountAct();
    if (!sprite || !act) {
        return false;
    }

    const int digitIndex = (std::max)(0, (std::min)(9, digit));
    const CMotion* motion = act->GetMotion(0, digitIndex);
    if (!motion) {
        motion = act->GetMotion(0, 0);
    }
    if (!motion) {
        return false;
    }

    RECT clipBox{};
    bool hasClip = false;
    for (const CSprClip& clip : motion->sprClips) {
        const SprImg* image = sprite->GetSprite(clip.clipType, clip.sprIndex);
        if (!image || image->width <= 0 || image->height <= 0) {
            continue;
        }

        const int drawX = clip.x - image->width / 2;
        const int drawY = clip.y - image->height / 2;
        RECT current = { drawX, drawY, drawX + image->width, drawY + image->height };
        if (!hasClip) {
            clipBox = current;
            hasClip = true;
        } else {
            clipBox.left = (std::min)(clipBox.left, current.left);
            clipBox.top = (std::min)(clipBox.top, current.top);
            clipBox.right = (std::max)(clipBox.right, current.right);
            clipBox.bottom = (std::max)(clipBox.bottom, current.bottom);
        }
    }

    if (!hasClip) {
        return false;
    }

    *outClipBox = clipBox;
    return true;
}

int ResolveDigitOutputWidth(int digit, float zoom)
{
    RECT clipBox{};
    if (!ResolveDigitClipBox(digit, &clipBox)) {
        return (std::max)(1, static_cast<int>(std::lround(8.0f * ResolveDigitScale(zoom))));
    }

    const int nativeWidth = (std::max)(1, static_cast<int>(clipBox.right - clipBox.left));
    return (std::max)(1, static_cast<int>(std::lround(static_cast<float>(nativeWidth) * ResolveDigitScale(zoom))));
}

unsigned int PremultiplyArgb(unsigned int color)
{
    const unsigned int alpha = (color >> 24) & 0xFFu;
    if (alpha == 0u || alpha == 0xFFu) {
        return color;
    }

    const unsigned int red = ((color >> 16) & 0xFFu) * alpha / 255u;
    const unsigned int green = ((color >> 8) & 0xFFu) * alpha / 255u;
    const unsigned int blue = (color & 0xFFu) * alpha / 255u;
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

unsigned int ModulateDigitColor(unsigned int srcRgb, unsigned int alpha, unsigned int tintRed, unsigned int tintGreen, unsigned int tintBlue)
{
    const unsigned int srcRed = (srcRgb >> 16) & 0xFFu;
    const unsigned int srcGreen = (srcRgb >> 8) & 0xFFu;
    const unsigned int srcBlue = srcRgb & 0xFFu;
    const unsigned int maxChannel = (std::max)(srcRed, (std::max)(srcGreen, srcBlue));

    if (maxChannel <= 24u) {
        return PremultiplyArgb(alpha << 24);
    }

    // Treat the digit sprite as a brightness mask so the requested damage tint
    // stays stable through the fade instead of inheriting the source art hue.
    const unsigned int modulatedRed = maxChannel * tintRed / 255u;
    const unsigned int modulatedGreen = maxChannel * tintGreen / 255u;
    const unsigned int modulatedBlue = maxChannel * tintBlue / 255u;
    return PremultiplyArgb((alpha << 24) | (modulatedRed << 16) | (modulatedGreen << 8) | modulatedBlue);
}

unsigned int BuildDigitMaskPixel(unsigned int rgb, unsigned int alpha)
{
    const unsigned int srcRed = (rgb >> 16) & 0xFFu;
    const unsigned int srcGreen = (rgb >> 8) & 0xFFu;
    const unsigned int srcBlue = rgb & 0xFFu;
    const unsigned int maxChannel = (std::max)(srcRed, (std::max)(srcGreen, srcBlue));
    const unsigned int luminance = maxChannel <= 24u ? 0u : maxChannel;
    return (alpha << 24)
        | (luminance << 16)
        | (luminance << 8)
        | luminance;
}

CTexture* GetOrCreateDigitClipTexture(CSprRes* sprite, const SprImg* image)
{
    if (!sprite || !image || image->width <= 0 || image->height <= 0) {
        return nullptr;
    }

    SprImg* mutableImage = const_cast<SprImg*>(image);
    if (mutableImage->tex && mutableImage->tex != &CTexMgr::s_dummy_texture) {
        return mutableImage->tex;
    }

    std::vector<unsigned int> pixels(static_cast<size_t>(image->width) * static_cast<size_t>(image->height), 0u);
    if (!image->rgba.empty()) {
        for (int y = 0; y < image->height; ++y) {
            for (int x = 0; x < image->width; ++x) {
                const unsigned int rgba = image->rgba[static_cast<size_t>(y) * static_cast<size_t>(image->width) + static_cast<size_t>(x)];
                const unsigned int alpha = (rgba >> 24) & 0xFFu;
                if (alpha == 0u) {
                    continue;
                }
                pixels[static_cast<size_t>(y) * static_cast<size_t>(image->width) + static_cast<size_t>(x)] =
                    BuildDigitMaskPixel(rgba & 0x00FFFFFFu, alpha);
            }
        }
    } else if (!image->indices.empty()) {
        for (int y = 0; y < image->height; ++y) {
            for (int x = 0; x < image->width; ++x) {
                const unsigned char paletteIndex = image->indices[static_cast<size_t>(y) * static_cast<size_t>(image->width) + static_cast<size_t>(x)];
                if (paletteIndex == 0) {
                    continue;
                }
                const unsigned int paletteColor = sprite->m_pal[paletteIndex] & 0x00FFFFFFu;
                pixels[static_cast<size_t>(y) * static_cast<size_t>(image->width) + static_cast<size_t>(x)] =
                    BuildDigitMaskPixel(paletteColor, 0xFFu);
            }
        }
    } else {
        return nullptr;
    }

    CTexture* texture = new CTexture();
    if (!texture || !texture->Create(static_cast<unsigned int>(image->width), static_cast<unsigned int>(image->height), PF_A8R8G8B8, false)) {
        delete texture;
        return nullptr;
    }
    texture->Update(0,
        0,
        image->width,
        image->height,
        pixels.data(),
        true,
        image->width * static_cast<int>(sizeof(unsigned int)));
    mutableImage->tex = texture;
    return texture;
}

u32 BuildDigitClipColor(const QueuedMsgEffectDraw& draw, const CSprClip& clip)
{
    const unsigned int tintAlpha = (draw.colorArgb >> 24) & 0xFFu;
    const unsigned int totalAlpha = static_cast<unsigned int>((std::max)(0, (std::min)(255, draw.alpha))) * tintAlpha / 255u;
    const unsigned int tintRed = (draw.colorArgb >> 16) & 0xFFu;
    const unsigned int tintGreen = (draw.colorArgb >> 8) & 0xFFu;
    const unsigned int tintBlue = draw.colorArgb & 0xFFu;
    const unsigned int clipAlpha = static_cast<unsigned int>(clip.a) * totalAlpha / 255u;
    const unsigned int clipRed = tintRed * clip.r / 255u;
    const unsigned int clipGreen = tintGreen * clip.g / 255u;
    const unsigned int clipBlue = tintBlue * clip.b / 255u;
    return (clipAlpha << 24)
        | (clipRed << 16)
        | (clipGreen << 8)
        | clipBlue;
}

bool QueueDigitSpriteQuad(const QueuedMsgEffectDraw& draw)
{
    CSprRes* sprite = GetDamageCountSprite();
    CActRes* act = GetDamageCountAct();
    if (!sprite || !act) {
        return false;
    }

    const int digitIndex = (std::max)(0, (std::min)(9, draw.digit));
    const CMotion* motion = act->GetMotion(0, digitIndex);
    if (!motion) {
        motion = act->GetMotion(0, 0);
    }
    if (!motion) {
        return false;
    }

    RECT clipBox{};
    if (!ResolveDigitClipBox(draw.digit, &clipBox)) {
        return false;
    }

    const float scale = ResolveDigitScale(draw.zoom);
    const int nativeWidth = (std::max)(1, static_cast<int>(clipBox.right - clipBox.left));
    const int nativeHeight = (std::max)(1, static_cast<int>(clipBox.bottom - clipBox.top));
    const int outWidth = (std::max)(1, static_cast<int>(std::lround(static_cast<float>(nativeWidth) * scale)));
    const int outHeight = (std::max)(1, static_cast<int>(std::lround(static_cast<float>(nativeHeight) * scale)));
    const float originX = static_cast<float>(draw.screenX + ResolveDigitShiftPixels(draw) - outWidth / 2) - 0.5f;
    const float originY = static_cast<float>(draw.screenY - outHeight / 2) - 0.5f;

    bool queuedAny = false;
    float clipSortOffset = 0.0f;
    for (const CSprClip& clip : motion->sprClips) {
        const SprImg* image = sprite->GetSprite(clip.clipType, clip.sprIndex);
        if (!image || image->width <= 0 || image->height <= 0) {
            continue;
        }

        CTexture* texture = GetOrCreateDigitClipTexture(sprite, image);
        if (!texture || texture == &CTexMgr::s_dummy_texture) {
            continue;
        }

        const float drawLeft = originX + static_cast<float>(clip.x - image->width / 2 - clipBox.left) * scale;
        const float drawTop = originY + static_cast<float>(clip.y - image->height / 2 - clipBox.top) * scale;
        const float drawRight = drawLeft + static_cast<float>(image->width) * scale;
        const float drawBottom = drawTop + static_cast<float>(image->height) * scale;

        const float maxU = texture->m_w != 0
            ? static_cast<float>(texture->m_surfaceUpdateWidth > 0 ? texture->m_surfaceUpdateWidth : static_cast<unsigned int>(image->width))
                / static_cast<float>(texture->m_w)
            : 1.0f;
        const float maxV = texture->m_h != 0
            ? static_cast<float>(texture->m_surfaceUpdateHeight > 0 ? texture->m_surfaceUpdateHeight : static_cast<unsigned int>(image->height))
                / static_cast<float>(texture->m_h)
            : 1.0f;

        RPFace* face = g_renderer.BorrowNullRP();
        if (!face) {
            continue;
        }

        const bool flipX = (clip.flags & 1) != 0;
        const u32 color = BuildDigitClipColor(draw, clip);
        if (((color >> 24) & 0xFFu) == 0u) {
            continue;
        }

        face->primType = D3DPT_TRIANGLESTRIP;
        face->verts = face->m_verts;
        face->numVerts = 4;
        face->indices = nullptr;
        face->numIndices = 0;
        face->tex = texture;
        face->mtPreset = 0;
        face->cullMode = D3DCULL_NONE;
        face->srcAlphaMode = D3DBLEND_SRCALPHA;
        face->destAlphaMode = D3DBLEND_INVSRCALPHA;
        face->alphaSortKey = 1.7f + clipSortOffset;

        face->m_verts[0] = { drawLeft,  drawTop,    0.0f, 1.0f, color, 0xFF000000u, flipX ? maxU : 0.0f, 0.0f };
        face->m_verts[1] = { drawRight, drawTop,    0.0f, 1.0f, color, 0xFF000000u, flipX ? 0.0f : maxU, 0.0f };
        face->m_verts[2] = { drawLeft,  drawBottom, 0.0f, 1.0f, color, 0xFF000000u, flipX ? maxU : 0.0f, maxV };
        face->m_verts[3] = { drawRight, drawBottom, 0.0f, 1.0f, color, 0xFF000000u, flipX ? 0.0f : maxU, maxV };
        g_renderer.AddRP(face, 1 | 8);
        clipSortOffset += 0.0001f;
        queuedAny = true;
    }

    return queuedAny;
}

bool GetDigitDrawBounds(const QueuedMsgEffectDraw& draw, RECT* outRect)
{
    if (!outRect) {
        return false;
    }

    RECT clipBox{};
    if (ResolveDigitClipBox(draw.digit, &clipBox)) {
        const float scale = ResolveDigitScale(draw.zoom);
        const int nativeWidth = (std::max)(1, static_cast<int>(clipBox.right - clipBox.left));
        const int nativeHeight = (std::max)(1, static_cast<int>(clipBox.bottom - clipBox.top));
        const int outWidth = (std::max)(1, static_cast<int>(std::lround(static_cast<float>(nativeWidth) * scale)));
        const int outHeight = (std::max)(1, static_cast<int>(std::lround(static_cast<float>(nativeHeight) * scale)));
        const int originX = draw.screenX + ResolveDigitShiftPixels(draw) - outWidth / 2;
        const int originY = draw.screenY - outHeight / 2;
        outRect->left = originX - 1;
        outRect->top = originY - 1;
        outRect->right = originX + outWidth + 1;
        outRect->bottom = originY + outHeight + 1;
        return true;
    }

    const int fontHeight = (std::max)(14, (std::min)(32, static_cast<int>(std::lround(8.0f + draw.zoom * 4.0f))));
    const int drawX = draw.screenX + ResolveDigitShiftPixels(draw) - fontHeight / 4;
    const int drawY = draw.screenY - fontHeight / 2;
    outRect->left = drawX - 2;
    outRect->top = drawY - 2;
    outRect->right = drawX + fontHeight + 2;
    outRect->bottom = drawY + fontHeight + 2;
    return true;
}

void ResolveLateralOffset(float rotationDegrees, float* outX, float* outZ)
{
    if (!outX || !outZ) {
        return;
    }

    *outX = 0.0f;
    *outZ = 0.0f;

    int dir = 0;
    int normalized = static_cast<int>(rotationDegrees);
    while (normalized < 0) {
        normalized += 360;
    }
    while (normalized >= 360) {
        normalized -= 360;
    }

    if (normalized < 90) {
        dir = 0;
    } else if (normalized < 180) {
        dir = 2;
    } else if (normalized < 270) {
        dir = 4;
    } else {
        dir = 6;
    }

    switch (dir) {
    case 0:
        *outX = -0.8f;
        *outZ = 0.8f;
        break;
    case 2:
        *outX = -0.8f;
        *outZ = -0.8f;
        break;
    case 4:
        *outX = 0.8f;
        *outZ = -0.8f;
        break;
    default:
        *outX = 0.8f;
        *outZ = 0.8f;
        break;
    }
}

} // namespace

CMsgEffect::CMsgEffect()
    : m_msgEffectType(0)
    , m_digit(0)
    , m_numberValue(0)
    , m_sprShift(0)
    , m_alpha(255)
    , m_masterGid(0)
    , m_colorArgb(0xFFFFFFFFu)
    , m_stateStartTick(timeGetTime())
    , m_pos{}
    , m_orgPos{}
    , m_destPos{}
    , m_destPos2{}
    , m_zoom(1.0f)
    , m_orgZoom(1.0f)
    , m_masterActor(nullptr)
    , m_isVisible(0)
    , m_isDisappear(0)
    , m_removedFromOwner(0)
{
}

CMsgEffect::~CMsgEffect()
{
    if (!m_removedFromOwner && m_masterActor) {
        m_masterActor->DeleteMatchingEffect(this);
    }
}

u8 CMsgEffect::OnProcess()
{
    if (m_isDisappear) {
        if (!m_removedFromOwner && m_masterActor) {
            m_masterActor->DeleteMatchingEffect(this);
            m_removedFromOwner = 1;
        }
        return 0;
    }

    const float stateCount = static_cast<float>(timeGetTime() - m_stateStartTick) * 0.041666668f;
    switch (m_msgEffectType) {
    case 14:
    case 21: {
        if (m_destPos.x == 0.0f && m_masterActor) {
            ResolveLateralOffset(m_masterActor->m_roty, &m_destPos.x, &m_destPos.z);
        }

        m_pos.x = m_orgPos.x + (stateCount * 0.33333334f + 3.0f) * m_destPos.x + m_destPos2.x;
        m_pos.z = m_orgPos.z + (stateCount * 0.33333334f + 3.0f) * m_destPos.z + m_destPos2.z;
        m_pos.y = m_orgPos.y + 8.0f - (2.0f - stateCount * 0.033333335f) * stateCount + m_destPos.y;
        m_zoom = stateCount >= 0.0f ? (std::max)(1.2f, m_orgZoom - stateCount * 0.23999999f) : m_orgZoom;
        m_alpha = (std::max)(0, static_cast<int>(250.0f - stateCount * 3.4f));
        if (stateCount >= 70.0f) {
            m_isDisappear = 1;
        }
        break;
    }
    case 16: {
        if (m_destPos.x == 0.0f && m_masterActor) {
            ResolveLateralOffset(m_masterActor->m_roty, &m_destPos.x, &m_destPos.z);
            m_destPos.x *= 0.625f;
            m_destPos.z *= 0.625f;
        }

        m_pos.x = m_orgPos.x + (stateCount * 0.33333334f + 3.0f) * m_destPos.x + m_destPos2.x;
        m_pos.z = m_orgPos.z + (stateCount * 0.33333334f + 3.0f) * m_destPos.z + m_destPos2.z;
        m_pos.y = m_orgPos.y + 8.0f - (2.0f - stateCount * 0.033333335f) * stateCount + m_destPos.y;
        m_zoom = stateCount >= 0.0f ? (std::max)(1.3f, m_orgZoom - stateCount * 0.23999999f) : m_orgZoom;
        m_alpha = (std::max)(0, static_cast<int>(250.0f - stateCount * 3.4f));
        if (stateCount >= 70.0f) {
            m_isDisappear = 1;
        }
        break;
    }
    case 22:
        m_pos.y = m_orgPos.y - stateCount * 0.1f;
        if (m_zoom < 3.0f) {
            m_zoom += stateCount * 0.18f;
            if (m_zoom > 3.0f) {
                m_zoom = 3.0f;
            }
        }
        m_alpha = stateCount > 90.0f
            ? (std::max)(0, 250 - static_cast<int>((stateCount - 90.0f) * 8.0f))
            : 250;
        if (stateCount >= 120.0f) {
            m_isDisappear = 1;
        }
        break;
    case 114:
    default:
        if (m_masterActor) {
            m_pos.x = m_masterActor->m_pos.x;
            m_pos.z = m_masterActor->m_pos.z;
        }
        m_pos.y = m_orgPos.y - stateCount * 0.54f;
        m_zoom = 1.0f;
        m_alpha = (std::max)(0, static_cast<int>(250.0f - stateCount * 3.0f));
        if (stateCount >= 80.0f) {
            m_isDisappear = 1;
        }
        break;
    }

    if (m_isDisappear && !m_removedFromOwner && m_masterActor) {
        m_masterActor->DeleteMatchingEffect(this);
        m_removedFromOwner = 1;
    }
    return m_isDisappear ? 0 : 1;
}

void CMsgEffect::SendMsg(CGameObject* sender, int msg, msgparam_t par1, msgparam_t par2, msgparam_t par3)
{
    switch (msg) {
    case 22:
        m_masterGid = static_cast<u32>(par1);
        m_msgEffectType = static_cast<int>(par2);
        return;
    case 49:
        m_masterActor = nullptr;
        return;
    case 50:
        m_masterActor = dynamic_cast<CGameActor*>(sender);
        if (m_masterActor && m_msgEffectType == 114) {
            m_pos.x = m_masterActor->m_pos.x;
            m_pos.z = m_masterActor->m_pos.z;
        }
        return;
    case 53:
        m_isDisappear = 1;
        return;
    case 64: {
        const vector3d* pos = reinterpret_cast<const vector3d*>(par1);
        if (!pos) {
            m_isDisappear = 1;
            return;
        }

        m_isVisible = 1;
        m_stateStartTick = timeGetTime();
        m_numberValue = (std::max)(0, static_cast<int>(par2));
        m_digit = (std::max)(0, (std::min)(9, m_numberValue % 10));
        m_sprShift = static_cast<int>(par3);
        m_pos = *pos;
        m_orgPos = *pos;

        if (m_msgEffectType == 114) {
            m_pos.y -= 12.0f;
            m_orgPos.y = m_pos.y;
            m_zoom = 5.0f;
            m_orgZoom = 5.0f;
        } else {
            m_pos.y -= 15.0f;
            m_orgPos.y = m_pos.y;
            m_zoom = 5.0f;
            m_orgZoom = 5.0f;
        }
        return;
    }
    default:
        return;
    }
}

void CMsgEffect::Render(matrix* viewMatrix)
{
    if (!m_isVisible || !viewMatrix) {
        return;
    }

    tlvertex3d projected{};
    if (!ProjectMsgEffectPoint(*viewMatrix, m_pos, &projected)) {
        return;
    }

    const int digitCount = CountDamageDigits(m_numberValue);
    int digits[6] = {};
    int digitWidths[6] = {};
    int totalWidth = 0;
    const int digitGap = -2;

    for (int index = 0; index < digitCount; ++index) {
        digits[index] = ResolveDigitAtIndex(m_numberValue, digitCount - 1 - index);
        digitWidths[index] = ResolveDigitOutputWidth(digits[index], m_zoom);
        totalWidth += digitWidths[index];
    }
    if (digitCount > 1) {
        totalWidth += digitGap * (digitCount - 1);
    }

    int currentLeft = -totalWidth / 2;
    for (int index = 0; index < digitCount; ++index) {
        QueuedMsgEffectDraw draw{};
        draw.screenX = static_cast<int>(std::lround(projected.x));
        draw.screenY = static_cast<int>(std::lround(projected.y));
        draw.digit = digits[index];
        draw.colorArgb = m_colorArgb;
        draw.alpha = m_alpha;
        draw.zoom = m_zoom;
        draw.sprShift = currentLeft + digitWidths[index] / 2;
        g_queuedMsgEffects.push_back(draw);
        currentLeft += digitWidths[index] + digitGap;
    }
}

bool QueueQueuedMsgEffectsQuads()
{
    bool queuedAny = false;
    for (const QueuedMsgEffectDraw& draw : g_queuedMsgEffects) {
        queuedAny = QueueDigitSpriteQuad(draw) || queuedAny;
    }
    g_queuedMsgEffects.clear();
    return queuedAny;
}

bool GetQueuedMsgEffectsBounds(RECT* outRect)
{
    if (!outRect) {
        return false;
    }

    SetRectEmpty(outRect);
    bool hasBounds = false;
    for (const QueuedMsgEffectDraw& draw : g_queuedMsgEffects) {
        RECT digitRect{};
        if (!GetDigitDrawBounds(draw, &digitRect)) {
            continue;
        }
        if (!hasBounds) {
            *outRect = digitRect;
            hasBounds = true;
        } else {
            RECT combined{};
            UnionRect(&combined, outRect, &digitRect);
            *outRect = combined;
        }
    }

    if (hasBounds) {
        InflateRect(outRect, 4, 4);
    }
    return hasBounds;
}

void ClearQueuedMsgEffects()
{
    g_queuedMsgEffects.clear();
}

bool HasQueuedMsgEffects()
{
    return !g_queuedMsgEffects.empty();
}

bool HasActiveMsgEffects()
{
    for (CGameObject* object : g_world.m_gameObjectList) {
        CMsgEffect* effect = dynamic_cast<CMsgEffect*>(object);
        if (!effect) {
            continue;
        }
        if (effect->m_isVisible && !effect->m_isDisappear) {
            return true;
        }
    }
    return false;
}