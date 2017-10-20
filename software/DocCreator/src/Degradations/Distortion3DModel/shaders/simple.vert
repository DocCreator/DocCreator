R"(
#version 330 core

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;//allow to pass normals from object space to view space.

in vec4 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;

out vec4 position_view;
out vec3 normal_view;
out vec2 texcoord;


void main() {

    position_view = view_matrix * vtx_position;

    gl_Position = projection_matrix * position_view;

    texcoord = vtx_texcoord;

    normal_view = normal_matrix * vtx_normal;

}

)"