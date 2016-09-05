// Blur example adapted from 
// - https://github.com/Overv/Open.GL/blob/master/content/articles-en/framebuffers.md

#version 330

in vec2 coord;

uniform int textureIndex;
uniform float mouseX;
uniform float mouseY;
uniform sampler2D textureData[2];

const float blurSizeH = 1.0 / 300.0;
const float blurSizeV = 1.0 / 200.0;

float LinearizeDepth(vec2 uv) {
	float n = 1.0; // camera z near
	float f = 100.0; // camera z far
	float z = texture2D(textureData[1], uv).x;
	return (2.0 * n) / (f + n - z * (f - n));
}

float threshold(in float thr1, in float thr2 , in float val) {
	if (val < thr1) {return 0.0;}
	if (val > thr2) {return 1.0;}
	return val;
}

// averaged pixel intensity from 3 color channels
float avg_intensity(in vec4 pix) {
	return (pix.r + pix.g + pix.b)/3.;
}

vec4 get_pixel(in vec2 coords, in float dx, in float dy) {
	return texture2D(textureData[0],coord + vec2(dx, dy));
}

// returns pixel color
float IsEdge(in vec2 coords){
	float dxtex = 1.0 / 512.0 /*image width*/;
	float dytex = 1.0 / 512.0 /*image height*/;
	float pix[9];
	int k = -1;
	float delta;

	// read neighboring pixel intensities
	for (int i=-1; i<2; i++) {
		for(int j=-1; j<2; j++) {
			k++;
			pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,	float(j)*dytex));
		}
	}

	// average color differences around neighboring pixels
	delta = (abs(pix[1]-pix[7])+
			abs(pix[5]-pix[3]) +
			abs(pix[0]-pix[8])+
			abs(pix[2]-pix[6])
			)/4.;

	return threshold(0.25,0.4,clamp(1.8*delta,0.0,1.0));
}

void adaptiveSketchFilter() {
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	color.g = (1-IsEdge(coord.xy))*LinearizeDepth(coord);
	color.r = color.g;
	color.b = color.g;
	gl_FragColor = color;
}

void depthFilter() {
	vec2 uv = coord;
	float d;
	if (uv.x < 0.5) // left part
		d = LinearizeDepth(uv);
	else // right part
		d = texture2D(textureData[1], uv).x;
	gl_FragColor = vec4(d, d, d, 1.0);
}

void focusLineFilter() {
	float samples[10];
	samples[0] = -0.08;
	samples[1] = -0.05;
	samples[2] = -0.03;
	samples[3] = -0.02;
	samples[4] = -0.01;
	samples[5] =  0.01;
	samples[6] =  0.02;
	samples[7] =  0.03;
	samples[8] =  0.05;
	samples[9] =  0.08;

	vec2 mouse = vec2(mouseX/800.0, mouseY/600.0);
	vec2 dir = vec2(1.0, 0.0);
	float dist = sqrt(pow(LinearizeDepth(mouse) - LinearizeDepth(coord), 2))*100; 
	dir = dir/dist; 
	vec4 color = texture(textureData[0], coord);
	vec4 sum = color;

	for (int i = 0; i < 10; i++)
		sum += texture2D( textureData[0], coord + dir * samples[i] * 1.0 );

	sum *= 1.0/11.0;
	float t = dist * 2.2;
	t = clamp( t ,0.0,1.0);

	gl_FragColor = mix( color, sum, t );
}

void globalSketchFilter() {
	vec4 color = texture(textureData[0], coord);
	color.g = (1-threshold(0.25,0.4,clamp(color.r,color.g,color.b)))*LinearizeDepth(coord);
	color.r = color.g;
	color.b = color.g;
	gl_FragColor = color;
}

void invertedFilter() {
	float depth = LinearizeDepth(coord);
	if (depth > 0.05) {
		vec4 color = texture(textureData[0], coord);
		gl_FragColor = vec4(1 - color.r, 1 - color.g, 1 - color.b, 1.0);
	} else {
		gl_FragColor = texture(textureData[0], coord);
	}
}

void motionBlurFilter() {
	float samples[10];
	samples[0] = -0.08;
	samples[1] = -0.05;
	samples[2] = -0.03;
	samples[3] = -0.02;
	samples[4] = -0.01;
	samples[5] =  0.01;
	samples[6] =  0.02;
	samples[7] =  0.03;
	samples[8] =  0.05;
	samples[9] =  0.08;

	vec2 dir = vec2(1.0, 0.0);
	float dist = LinearizeDepth(coord)*5; 
	dir = dir/dist; 
	vec4 color = texture(textureData[0], coord);
	vec4 sum = color;

	for (int i = 0; i < 10; i++)
		sum += texture2D( textureData[0], coord + dir * samples[i] * 1.0 );

	sum *= 1.0/11.0;
	float t = dist * 2.2;
	t = clamp( t ,0.0,1.0);

	gl_FragColor = mix( color, sum, t );
}

