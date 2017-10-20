R"(
#version 330 core

//CODE DUPLICATION with simple.frag
// except that we do not use texture !

/*

Phong or Blinn-Phong lighting model computed per fragment (Phong shading)

*/



in vec4 position_view;
in vec3 normal_view;
in vec4 color;


out vec4 out_color;


//We suppose that light_ambient is light ambient already multiplied with material ambient


//const vec3 light_position_world = vec3(0.1, 0.1, 5.0);
//const vec3 light_ambient = vec3(.01);//vec3(0.1, 0.1, 0.1);
//const vec3 light_diffuse = vec3(0.81);//vec3(0.9);
//const vec3 light_specular = vec3(0.16); //vec3(0.4); //vec3(0.1, 0.1, 0.1); //vec3(1.0, 1.0, 1.0);

//const float specular_exponent = 6.0; //shininess


uniform vec3 light_position_view;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform float specular_exponent;



void main()
{
  // //ambient intensity
  // vec3 Ia = light_ambient;

  // //diffuse intensity
  // //vec3 light_position_view = vec3(view_matrix * vec4(light_position_world, 1.0)); //B:TODO in c++ code !!!
  
  // //B: we always use normalized 'l' vector.
  // // According to sources, it is not always the case...

  // vec3 l = normalize(light_position_view - position_view.xyz); //surface to light
  // float dot_prod = max(dot(l, normal_view), 0.);
  // vec3 Id = light_diffuse * dot_prod;

  // //specular intensity
  // vec3 v = normalize( - position_view.xyz); //surface to viewer
  
  // /*
  // //Phong
  // vec3 reflection_view = reflect(-l, normal_view);
  // float dot_prod_specular = max(dot(reflection_view, v), 0.);
  // */
  // //Blinn-Phong
  // vec3 half_way = normalize(v + l);
  // float dot_prod_specular = max(dot(half_way, normal_view), 0.);
  
  // float specular_factor = pow(dot_prod_specular, specular_exponent);

  // vec3 Is = light_specular * specular_factor;
 
  // out_color = color * vec4(Id + Ia + Is, 1.0);
  out_color = color;
}

)"