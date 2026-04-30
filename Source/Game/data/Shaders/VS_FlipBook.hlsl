#include "common.hlsli"


float4 main(ModelVertexToPixel input) : SV_TARGET
{
   // Inputs (UV + Time)
   float2 uv = input.texCoord0;
   float time = TotalTime;

   // Flipbook Layout (Rows + Columns)
   float columns = 3;
   float rows = 3;
   float totalframes = columns * rows;

   // Speed (frames per second)
   float fps = 60;

   // Flipbook Calculations
   float frame = (int)floor(time * fps);
   frame = frame % totalframes;

   // Frame to Grid
   int col = frame % columns;
   int row = frame / columns;

   // Cell Scale
   float2 cellscale = float2(1.0 / columns, 1.0 / rows);

   // Final Calculations
   float2 finalUV;
   finalUV.x = uv.x * cellscale.x + col * cellscale.x;
   finalUV.y = uv.y * cellscale.y + row * cellscale.y;

   return albedoTexture.Sample(defaultSampler, finalUV);
}