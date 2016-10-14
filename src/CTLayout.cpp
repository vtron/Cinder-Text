//
//  CTLayout.cpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/6/16.
//
//

#include "CTLayout.hpp"

#include "harfbuzz/hb.h"

namespace typography {
    LayoutRef Layout::create() {
        return std::shared_ptr<Layout>(new Layout());
    }
    
    LayoutRef LayoutGenerator::generateLayout(typography::FontRef font, std::string text, ci::Rectf bounds) {
        // Create Harfbuzz buffer and init
        hb_buffer_t *buf = hb_buffer_create();
        
        hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
        hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
        hb_buffer_set_language(buf, hb_language_from_string("en", strlen("en")));
        
        // Shape the text
        int len = static_cast<int>(text.length());
        hb_buffer_add_utf8(buf, text.c_str(), len, 0, len);
        hb_shape(font->getHarfbuzzFont(), buf, nullptr, 0);
        
        // Get the glyphs
        unsigned int glyph_count;
        hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
        
        // Create the layout
        LayoutRef layout = Layout::create();
        
        int xPos = 0;
        
        for(int i=0; i<glyph_count; i++) {
            unsigned int glyphIndex = glyph_info[i].codepoint;
            FT_Glyph_Metrics metrics = font->getMetrics(glyphIndex);
            
            float xOffset = metrics.horiBearingX/64.0 + glyph_pos[i].x_offset;
            float xAdvance = glyph_pos[i].x_advance/64.0;
            xPos += xOffset;
            
            float yPos = font->getSize() - metrics.horiBearingY/64.0f;
            
            layout->glyphs.push_back(LayoutGlyph(font, glyphIndex, ci::vec2(xPos, yPos)));
            
            xPos += xAdvance - xOffset;
        }
        
        // Finished with buffer, delete it
        hb_buffer_destroy(buf);
        
        return layout;
    }
}
