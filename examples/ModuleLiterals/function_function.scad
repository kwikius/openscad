
a = function (fnarg) 
   function (harg) 
      module (darg, height = harg,fn = fnarg) 
      { 
         echo(darg); 
         cylinder(d= darg, h = height, $fn =fn); 
      };
fn = 10;
h = 2;
d = 4;

(a(fn)(h))(d); // ECHO: 3  and output cylinder on viewport
// https://github.com/kwikius/openscad/tree/module_literal_v3