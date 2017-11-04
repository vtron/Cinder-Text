#version 150

uniform sampler2DArray uTexArray;
uniform uint uLayer;
uniform vec2 uSubTexSize;

in vec2 texCoord;
in vec4 globalColor;

out vec4 oColor;

void main( void )
{ 
	vec3 coord = vec3(texCoord.x * uSubTexSize.x, texCoord.y * uSubTexSize.y, uLayer);
	vec4 color = texture( uTexArray, coord );

	oColor = vec4(color.r, 1.0, 1.0, color.r);
	//oColor = color;
}