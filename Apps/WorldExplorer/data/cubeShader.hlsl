/* Vertex Shader */

struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexInput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 normal : NORMAL0;
};

VertexShaderOutput vs_main(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.position = mul(ModelViewProjectionCB.MVP, IN.position);
    OUT.color = IN.color;
    OUT.uv = IN.uv;
    OUT.normal = IN.normal;

    return OUT;
}

/* Pixel Shader */

struct PixelOutput // la structure de sortie permet de définir les branchements entre les variables code et les trucs bindé
{
  float4 color : SV_TARGET0; // la variable color est lié à la RenderTarget de sortie 0
  /*
  // si plusieurs RT comme l'oEngine le GBuffer ça donnerait
  float4 Normal : SV_TARGET1;
  float4 Spec : SV_TARGET2;
  */
};

PixelOutput ps_main(VertexShaderOutput IN)
{
    PixelOutput OUT;

    // Return gamma corrected color.
    //OUT.color = pow( abs( IN.color ), 1.0f / 2.2f );
    float dotValue = dot(float3(0, 1, 0), float3(IN.normal.x, IN.normal.y, IN.normal.z));
    OUT.color =  float4(dotValue, dotValue, dotValue, 1.0);

    return OUT;
}
