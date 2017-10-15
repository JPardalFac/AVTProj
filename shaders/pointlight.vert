#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	
	//if the 4th component is 0, l_pos is a directional light, if it's 1, l_pos is a pointlight 
	if(l_pos.w == 1)
		DataOut.lightDir = vec3(l_pos - pos);
	else 
		DataOut.lightDir = normalize(vec3(l_pos));
	DataOut.eye = vec3(-pos);

	gl_Position = m_pvm * position;	
}