

xrotate = module(S, p, ax){
  (S -> -p ^> [ax,0,0] -> p)();
};

xrotate(module cube([10,10,40]),[0,0,40],45);