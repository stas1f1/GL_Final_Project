varying vec3 normal;
varying vec3 position;  
varying vec2 texture;
  
uniform sampler2D texture_image; 

uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;

uniform bool point_active;
varying vec3 point_position_view;
uniform vec3 point_ambient;
uniform vec3 point_diffuse;
uniform vec3 point_specular;

uniform bool spot_active;
varying vec3 spot_position_view;
varying vec3 spot_direction_view;
uniform vec3 spot_ambient;
uniform vec3 spot_diffuse;
uniform vec3 spot_specular;
uniform float spot_cutoff;

uniform bool directional_active;
uniform vec3 directional_direction;
varying vec3 directional_direction_view;
uniform vec3 directional_ambient;
uniform vec3 directional_diffuse;
uniform vec3 directional_specular;

void main (void)  
{  
  vec3 texture_color = vec3(texture2D(texture_image, texture));
  vec3 color;
  vec3 norm = normalize(normal);
  
  if (point_active) {
    vec3 color_ambient = material_ambient*point_ambient;

	vec3 surf2light = normalize(point_position_view-position);
	float dcont = max(0.0, dot(norm, surf2light));
	vec3 color_diffuse = material_diffuse*dcont*point_diffuse;

	vec3 surf2view=normalize(-point_position_view);
    vec3 reflection=reflect(-surf2light,norm);
    float scont=pow(max(0.0,dot(surf2view,reflection)), 10);
    float dist=length(position-point_position_view);
    float att=1.0/(1.0+0.1*dist+0.01*dist*dist);
    vec3 color_specular = scont * dcont * point_specular * material_specular * att;
    
    color += color_ambient + color_diffuse + color_specular;
  }

  if (spot_active) {

    vec3 color_ambient = material_ambient*spot_ambient;
	vec3 color_diffuse = vec3(0.0, 0.0, 0.0);
	vec3 color_specular = vec3(0.0, 0.0, 0.0);

	vec3 surf2light = normalize(spot_position_view-position);
	vec3 direction=normalize(spot_position_view-spot_direction_view);
    float angle = acos(dot(surf2light,direction));
    if ( angle < radians(spot_cutoff) ) {
		float dcont = max(0.0, dot(norm, surf2light));
		color_diffuse = material_diffuse*dcont*spot_diffuse;
		color_specular = material_specular*spot_specular;

		vec3 surf2view=normalize(-spot_position_view);
		vec3 reflection=reflect(-surf2light,norm);
		float scont=pow(max(0.0,dot(surf2view,reflection)), 10);
		float dist=length(position-spot_position_view);
		float att=1.0/(1.0+0.1*dist+0.01*dist*dist);
		vec3 color_specular = scont*dcont*point_specular*material_specular*att;
	}
    color += color_ambient + color_diffuse + color_specular;
  }

  if (directional_active) {
	vec3 color_ambient = material_ambient*directional_ambient;

	vec3 surf2light = normalize(directional_direction_view);
	float dcont = max(0.0, dot(norm, surf2light));
	vec3 color_diffuse = material_diffuse*dcont*directional_diffuse;

    vec3 color_specular = material_specular*directional_specular;
    
    color += color_ambient + color_diffuse + color_specular;

  }
   
  gl_FragColor += vec4(texture_color*color, 1); 
}