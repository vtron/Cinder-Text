//
//  CTTextureRenderer.cpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/7/16.
//
//

#include "CTRendererTexture.hpp"

namespace typography {
    RendererTexture::GlyphTextureMap RendererTexture::glyphTextures = [] {
        GlyphTextureMap map;
        return map;
    }();
    
    RendererTextureRef RendererTexture::create() {
        return RendererTextureRef(new RendererTexture());
    }
    
    void RendererTexture::draw() {
        if(mLayout != nullptr) {
            for(auto glyph : mLayout->getAllGlyphs()) {
                ci::gl::pushModelMatrix();
                ci::gl::translate(glyph->position);
                ci::gl::draw(RendererTexture::glyphTextures[glyph->font][glyph->index]);
                
                ci::gl::popModelMatrix();
            }
        }
    }
    
    void RendererTexture::setLayout(LayoutRef layout) {
        Renderer::setLayout(layout);
        
        for(auto layoutGlyph : layout->getAllGlyphs()) {
            generateTexturesForFont(layoutGlyph->font);
        }
    }
    
    void RendererTexture::generateTexturesForFont(FontRef font) {
        if(glyphTextures.count(font) == 0) {
            for(auto glyphIndex : font->getGlyphIndices()) {
                ci::ChannelRef glyphChannel = font->getBitmapChannel(int(glyphIndex));
                ci::SurfaceRef surface = ci::Surface::create(glyphChannel->getWidth(), glyphChannel->getHeight(), true);
                
                Surface::Iter iter = surface->getIter(surface->getBounds());
                while( iter.line() ) {
                    while( iter.pixel() ) {
                        iter.r() = 255;
                        iter.g() = 255;
                        iter.b() = 255;
                        int curIndex = iter.y() * iter.getWidth() + iter.x();
                        iter.a() = glyphChannel->getData()[curIndex];
                    }
                }
                
                RendererTexture::glyphTextures[font][glyphIndex] = ci::gl::Texture::create(*surface);
//                
//                ci::gl::TextureRef tex = ci::gl::Texture::create(glyphChannel->getWidth(), glyphChannel->getHeight());
//                tex->
                
            }
        }
    }
}
