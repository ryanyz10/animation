// #ifndef FONTS_H
// #define FONTS_H

// #include <GL/glew.h>
// #include <glm/glm.hpp>

// #include <iostream>
// #include <string>
// #include <map>

// #include <ft2build.h>
// #include FT_FREETYPE_H

// struct CharTexture
// {
//     GLuint textureID;   // ID handle of the glyph texture
//     glm::ivec2 size;    // Size of glyph
//     glm::ivec2 bearing; // Offset from baseline to left/top of glyph
//     GLuint advance;     // Offset to advance to next glyph
// };

// struct Font
// {
//     Font();
//     ~Font();

//     void loadFont(const char *path);
//     void generateChar(char c);
//     void unload();

// private:
//     FT_Library ft;
//     FT_Face face;
//     std::map<char, CharTexture> characters;
// };

// #endif