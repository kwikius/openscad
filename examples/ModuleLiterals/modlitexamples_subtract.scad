
module subtract(a,b){
   // N.B to prevent recursion
   // difference(){a(); b();}
   (a-b)();
};

diff = module subtract(
   module { 
      translate([-20,0,-20])
         cube([40,40,40]);
   }, 
   module{ 
     translate([0,5,0]){
      sphere(d=20);
     }
   }
);

subtract(
   module diff,
   module {
      translate([0,20,5]){
         cube([20,20,40],center = true);
      }
   }
);