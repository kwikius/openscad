
module subtract(a,b){
  difference(){
    a();
    b();
  }
};

diff = module subtract(
   module { 
      translate([-20,0,-20])
         cube([40,40,40]);
   }, 
   module sphere(d=20)
);

// diff();

subtract(
   diff,
   module {
      translate([40,0,0]){
         cube([20,20,20],center = true);
      }
   }
);