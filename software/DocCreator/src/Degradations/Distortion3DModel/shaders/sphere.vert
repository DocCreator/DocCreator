R"(
#version 330 core

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;//allow to pass normals from object space to view space.
uniform vec4 sphere_color;

in vec4 vtx_position;
in vec3 vtx_normal;
//in vec4 vtx_color;

out vec4 position_view;
out vec3 normal_view;
out vec4 color;

void main() {

    position_view = view_matrix * vtx_position;

    gl_Position = projection_matrix * position_view;

    normal_view = normal_matrix * vtx_normal;

    color = sphere_color; //vtx_color;

}

)"