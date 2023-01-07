
module m() {
( module cube([10,20,30]) -> [10,0,0] ^> [0,45,0] )();
}

m();

translate ([20,0,0]) m();