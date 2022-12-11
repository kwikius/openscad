/*
module in conditional
*/

cond = false;

((cond)
  ? module cube([10,20,30], center = true)
  : module cylinder(h = 10, r = 5)
)();

