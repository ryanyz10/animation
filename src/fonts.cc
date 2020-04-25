// #include "fonts.h"

// Font::Font()
// {
// }

// Font::~Font()
// {
// }

// void Font::loadFont(const char *path)
// {
//     if (FT_Init_FreeType(&ft))
//     {
//         std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//         return;
//     }

//     if (FT_New_Face(ft, path, 0, &face))
//     {
//         std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
//         return;
//     }

//     std::cout << "loaded font from " << path << std::endl;
// }

// void Font::generateChar(char c)
// {
//     if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//     {
//         std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//         return;
//     }

//     // Generate texture
//     GLuint texture;
//     glGenTextures(1, &texture);
//     glBindTexture(GL_TEXTURE_2D, texture);
//     glTexImage2D(
//         GL_TEXTURE_2D,
//         0,
//         GL_RED,
//         face->glyph->bitmap.width,
//         face->glyph->bitmap.rows,
//         0,
//         GL_RED,
//         GL_UNSIGNED_BYTE,
//         face->glyph->bitmap.buffer);

//     // Set texture options
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//     // Now store character for later use
//     CharTexture charTex = {
//         texture,
//         glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//         glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//         face->glyph->advance.x};

//     characters.insert(std::pair<char, CharTexture>(c, charTex));
// }

// void Font::unload()
// {
//     FT_Done_Face(face);
//     FT_Done_FreeType(ft);
// }