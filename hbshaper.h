// This entire file was based upon https://github.com/tangrams/harfbuzz-example/blob/master/src/hbshaper.h
// I downloaded and modified it under the MIT license
#pragma once

#include "hb.h"
#include "hb-ft.h"
#include "freetypelib.h"
//#include "glutils.h"
#include "GL.hpp"
#include <cmath>
#include <vector>
#include <limits>
#include "game4_util.h"
#include <glm/glm.hpp>
#include "gl_errors.hpp"


using namespace std;

struct HBText{
    std::string data;
    std::string language;
    hb_script_t script;
    hb_direction_t direction;
    const char* c_data() { return data.c_str(); };
};
typedef struct HBText HBText;

namespace HBFeature {
    const hb_tag_t KernTag = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
    const hb_tag_t LigaTag = HB_TAG('l', 'i', 'g', 'a'); // standard ligature substitution
    const hb_tag_t CligTag = HB_TAG('c', 'l', 'i', 'g'); // contextual ligature substitution

    static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };
}

class HBShaper {
private:
    FreeTypeLib* lib;
    FT_Face* face;

    hb_font_t* font;
    hb_buffer_t* buffer;
    vector<hb_feature_t> features;

public:

    HBShaper::HBShaper(const string& fontFile, FreeTypeLib* fontLib, int textSize) {
        lib = fontLib;
        face = lib->loadFace(fontFile, textSize * 64, 72, 72);

    }

    void HBShaper::addFeature(hb_feature_t feature) {
        features.push_back(feature);
    }

    std::vector<VMesh*> HBShaper::drawText(HBText& text, float x, float y, glm::u8vec4 const& color, glm::vec2& botright) {
        vector<VMesh*> meshes;

        hb_buffer_reset(buffer);

        hb_buffer_set_direction(buffer, text.direction);
        hb_buffer_set_script(buffer, text.script);
        hb_buffer_set_language(buffer, hb_language_from_string(text.language.c_str(), (int)text.language.size()));
        int length = (int)text.data.size();

        hb_buffer_add_utf8(buffer, text.c_data(), length, 0, length);

        // harfbuzz shaping
        hb_shape(font, buffer, features.empty() ? NULL : &features[0], (unsigned int)features.size());

        unsigned int glyphCount;
        hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
        hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned int i = 0; i < glyphCount; ++i) {
            Glyph* glyph = lib->rasterize(face, glyphInfo[i].codepoint);

            int twidth = (int)pow(2, ceil(log(glyph->width) / log(2)));
            int theight = (int)pow(2, ceil(log(glyph->height) / log(2)));

            auto tdata = new unsigned char[twidth * theight]();

            for (unsigned int iy = 0; iy < glyph->height; ++iy) {
                memcpy(tdata + iy * twidth, glyph->buffer + iy * glyph->width, glyph->width);
            }

            float s0 = 0.0;
            float t0 = 0.0;
            float s1 = (float)glyph->width / twidth;
            float t1 = (float)glyph->height / theight;
            float xa = (float)glyphPos[i].x_advance / 64;
            float ya = (float)glyphPos[i].y_advance / 64;
            float xo = (float)glyphPos[i].x_offset / 64;
            float yo = (float)glyphPos[i].y_offset / 64;
            float x0 = x + xo + glyph->bearing_x;
            float y0 = floor(y + yo + glyph->bearing_y);
            float x1 = x0 + glyph->width;
            float y1 = floor(y0 - glyph->height);

            if (x1 > botright[0]) botright[0] = x1;
            if (y0 < botright[1]) botright[1] = y0;

            VMesh* m = new VMesh;

            //some openGL code from this reference: https://learnopengl.com/In-Practice/Text-Rendering

            m->vertices.emplace_back(Vertex(glm::vec3(x0, y1, 0.0f), color, glm::vec2(0.0f, 1.0f)));
            m->vertices.emplace_back(Vertex(glm::vec3(x1, y1, 0.0f), color, glm::vec2(1.0f, 1.0f)));
            m->vertices.emplace_back(Vertex(glm::vec3(x0, y0, 0.0f), color, glm::vec2(0.0f, 0.0f)));

            m->vertices.emplace_back(Vertex(glm::vec3(x0, y0, 0.0f), color, glm::vec2(0.0f, 0.0f)));
            m->vertices.emplace_back(Vertex(glm::vec3(x1, y1, 0.0f), color, glm::vec2(1.0f, 1.0f)));
            m->vertices.emplace_back(Vertex(glm::vec3(x1, y0, 0.0f), color, glm::vec2(1.0f, 0.0f)));

            GLuint textureId;

            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph->width, glyph->height, 0, GL_RED, GL_UNSIGNED_BYTE, glyph->buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
            m->textureId = textureId;

            meshes.push_back(m);

            x += xa;
            y += ya;

            lib->freeGlyph(glyph);
        }

        return meshes;
    }
    
    void HBShaper::deleteMeshes(std::vector<VMesh*>& meshes) {
        while (!meshes.empty()) {
            VMesh* mesh = meshes.back();
            glDeleteTextures(1, &mesh->textureId);
            delete mesh;
            meshes.pop_back();
        }
    }

    void HBShaper::init() {
        buffer = hb_buffer_create();
        font = hb_ft_font_create(*face, NULL);

        hb_buffer_allocation_successful(buffer);
    }

    HBShaper::~HBShaper() {
        lib->freeFace(face);

        hb_buffer_destroy(buffer);
        hb_font_destroy(font);
    }
};