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
    typedef std::shared_ptr<class TextBox> TextBoxRef;
    
    class TextBox {
    public:
        TextBoxRef create(FontRef font, std::string text, int width, int height);
        
        void draw();
        
    protected:
    private:
        TextBox(Font font, std::string text, int width, int height);
        
        void generateLayout();
        
        RendererRef mRenderer;
        
        FontRef mFont;
        std::string mText;
        ci::vec2 size;
        LayoutRef mLayout;
    };
}
