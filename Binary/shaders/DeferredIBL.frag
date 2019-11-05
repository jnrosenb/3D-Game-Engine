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
float Geometry(vec3 N, vec3 V, float alpha);
//float Geometry(vec3 L, vec3 H);
float Dist(vec3 N, vec3 H, float alpha);
float Fresnel(vec3, vec3, float);
vec3 GetIrradiance(vec3);
vec3 GetSpecular(vec3 wk, vec3 N, vec3 H, float alpha);
vec3 gammaCorrection(float exposure, vec3 SRGBcolor);
float pdot(vec3 A, vec3 B);


layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

layout(std140) uniform SpecularSamples
{
	int actualCount;
	vec4 pairs[64];
};

uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferDiffuse;
uniform sampler2D GBufferSpecGloss;
uniform sampler2D skyMap;
uniform sampler2D irradianceMap;
uniform sampler2D geoDepthBuffer;

uniform int maxMipmapLevel;


in VS_OUT
{
	vec2 uv;
};

out vec4 frag_color;


//MAIN
void main(void) 
{
	//DO NOT PAINT OVER PIXELS THAT HAVE NOT HAD THEIR DEPTH MODIFIED
	float depth = texture(geoDepthBuffer, uv).x;
	if (depth >= 1.0)
		discard;

	//FETCH STUFF FROM MAPS
	vec4 worldPos = texture(GBufferPos, uv);
	vec3 m = normalize(texture(GBufferNormals, uv).xyz);
	vec3 diffuse_color = texture(GBufferDiffuse, uv).xyz;
	vec3 ks = texture(GBufferSpecGloss, uv).xyz;
	float roughness = texture(GBufferSpecGloss, uv).w;
	roughness = 1.0 / pow(roughness, 2.0);
	float metallic = texture(GBufferDiffuse, uv).w;
	
	//Reflection vec
	vec3 v = normalize((eye - worldPos).xyz);
	vec3 r = 2 * m * pdot(v, m) - v;

	//AMBIENT (Ambient color without IBL)-----------------------------
	vec3 ambient = 0.25 * diffuse_color;

	//DIFFUSE---------------------------------------------------------
	vec3 diffuseIrr = ( pow(1.0 - metallic, 2) * diffuse_color / (PI) ) * GetIrradiance(m);

	//SPECULAR--------------------------------------------------------
	//Precalculate the rotation matrix
	vec3 up = normalize(r);									// Y axis in rot
	vec3 forward = cross(up, vec3(0, 1, 0));		// Z axis in rot
	vec3 right = cross(up, forward);				// X axis in rot
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
		float exponent = 1.0 / (roughness + 1.0);
		float phi = 2 * PI * pairs[i].x;			        //Goes from 0 to 2PI
		float theta = acos(pow(pairs[i].y, exponent));		//Goes from 0 to PI, with cosine falloff

		//Transform into a cartesian direction vector (skewed towards (0,1,0))
		float xterm = sin(theta) * cos(phi);
		float yterm = cos(theta);
		float zterm = sin(theta) * sin(phi);
		vec3 wk = normalize(vec3( xterm, yterm, zterm ));
		
		//Rotate so its facing the specular dir
		wk = rot * wk;
		wk = normalize(vec3(-wk.x, -wk.y, wk.z));	//Don't know why this works

		//Add to the montecarlo summation
		vec3 H = normalize(v + wk);
		vec3 L_wk = GetSpecular(wk, m, H, roughness);

		//Calculate the specular contribution for this Wk
		vec3 additionTerm;
		//float clampedDot = clamp(pdot(v, m), 0.5, 1.0);
		//float denom = 4 * clampedDot;
		float denom = 4 * pdot(v, m);
		if (denom == 0.0)				// TODO- Find out why this happens
		{								// TODO- Find out why this happens
			denom = 4 * pdot(-v, m);	// TODO- Find out why this happens
		}								// TODO- Find out why this happens
		additionTerm = L_wk * ( Fresnel(wk, H, metallic) * Geometry(m, v, roughness) / denom ); // simplified - Fresnel can use m or H
		montecarloSum += stepSize * additionTerm;
	}

	//Final color---------------------------------------------------------------
	vec3 finalClr = ambient + montecarloSum + diffuseIrr;
	frag_color = vec4(finalClr.xyz, 1);
}


//Geometry term
//float Geometry(vec3 Wk, vec3 H)
float Geometry(vec3 N, vec3 V, float alpha)
{	
	//float den = pdot(Wk, H);
	//return 1.0 / pow(den, 2);
	
	return 1.0;
	
    //float r = (alpha + 1.0);
    //float k = (r*r) / 8.0;
    //float num   = dot(N, V);
    //float denom = num * (1.0 - k) + k;
    //return num / denom;
}


//Distribution term
float Dist(vec3 Wk, vec3 H, float alpha)
{	
	//return ( (alpha + 2) / (2*PI) ) * pow(pdot(Wk, H), alpha);
	return pow(cos(PI * pdot(Wk, H)), (1 + alpha));
}


//Ratio of reflectivity
float Fresnel(vec3 Wk, vec3 H, float metallic)
{
	float base = 1.0 - dot(Wk, H);
	float expon = 5.0;
	float res = metallic + (1.0 - metallic) * pow(base, expon);
	return res;
}


//Irradiance for diffuse
vec3 GetIrradiance(vec3 N)
{	
	vec2 normalToUV =  vec2(0.5 - atan(N.z, N.x)/(2*PI), acos(N.y)/PI );
	vec3 irr = texture(irradianceMap, normalToUV).xyz;
	return gammaCorrection(1.0, irr);
}


//Irradiance for diffuse
vec3 GetSpecular(vec3 Wk, vec3 N, vec3 H, float alpha)
{
	//Get the roughness between 0 and 1
	float roughness = sqrt(1.0 / alpha);
	
	//float D = Dist(Wk, H, alpha);
	//float D = Dist(Wk, H, roughness);
	
	// 1
	//float pixelsPerKernel = (2048*1024) / actualCount;
	//float lod = 0.5 * log2(pixelsPerKernel) + 0.5f * log2(D);
	
	// 2
	//float saTexel  = (4.0 * PI) / (6.0 * 2048*1024);
	//float saSample = 1.0 / (actualCount * D + 0.0001);
	//float lod = 0.5 * log2(saSample / saTexel); 
	
	// 3
	float lod = maxMipmapLevel * sqrt(roughness);
	lod = clamp(lod, 0.0f, maxMipmapLevel);

	vec2 WkToUV =  vec2(0.5 - (atan(Wk.z, Wk.x)/(2*PI)), acos(Wk.y)/PI);
	vec3 specularL = textureLod(skyMap, WkToUV, lod).xyz;				//TODO - actually use lod
	return gammaCorrection(1.0, specularL);
}


//Gamma correction
vec3 gammaCorrection(float exposure, vec3 SRGBcolor)
{
	float gamma = 1.0 / 2.2;
	vec3 cout = (exposure * SRGBcolor) / (exposure * SRGBcolor + vec3(1, 1, 1));
	cout.r = pow(cout.r, gamma);
	cout.g = pow(cout.g, gamma);
	cout.b = pow(cout.b, gamma);
	return cout;
}


//For truncated dot function
float pdot(vec3 A, vec3 B)
{
	return max(dot(A, B), 0);
}