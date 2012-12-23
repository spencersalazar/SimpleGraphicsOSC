
uniform mediump vec4 color;
uniform mediump vec4 texOffset;

varying lowp vec2 texCoordOut;
uniform sampler2D tex;

void main (void)
{
	gl_FragColor = color * (texOffset + texture2D(tex, texCoordOut));
	//gl_FragColor = color;
	//gl_FragColor = color;
}
