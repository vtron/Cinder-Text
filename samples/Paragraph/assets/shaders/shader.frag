#version 150

uniform sampler2DArray uTexArray;
uniform uint uLayer;
uniform vec2 uSubTexSize;

in vec2 texCoord;
in vec4 globalColor;

out vec4 color;

void main( void )
{ 
	vec3 coord = vec3(texCoord.x * uSubTexSize.x, texCoord.y * uSubTexSize.y, uLayer);
	vec4 texColor = texture( uTexArray, coord );

	color = vec4(1.0, 1.0, 1.0, texColor.r);
	color = color * globalColor;
}