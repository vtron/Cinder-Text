#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CTFont.hpp"
#include "CTTextBox.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

struct TextureGlyph {
    TextureGlyph(ci::gl::TextureRef tex, ci::vec2 pos) : tex(tex), pos(pos) {}
    ci::gl::TextureRef tex;
    ci::ivec2 pos;
};

class FreetypeBasicLoadingApp : public App {
  public:
	void setup() override;
    
	void update() override;
	void draw() override;
    
    void mouseDown(MouseEvent event) override;
    
    //"NotoSans-unhinted/NotoSans-Regular.ttf"
    std::string testFont = "AGaramondPro Regular.otf";
    std::string testText = "The quick brown fox infinitely jumps over the lazy dog. Lorem ipsum dolor sit amet, te mel alia elit disputationi, sit ne possit invenire, meliore definiebas reprimique usu te. In eam sale delicatissimi, an diceret vivendo est. Ad solet nonumy scriptorem qui. No alia munere efficiendi vim, te qui labitur principes, in has harum erroribus.";
    typography::TextBoxRef testTextBox;
    int typeSize = 30;
    
    void listFonts();
    
    std::vector<TextureGlyph> harfbuzzCharacters;
    std::vector<TextureGlyph> freetypeCharacters;
};

void FreetypeBasicLoadingApp::setup()
{
    ci::app::getWindow()->setSize(1024, 768);
    ci::DataSourceRef fontFile = ci::app::loadAsset(testFont);
    typography::FontRef font = typography::Font::create(fontFile, typeSize);
    
    testTextBox = typography::TextBox::create(font, testText, 300, 300);
    
//    // Freetype (testing for comparison)
//    int xPos = 0;
//    for(int i=0; i<testText.length(); i++) {
//        ci::ChannelRef glyphChannel = font->getCharacterBitmapChannel(testText[i]);
//        ci::gl::TextureRef tex = ci::gl::Texture::create(*glyphChannel);
//        
//        float xOffset = font->getFace()->glyph->metrics.horiBearingX/64.0;
//        float xAdvance = font->getFace()->glyph->metrics.horiAdvance/64.0;
//        xPos += xOffset;
//        
//        float yPos = typeSize - font->getFace()->glyph->metrics.horiBearingY/64.0f;
//        freetypeCharacters.push_back(TextureGlyph(tex, ci::vec2(xPos, yPos)));
//        xPos += xAdvance - xOffset;
//    }
}

void FreetypeBasicLoadingApp::update()
{
}

void FreetypeBasicLoadingApp::draw()
{
    ci::gl::enableAlphaBlending();
	gl::clear( Color(0, 0, 0 ) );
    gl::color(1, 1, 1);
    
    // Draw Freetype
    {
        ci::gl::ScopedModelMatrix matrix;
        ci::gl::translate(ci::vec2(50,100));
        for(auto character : freetypeCharacters) {
            ci::gl::pushModelView();
            ci::gl::translate(character.pos);
            ci::gl::draw(character.tex);
            ci::gl::popModelView();
        }
    }
    
    // Draw renderer
    {
        ci::gl::ScopedModelMatrix matrix;
        ci::gl::translate(ci::vec2(50,150));
        testTextBox->draw();
    }
}

void FreetypeBasicLoadingApp::mouseDown(cinder::app::MouseEvent event) {
    testTextBox->setSize(event.getPos().x, 300);
    testTextBox->generateLayout();
}

CINDER_APP( FreetypeBasicLoadingApp, RendererGl )
