
load_cell  = module {
   
   loadCellHole = module {
      centreSlot = module cube([12,14,7], center = true);
      centreSlot();
      //---------
      pos = function (x) [x,0,-7];
      largeHole = module (x) {
          rotx90 = [90,0,0];
          hole = module cylinder( d = 11.5, h = 14, $fn = 50);
          ( hole -> pos(x) ^> rotx90 )();
      };
      largeHoleOffset = 5;
      largeHole(-largeHoleOffset);
      largeHole(largeHoleOffset);  
      //---------
      mountHole = module (x) { 
         hole = module cylinder(d = 4, h = 14, $fn = 20);
         ( hole -> pos(x) )();
      };
      mountHoleOffsets = [20,35];
      for ( x = mountHoleOffsets){
         mountHole(-x);
         mountHole(x);
      }
   };
   loadCellBar = module cube([80,12.7,12.7],center = true);
   (loadCellBar - loadCellHole)();
};

load_cell();