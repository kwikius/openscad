
// modified from
// https://github.com/doug-moen/openscad2/blob/master/rfc/Objects.md#object-literals

lollipop = module (radiusIn) {
  radius   = radiusIn; // candy
  diameter = 3;  // stick
  height   = 50; // stick
  translate([0,0,height]) sphere(r=radius);
  cylinder(d=diameter,h=height);
};

// Member access is only available for module without arguments
// but for modules with arguments,you can create a module reference 
// to the module with all arguments supplied. 
// This could be later modified to e.g lollipop(20).radius
lollipop1 = module lollipop(20);
echo(lollipop_radius = lollipop1.radius);
lollipop1();