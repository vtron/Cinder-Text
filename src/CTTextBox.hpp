//
//  CTTextBox.hpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/6/16.
//
//

#pragma once
#include "CTFont.hpp"
#include "CTRenderer.hpp"
#include "CTLayout.hpp"

namespace typography {
    class TextBox;
    typedef std::shared_ptr<TextBox> TextBoxRef;
    
    class TextBox {
    public:
        static TextBoxRef create(FontRef font, std::string text, unsigned int width, unsigned int height);
        
        void setSize(unsigned int width, unsigned int height) { mSize = ci::vec2(width, height); };
        
        void generateLayout();
        
        void draw();
        
    private:
        TextBox(FontRef font, std::string text, unsigned int width, unsigned int height);
        
        RendererRef mRenderer;
        
        FontRef mFont;
        std::string mText;
        LayoutRef mLayout;
        ci::ivec2 mSize;
    };
}
