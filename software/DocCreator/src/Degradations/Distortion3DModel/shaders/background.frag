R"(
#version 330 core

uniform sampler2D colorMap;

//in vec4 position_view;
//in vec3 normal_view;
in vec2 texcoord;

out vec4 out_color;

//We suppose that light_ambient is light ambient already multiplied with material ambient

uniform bool use_texture;
uniform mat3 tex_matrix;


void main()
{
  out_color = vec4(1.0, 0.0, 0.0, 1.0);
  if (use_texture) {
    vec3 uv = tex_matrix * vec3(texcoord.xy, 1.0);
    
    vec4 t = texture(colorMap, uv.xy);

    out_color = t;
  }

}

)"