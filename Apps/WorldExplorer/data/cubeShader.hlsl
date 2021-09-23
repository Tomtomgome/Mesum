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
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION0;
    float4 color : COLOR0;
};

VertexShaderOutput vs_main(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.position = mul(ModelViewProjectionCB.MVP, IN.position);
    OUT.color = IN.color;

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
    OUT.color = pow( abs( IN.color ), 1.0f / 2.2f );

    return OUT;
}
