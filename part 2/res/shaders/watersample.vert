/////////////////////////////
// Attributes
attribute vec4 a_position;

/////////////////////////////
// Uniforms
uniform mat4 u_worldViewProjectionMatrix;

/////////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;

/////////////////////////////
void main()
{
	v_vertexRefractionPosition = u_worldViewProjectionMatrix * a_position;
	
	gl_Position = v_vertexRefractionPosition;
}