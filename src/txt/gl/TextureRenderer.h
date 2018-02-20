#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"

#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"

namespace txt
{
	namespace gl
	{
		class TextureRenderer
			: public txt::Renderer
		{
			public:
				TextureRenderer();

				//void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) override;
				//void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) override;
				void draw() override;
				void setLayout( const txt::Layout& layout ) override;

				static void loadFont( const Font& font );
				static void unloadFont( const Font& font );

			private:
				// Texture (FBO) caching
				void renderToFbo();
				void allocateFbo( int size );
				ci::gl::FboRef mFbo;

				ci::gl::BatchRef mBatch;

				// Font + Glyph Caching (shared between all instances)
				typedef struct {
					ci::gl::Texture3dRef texArray;
					unsigned int layer;
					ci::vec2 subTexSize;
				} GlyphCache;

				typedef struct {
					std::map<uint32_t, GlyphCache > glyphs;
				} FontCache;

				FontCache& getCacheForFont( const Font& font );
				static void cacheFont( const Font& font );
				static void uncacheFont( const Font& font );

				static std::unordered_map<Font, FontCache> mFontCaches;
		};
	}
}