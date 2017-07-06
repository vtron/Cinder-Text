//
//  CTTextureRenderer.hpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/7/16.
//
//

#pragma once

#include "cinder/gl/gl.h"
#include "CTRenderer.hpp"
#include <map>

namespace typography
{
	typedef std::shared_ptr<class RendererTexture> RendererTextureRef;

	class RendererTexture : public Renderer
	{
		public:
			using GlyphTextureMap = std::map<FontRef, std::map<unsigned long, ci::gl::TextureRef> >;

			static RendererTextureRef create();

			void draw() override;
			void setLayout( LayoutRef mLayout ) override;

		private:
			RendererTexture() {};

			void generateTexturesForFont( FontRef font );

			static GlyphTextureMap glyphTextures;
	};
}
