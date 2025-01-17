#ifndef TEXTURE_TO_RENDER_H
#define TEXTURE_TO_RENDER_H

class TextureToRender
{
public:
	TextureToRender();
	~TextureToRender();
	void create(int width, int height, bool multisample = false, int samples = 0);
	void bind();
	void unbind();
	int getTexture() const { return tex_; }
	int getFrameBuffer() const { return fb_; }

private:
	int w_, h_;
	unsigned int fb_ = -1;
	unsigned int tex_ = -1;
	unsigned int dep_ = -1;
};

#endif