void oilPaintingFilter () {
	int radius = 5;
	const vec2 src_size = vec2 (800.0, 600.0);
    vec2 uv = coord;
    float n = float((radius + 1) * (radius + 1));

    vec3 m[4];
    vec3 s[4];
    for (int k = 0; k < 4; ++k) {
        m[k] = vec3(0.0);
        s[k] = vec3(0.0);
    }

    for (int j = -radius; j <= 0; ++j)  {
        for (int i = -radius; i <= 0; ++i)  {
            vec3 c = texture2D(textureData[0], uv + vec2(i,j) / src_size).rgb;
            m[0] += c;
            s[0] += c * c;
        }
    }

    for (int j = -radius; j <= 0; ++j)  {
        for (int i = 0; i <= radius; ++i)  {
            vec3 c = texture2D(textureData[0], uv + vec2(i,j) / src_size).rgb;
            m[1] += c;
            s[1] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)  {
        for (int i = 0; i <= radius; ++i)  {
            vec3 c = texture2D(textureData[0], uv + vec2(i,j) / src_size).rgb;
            m[2] += c;
            s[2] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)  {
        for (int i = -radius; i <= 0; ++i)  {
            vec3 c = texture2D(textureData[0], uv + vec2(i,j) / src_size).rgb;
            m[3] += c;
            s[3] += c * c;
        }
    }


    float min_sigma2 = 1e+2;
    for (int k = 0; k < 4; ++k) {
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);

        float sigma2 = s[k].r + s[k].g + s[k].b;
        if (sigma2 < min_sigma2) {
            min_sigma2 = sigma2;
            gl_FragColor = vec4(m[k], 1.0);
        }
    }
 }

void originalBlurFilter() {
	vec4 color = texture(textureData[0], coord);
	vec4 sum = vec4(0.0);
	
	for (int x = -4; x <= 4; x++) {
		for (int y = -4; y <= 4; y++) {
			sum += texture(textureData[0], vec2(coord.x + x * blurSizeH, coord.y + y * blurSizeV)) / 81.0;
		}
	}
	gl_FragColor = sum;
}

void radialBlurFilter() {
	float samples[10];
	samples[0] = -0.08;
	samples[1] = -0.05;
	samples[2] = -0.03;
	samples[3] = -0.02;
	samples[4] = -0.01;
	samples[5] =  0.01;
	samples[6] =  0.02;
	samples[7] =  0.03;
	samples[8] =  0.05;
	samples[9] =  0.08;

	vec2 mouse = vec2(mouseX/800.0, mouseY/600.0);
	vec2 dir = mouse - coord;
	float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
	dir = dir/dist; 
	vec4 color = texture(textureData[0], coord);
	vec4 sum = color;

	for (int i = 0; i < 10; i++)
		sum += texture2D( textureData[0], coord + dir * samples[i] * 1.0 );

	sum *= 1.0/11.0;
	float t = dist * 2.2;
	t = clamp( t ,0.0,1.0);

	gl_FragColor = mix( color, sum, t );
}

void sepiaFilter() {
	vec4 color = texture(textureData[0], coord);
	gl_FragColor = vec4((color.r * .393) + (color.g *.769) + (color.b * .189), 
		(color.r * .349) + (color.g *.686) + (color.b * .168), 
		(color.r * .272) + (color.g *.534) + (color.b * .131), 1.0);
}       

void swirlDistortionFilter() {
	float radius = 200.0;
	float angle = 0.8;
	vec2 center = vec2(mouseX, mouseY);

	vec2 texSize = vec2(800, 600);
	vec2 tc = coord * texSize;
	tc -= center;
	float dist = length(tc);
	if (dist < radius) {
		float percent = (radius - dist) / radius;
		float theta = percent * percent * angle * 8.0;
		float s = sin(theta);
		float c = cos(theta);
		tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));
	}
	tc += center;
	vec3 color = texture2D(textureData[0], tc / texSize).rgb;
	gl_FragColor = vec4(color, 1.0);
}

void main()
{
	switch (textureIndex) {
		case 97:
			// a for adaptive threshold sketch filter
			adaptiveSketchFilter();
			break;
		case 98:
			// b for original blur texture
			originalBlurFilter();
			break;
		case 100:
			// d for depth filter
			depthFilter();
			break;
		case 103:
			// g for global threshold sketch filter
			globalSketchFilter();
			break;
		case 105:
			// i for inverted
			invertedFilter();
			break;
		case 108:
			// l for focus line filter
			focusLineFilter();
			break;
		case 109:
			// m for motion blur
			motionBlurFilter();
			break;
		case 111:
			// o for oil painting
			oilPaintingFilter();
			break;
		case 114:
			// r for radial blur
			radialBlurFilter();
			break;
		case 115:
			// s for swirl distortion
			swirlDistortionFilter();
			break;
		case 116:
			// t for sepia filter
			sepiaFilter();
			break;
		default:
			gl_FragColor = texture(textureData[0], coord);
			break;
	}
}
