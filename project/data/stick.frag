
//varying lowp vec4 colorVarying;
//
//void main()
//{
//    gl_FragColor = colorVarying;
//}


uniform mediump mat4 ViewMatrix;
uniform mediump mat4 ModelViewMatrix;
uniform mediump mat4 ModelMatrix;
uniform mediump mat4 ProjectionMatrix;

uniform mediump mat3 NormalMatrix;

uniform mediump vec4 LightPos;
uniform mediump vec4 EyePos;

uniform lowp vec3 Ka;   // ambient material coefficient
uniform lowp vec3 Kd;   // diffuse material coefficient
uniform lowp vec3 Ks;   // specular material coefficient

uniform mediump float Ns;   // specular material exponent (shininess)
uniform mediump float Ni;   // optical density (1.0 means light does not bend while passing through object)
uniform mediump float transparency;
uniform mediump vec3 ambientColor;

uniform lowp vec3 Ia;   // ambient light intensity
uniform lowp vec3 Id;   // diffuse light intensity
uniform lowp vec3 Is;   // specular light intensity

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D SphereMap;

uniform samplerCube skyboxDiffuse;
uniform samplerCube skyboxSpecular;

varying lowp vec4 ambientVarying;
varying lowp vec4 diffuseVarying;
varying lowp vec4 specularVarying;
varying lowp vec4 texCoordVarying;

varying mediump vec4 posVarying;       // pos in world space
varying mediump vec3 normalVarying;    // normal in world space

varying mediump vec4 pos;
varying mediump vec3 normal;
varying mediump vec3 cameraVector;
varying mediump vec3 tangentVarying;

void main()
{
    
    /*TBN calculation*/
    
    mediump vec3 t = normalize(vec3(NormalMatrix * tangentVarying));
    mediump vec3 n1 = normalize(vec3(NormalMatrix * normal));
    mediump vec3 b = normalize(vec3(NormalMatrix * cross(t, n1)));
    
    mediump vec3 tOrthogonalized = t-dot(n1,t)*n1;
    mediump vec3 bOrthogonalized = b-dot(n1,b)*n1 - dot(tOrthogonalized,b)*tOrthogonalized;
    mediump mat3 tbn = mat3(tOrthogonalized, bOrthogonalized, n1);
    
    //read normal out of NormalMap
    mediump vec3 n = normalize(tbn *  texture2D(NormalMap, texCoordVarying.st).rgb * 2.0 - 1.0);
    
    /*IBL*/
    
    mediump vec3 N = normalize(normal);
        
    mediump vec3 difLighting = textureCube(skyboxDiffuse, n).rgb;
    
    lowp vec4 color = texture2D(DiffuseMap, vec2(texCoordVarying));
    
    mediump vec4 iblColor;
    iblColor.xyz = vec3(color) * difLighting * 0.6;
    iblColor.a = 1.0;
    
    gl_FragColor = 7.0 * iblColor;

    
    /////////////////////////////////
    
//    mediump vec3 n = normalize(texture2D(NormalMap, texCoordVarying.st).rgb); // * 2.0 - 1.0);
//    mediump vec3 l = normalize(LightPos - pos).xyz;
//    
//    mediump vec3 Ca = Ka * Ia;
//    mediump vec3 Cd = Kd * max(0.0,dot(n,l)) * Id;
//    
//    mediump vec3 Cs = vec3(0.0);
//    if (dot(n,l) > 0.0)
//    {
//        mediump vec3 v = normalize(EyePos - pos).xyz;
//        mediump vec3 r = normalize(l + v);
//        
//        Cs = Ks * pow(dot(n,r),Ns) * Is;
//    }
//    
//    //read color from DiffuseMap
//    lowp vec4 color = texture2D(DiffuseMap, vec2(texCoordVarying));
//    lowp float colorAlpha = 1.0;
//    lowp vec4 colorTransp = (vec4(clamp(Cd, 0.0, 1.0), colorAlpha) + vec4(Ca, colorAlpha)) * color + vec4(clamp(Cs, 0.0, 1.0), colorAlpha);
//            gl_FragColor = vec4(n * 0.5 + vec3(0.5),1.0);
//    gl_FragColor = color;
}