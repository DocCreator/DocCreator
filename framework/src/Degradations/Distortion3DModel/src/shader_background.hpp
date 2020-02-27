#pragma once

const std::string vs_background = R"(

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
//uniform mat3 normal_matrix;//allow to pass normals from object space to view space.
//uniform vec4 sphere_color;

in vec4 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;

out vec4 position_view;
//out vec3 normal_view;
out vec2 texcoord;

void main() {

    position_view = view_matrix * vtx_position;

    gl_Position = projection_matrix * position_view;

    //normal_view = normal_matrix * vtx_normal;

    //color = sphere_color; //vtx_color;

    texcoord = vtx_texcoord;
}
)";

const std::string fs_background = R"(

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
)";
