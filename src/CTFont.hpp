//
//  CTFont.hpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 9/23/16.
//
//

#pragma once

#include "freetype2/ft2build.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "harfbuzz/hb.h"
#include "harfbuzz/hb-ft.h"

#include "cinder/DataSource.h"

using namespace cinder;

namespace typography {
    class Font;
    typedef std::shared_ptr<Font> FontRef;
    
    class Font {
    public:
        static FontRef create(ci::DataSourceRef dataSource, float size);
        ~Font();
        
        float getSize() { return mSize; };
        FT_Face &getFace() { return mFace; };
        hb_font_t *getHarfbuzzFont() { return mHbFont; }
        
        ci::ChannelRef getCharacterBitmapChannel(char character);
        ci::ChannelRef getBitmapChannel(FT_UInt glyphIndex);
        
        FT_Glyph_Metrics getMetrics(FT_UInt glyphIndex);
        
        const std::vector<FT_ULong> &getGlyphIndices() { return mGlyphIndices; };
        
    private:
        Font(ci::DataSourceRef dataSource, float size);
        
        bool loadGlyphAtIndex(FT_UInt index);
        
        float mSize;
        FT_Face mFace;
        hb_font_t *mHbFont;
        
        // Map of glyphs
        std::vector<FT_ULong> mGlyphIndices;
    };
    
    
}
