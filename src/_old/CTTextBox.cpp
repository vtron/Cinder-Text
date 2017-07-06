//
//  CTTextBox.cpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 10/6/16.
//
//

#include "CTTextBox.hpp"
#include "CTRendererTexture.hpp"

namespace typography {
    TextBoxRef TextBox::create(FontRef font, std::string text, unsigned int width, unsigned int height) {
        TextBoxRef ref(new TextBox(font, text, width, height));
        return ref;
    }
    
    TextBox::TextBox(FontRef font, std::string text, unsigned int width, unsigned int height)
    : mFont(font)
    , mText(text)
    , mSize(width, height)
    {
        generateLayout();
    }
    
    void TextBox::generateLayout() {
        mLayout = typography::LayoutGenerator::generateLayout(mFont, mText, mSize.x, mSize.y);
        mRenderer = typography::RendererTexture::create();
        mRenderer->setLayout(mLayout);
    }
    
    void TextBox::draw() {
        mRenderer->draw();
    }
}

