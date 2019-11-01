//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define SPECULAR_N		30
#define PI				3.141592


// Geometry term
float Geometry(vec3 N, vec3 H, float roughness);
float Fresnel(vec3, vec3, float);
vec3 GetIrradiance(vec3);
vec3 GetSpecular(vec3);


layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

layout(std140) uniform SpecularSamples
{
	int actualCount;
	vec2 pairs[100];
};

uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferDiffuse;
uniform sampler2D GBufferSpecGloss;
uniform sampler2D skyMap;
uniform sampler2D irradianceMap;
uniform sampler2D geoDepthBuffer;

in VS_OUT
{
	vec2 uv;
};

out vec4 frag_color;


//MAIN
void main(void) 
{
	//This is so it doesnt paint over
	float depth = texture(geoDepthBuffer, uv).x;
	if (depth >= 1.0)
		discard;

	//This should be a uniform***
	float roughness = 0.01;

	//FETCH STUFF FROM MAPS
	vec4 worldPos = texture(GBufferPos, uv);
	vec3 m = normalize(texture(GBufferNormals, uv).xyz);
	vec3 diffuse_color = texture(GBufferDiffuse, uv).xyz;
	vec3 ks = texture(GBufferSpecGloss, uv).xyz;
	float specExponent = texture(GBufferSpecGloss, uv).w;
	vec3 v = normalize((eye - worldPos).xyz);

	//AMBIENT (Ambient color without IBL)-----------------------------
	vec3 ambient = 0.2 * diffuse_color;

	//DIFFUSE---------------------------------------------------------
	vec3 diffuseIrr = (diffuse_color / (2*PI)) * GetIrradiance(m);

	//SPECULAR--------------------------------------------------------
	//Precalculate the rotation matrix
	vec3 r = 2 * m * max(dot(v, m), 0) - v;
	//vec3 r = reflect(v, m);
	vec3 up = normalize(r);						// Y axis in rot
	vec3 forward = cross(vec3(0, 1, 0), up);	// Z axis in rot
	vec3 right = cross(forward, r);				// X axis in rot
	mat3 rot;
	rot[0] = right;
	rot[1] = up;
	rot[2] = forward;

	//Go through each of the specular sampling dirs, and turn them into each wk
	vec3 montecarloSum = vec3(0, 0, 0);
	float stepSize = 1.0 / actualCount;
	for (int i = 0; i < actualCount; ++i)
	{
		//Get hammersmield random pair and turn into D distribution
		vec2 dpair = vec2(0.0);
		dpair.x = pairs[i].x;
		float exponent = 1.0 / (roughness + 1.0);
		dpair.y = acos( pow(pairs[i].y, exponent)) / PI;

		//Transform into a cartesian direction vector
		float term = 2.0 * PI / ( 0.5 - dpair.x );
		
		float xterm = cos(term) * sin( PI * dpair.y );
		float yterm = cos(PI * dpair.y);
		float zterm = sin(term) * sin( PI * dpair.y );
		vec3 wk = vec3( xterm, yterm, zterm );
		
		//Rotate so its facing the specular dir
		wk = normalize( rot * wk );
		wk = normalize(vec3(-r.x, -r.y, r.z)); //*******THIS IS NOT REALLY CORRECT

		//Add to the montecarlo summation (for now, using r instead of L)
		vec3 H = normalize(m + wk);
		vec3 Lw = GetSpecular(wk);
		vec3 additionTerm = Fresnel(r, m, ks.x) * Geometry(m, H, roughness) * Lw * max(dot(Lw, m), 0) / ( 4 * dot(Lw, m) * dot(v, m) );
		montecarloSum += (stepSize * additionTerm);
	}

	//Final color---------------------------------------------------------------
	vec3 finalClr = ambient + montecarloSum + diffuseIrr;
	frag_color = vec4(finalClr.xyz, 1);
}

//Geometry term
float Geometry(vec3 N, vec3 H, float alpha)
{	
	//return 1.0;
	return ( (alpha + 2) / (2*PI) ) * pow(dot(N,H), alpha);
}

//Ks is specular intensity apparently
float Fresnel(vec3 L, vec3 H, float ks)
{
	return 1.0;
	//float base = 1.0 - max(dot(L, H), 0);
	//float expon = 5.0;
	//float res = ks + (1.0 - ks) * pow(base, expon);
	//return res;
}

//Irradiance for diffuse
vec3 GetIrradiance(vec3 N)
{
	float gamma = 1.0 / 2.2;
	float exposure = 10;
	
	vec2 normalToUV =  vec2(0.5 - atan(N.y, N.x)/(2*PI), acos(N.z)/PI );
	vec3 irr = texture(irradianceMap, normalToUV).xyz;

	vec3 cout = (exposure * irr) / (exposure * irr + vec3(1, 1, 1));
	cout.r = pow(cout.r, gamma);
	cout.g = pow(cout.g, gamma);
	cout.b = pow(cout.b, gamma);

	return cout;
}


//Irradiance for diffuse
vec3 GetSpecular(vec3 N)
{
	vec2 normalToUV =  vec2(0.5 - atan(N.z, N.x)/(2*PI), acos(N.y)/PI);
	vec3 specularL = texture(skyMap, normalToUV).xyz;
	return specularL;

	//float gamma = 1.0 / 2.2;
	//float exposure = 1;
	//
	//vec2 normalToUV =  vec2(0.5 - atan(N.y, N.x)/(2*PI), acos(N.z)/PI);
	//vec3 specularL = texture(skyMap, normalToUV).xyz;
	//
	//vec3 cout = specularL;//(exposure * specularL) / (exposure * specularL + vec3(1, 1, 1));
	//cout.r = pow(cout.r, gamma);
	//cout.g = pow(cout.g, gamma);
	//cout.b = pow(cout.b, gamma);
	//
	//return cout;
}