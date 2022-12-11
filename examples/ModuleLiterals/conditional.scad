/*
module in conditional
*/

cond = true;

if (cond){
  m = module cube([10,20,30], center = true);
  m();
}else{
  m = module cylinder(h = 10, r = 5);
  m();
}

