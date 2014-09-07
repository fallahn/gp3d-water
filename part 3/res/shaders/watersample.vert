/////////////////////////////
// Attributes
attribute vec4 a_position;

/////////////////////////////
// Uniforms
uniform mat4 u_worldViewProjectionMatrix;
uniform mat4 u_worldViewProjectionReflectionMatrix;

/////////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;
varying vec4 v_vertexReflectionPosition;

/////////////////////////////
void main()
{
	v_vertexRefractionPosition = u_worldViewProjectionMatrix * a_position;
	v_vertexReflectionPosition = u_worldViewProjectionReflectionMatrix * a_position;
	
	gl_Position = v_vertexRefractionPosition;
}