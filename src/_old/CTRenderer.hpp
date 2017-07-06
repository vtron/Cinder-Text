//
//  CTRenderer.hpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 9/23/16.
//
//

#pragma once

#include "CTLayout.hpp"

namespace typography {
    class Renderer {
    public:
        virtual void draw() = 0;
        virtual void setLayout(LayoutRef layout) { mLayout = layout; };
        
    protected:
        LayoutRef mLayout;
    };
    
    typedef std::shared_ptr<Renderer> RendererRef;
}
