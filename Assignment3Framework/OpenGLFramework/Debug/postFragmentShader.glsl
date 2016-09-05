// Blur example adapted from 
// - https://github.com/Overv/Open.GL/blob/master/content/articles-en/framebuffers.md

#version 330

in vec2 coord;

uniform sampler2D textureData;   

const float blurSizeH = 1.0 / 300.0;
const float blurSizeV = 1.0 / 200.0;         

void main()
{
	vec4 sum = vec4(0.0);
	
	for (int x = -4; x <= 4; x++) 
	{
		for (int y = -4; y <= 4; y++) 
		{
			sum += texture(textureData, vec2(coord.x + x * blurSizeH, coord.y + y * blurSizeV)) / 81.0;
		}
	}
    gl_FragColor = sum;
	
	// Actual color (no blur/gray scale)
	//gl_FragColor = texture(textureData, coord);
}
