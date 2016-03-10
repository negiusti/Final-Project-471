#version 330 core 
in vec3 fragNor;
in vec4 fragColor;
in vec3 light;
in vec3 view;
out vec4 color;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float Shine;

void main()
{
	//phong
	vec3 normal = normalize(fragNor);
	vec3 reflection = normalize(fragNor);
	color = vec4(MatDif*max(dot(normalize(light), normal), 0) 
	+ MatSpec*max(pow(dot(normal, normalize((view + light) / 2.0)), Shine), 0)
	+ MatAmb, 1.0);

	//goraud
	//vec3 normal = normalize(fragNor);
	//vec3 Ncolor = 0.5*normal + 0.5;
	//color = vec4(Ncolor, 1.0);

}
