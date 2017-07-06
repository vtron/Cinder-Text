//
//  CTLayout.cpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/6/16.
//
//

#include "CTLayout.hpp"

#include "harfbuzz/hb.h"
#include "freetype2/ft2build.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace typography {
    // Harfbuzz shaped text
    // A string, shaped by harfbuzz for one font with resulting glyphs
    typedef std::shared_ptr<class HarfbuzzShapedText> HarfbuzzShapedTextRef;
    
    class HarfbuzzShapedText {
    public:
        static HarfbuzzShapedTextRef create(FontRef font, std::string text) {
            return HarfbuzzShapedTextRef(new HarfbuzzShapedText(font, text));
        };
        
        ~HarfbuzzShapedText() {
            // Destroy our buffer (invalidates all other pointers)
            hb_buffer_destroy(mBuf);
        }
        
        FontRef getFont() { return mFont; };
        int getTotalGlyphs() { return mGlyphCount; };
        unsigned int getGlyphCodepoint(int index) { return mGlyphInfo[index].codepoint; };
        hb_glyph_position_t getGlyphPosition(int index) { return mGlyphPositions[index]; };
        
    private:
        HarfbuzzShapedText(FontRef font, std::string text)
        : mFont(font)
        {
            // Create Harfbuzz buffer and init
            mBuf = hb_buffer_create();
            
            hb_buffer_set_direction(mBuf, HB_DIRECTION_LTR);
            hb_buffer_set_script(mBuf, HB_SCRIPT_LATIN);
            hb_buffer_set_language(mBuf, hb_language_from_string("en", strlen("en")));
            
            // Shape the text
            int len = static_cast<int>(text.length());
            hb_buffer_add_utf8(mBuf, text.c_str(), len, 0, len);
            hb_shape(font->getHarfbuzzFont(), mBuf, nullptr, 0);
            
            // Get the glyphs
            mGlyphInfo = hb_buffer_get_glyph_infos(mBuf, &mGlyphCount);
            mGlyphPositions = hb_buffer_get_glyph_positions(mBuf, &mGlyphCount);
        }
        
        const FontRef mFont;
        
        
        hb_glyph_info_t *mGlyphInfo;
        hb_glyph_position_t *mGlyphPositions;
        unsigned int mGlyphCount;
        hb_buffer_t *mBuf;
    };
    
    
    
    
    LayoutRef Layout::create() {
        return std::shared_ptr<Layout>(new Layout());
    }
    
    // Detect whitespace, for now just English spaces
    bool isWhitespace(FontRef font, int codepoint) {
        FT_UInt spaceIndex = FT_Get_Char_Index(font->getFace(), ' ');
        return codepoint == spaceIndex;
    }
    
    LayoutRef layoutTextBox(typography::FontRef font, std::string text, unsigned int width, unsigned int height) {
        LayoutRef layout = Layout::create();
        
        HarfbuzzShapedTextRef hbText = HarfbuzzShapedText::create(font, text);
        
        float leading = 0;
        int xPos = 0;
        int yPos = 0;
        
        LayoutLineRef currentLine(new LayoutLine());
        int currentWordStartPos = 0;
        std::vector<LayoutGlyphRef> currentWord;
        
        for(int i=0; i<hbText->getTotalGlyphs(); i++) {
            // Get the glyph index, see if we're at a word breakpoint
            unsigned int glyphIndex = hbText->getGlyphCodepoint(i);
            
            if(isWhitespace(font, glyphIndex)) {
                currentLine->insert(currentLine->end(), currentWord.begin(), currentWord.end());
                currentWord.clear();
                currentWordStartPos = i;
            }
            
            // Metrics
            FT_Glyph_Metrics metrics = hbText->getFont()->getMetrics(glyphIndex);
            
            float xOffset = metrics.horiBearingX/64.0 + hbText->getGlyphPosition(i).x_offset;
            float xAdvance = hbText->getGlyphPosition(i).x_advance/64.0;
            
            // Advance our position to start of glyph
            xPos += xOffset;
            
            // Get a vector for the position this glyph should start
            ci::vec2 glyphPos(xPos, yPos + hbText->getFont()->getSize() - metrics.horiBearingY/64.0f);
            
            // Advance our position to the end of glyph
            xPos += xAdvance - xOffset;
            
            // Check to see if we went over the bounds,
            // if so step back the character
            if(xPos > width) {
                xPos = 0;
                yPos += hbText->getFont()->getFace()->size->metrics.height/64.0 + leading;
                
                i = currentWordStartPos;
                currentWord.clear();
            } else {
                currentWord.push_back(LayoutGlyph::create(hbText->getFont(), glyphIndex, glyphPos));
            }
        }
        
        currentLine->insert(currentLine->end(), currentWord.begin(), currentWord.end());
        layout->lines.push_back(currentLine);
        
        return layout;
    }
                      
    
    LayoutRef LayoutGenerator::generateLayout(typography::FontRef font, std::string text, unsigned int width, unsigned int height) {
        HarfbuzzShapedTextRef harfbuzzText = HarfbuzzShapedText::create(font, text);
        return layoutTextBox(font, text, width, height);
    }
    
}
