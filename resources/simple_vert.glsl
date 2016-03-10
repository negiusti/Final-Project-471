#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;
uniform mat4 V;
uniform vec3 lightPos;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float Shine;
out vec3 fragNor;
out vec3 light;
out vec3 view;
out vec4 fragColor;

void main()
{
	gl_Position = P * V * MV * vertPos;

	//phong
	fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	light = normalize(lightPos - vec3(MV*vertPos));
	view  = normalize(-1 * vec3(MV*vertPos));

	//gouraud
	//vec3 normal = normalize(fragNor);
	//vec3 reflection = normalize(fragNor);
	//fragColor = vec4(MatDif*max(dot(normalize(light), normal), 0) 
	//	+ MatSpec*max(pow(dot(normal, normalize((view + light) / 2.0)), Shine), 0)
	//	+ MatAmb, 1.0);
}
