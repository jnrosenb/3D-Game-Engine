//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

//Uniforms
uniform sampler2D skyMap;

in VS_OUT
{
	vec4 nml;
	vec2 uv;
};

out vec4 fragColor;

void main()
{
	float gamma = 1.0 / 2.2;
	float exposure = 1;
	
	fragColor = texture2D(skyMap, uv);

	vec3 cout = (exposure*fragColor.xyz)/(exposure*fragColor.xyz + vec3(1, 1, 1));
	cout.r = pow(cout.r, gamma);
	cout.g = pow(cout.g, gamma);
	cout.b = pow(cout.b, gamma);

	fragColor = vec4(cout, 1);
}