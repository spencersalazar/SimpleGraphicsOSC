
attribute highp vec4	myVertex;
uniform mediump mat4	myPMVMatrix;
uniform mediump mat3	myModelView;

attribute lowp vec2 texCoordIn;
varying lowp vec2 texCoordOut;

void main (void)
{
	gl_Position = myPMVMatrix * myVertex;
	texCoordOut = texCoordIn;
}
