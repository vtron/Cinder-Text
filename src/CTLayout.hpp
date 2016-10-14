//
//  CTLayout.hpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/6/16.
//
//

#pragma once

#include "CTFont.hpp"

namespace typography {
    struct LayoutGlyph {
        LayoutGlyph(FontRef font, int glyphIndex, ci::vec2 position) :
        font(font),
        index(glyphIndex),
        position(position)
        {}
        
        FontRef font;
        int index;
        ci::vec2 position;
    };
    
    // Layout
    // Describes the positions of glyphs
    // associated with a font
    
    typedef std::shared_ptr<class Layout> LayoutRef;
    
    struct Layout {
    public:
        static LayoutRef create();
        
        std::vector<LayoutGlyph> glyphs;
    private:
        Layout() {};
    };
    
    // Layout Generator
    class LayoutGenerator {
    public:
        
        static LayoutRef generateLayout(FontRef font, std::string text, ci::Rectf bounds);
        
    private:
        LayoutGenerator();
    };
}
