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
    // A glyph with position and
    // it's associated font
    typedef std::shared_ptr<class LayoutGlyph> LayoutGlyphRef;
    typedef std::vector<LayoutGlyphRef> LayoutLine;
    typedef std::shared_ptr<LayoutLine> LayoutLineRef;
    
    class LayoutGlyph {
    public:
        static LayoutGlyphRef create(FontRef font, unsigned int glyphIndex, ci::vec2 position) {
            return LayoutGlyphRef(new LayoutGlyph(font, glyphIndex, position));
        }
        
        const FontRef font;
        const unsigned int index;
        const ci::vec2 position;
        
    private:
        LayoutGlyph(FontRef font, unsigned int glyphIndex, ci::vec2 position) :
        font(font),
        index(glyphIndex),
        position(position)
        {}
    };
    
    // Layout
    // Describes the positions of glyphs
    // associated with a font
    
    typedef std::shared_ptr<class Layout> LayoutRef;
    
    class Layout {
    public:
        typedef enum Alignment { LEFT, CENTER, RIGHT } Alignment;
        enum { GROW = 0 };
        
        static LayoutRef create();
        
        std::vector<LayoutLineRef> lines;
        std::vector<LayoutGlyphRef> getAllGlyphs() {
            std::vector<LayoutGlyphRef> allGlyphs;
            
            for(auto line : lines) {
                allGlyphs.insert(allGlyphs.end(), line->begin(), line->end());
            }
            
            return allGlyphs;
        }
        
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
