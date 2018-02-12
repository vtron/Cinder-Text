#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"

#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"

namespace txt
{
	class RendererGl;
	typedef std::shared_ptr<RendererGl> RendererGlRef;

	class RendererGl
		: public txt::Renderer
	{
		public:
			static RendererGlRef instance()
			{
				static RendererGlRef ref( new RendererGl() );
				return ref;
			}

			void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) override;
			void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) override;
			void draw( const Layout& layout ) override;

			void preloadFont( const Font& font ) override;

		private:
			RendererGl();
			void drawGlyph( const Font& font, unsigned int glyphIndex ) {};

			//ci::gl::TextureRef getGlyphTexture( const Font& font, unsigned int glyphIndex );

			//void cacheGlyphAsTexture( const Font& font, uint32_t glyphIndex );
			//std::unordered_map < Font, std::unordered_map<uint32_t, ci::gl::TextureRef > > mGlyphTextures;
			typedef struct {
				ci::gl::Texture3dRef texArray;
				unsigned int layer;
				ci::vec2 subTexSize;
			} GlyphCache;

			typedef struct {
				std::map<uint32_t, GlyphCache > glyphs;
			} FontCache;

			FontCache& getFontCache( const Font& font );
			void cacheFont( const Font& font );

			std::unordered_map<Font, FontCache> mFontCaches;

			ci::gl::BatchRef mBatch;
	};
}