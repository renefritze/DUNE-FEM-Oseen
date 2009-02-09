M_invers = [
 0.25       0       0       0       0       0       0       0;
    0    0.25       0       0       0       0       0       0;
    0       0    0.25       0       0       0       0       0;
    0       0       0    0.25       0       0       0       0;
    0       0       0       0    0.25       0       0       0;
    0       0       0       0       0    0.25       0       0;
    0       0       0       0       0       0    0.25       0;
    0       0       0       0       0       0       0    0.25;
];

W = [
    2       0       2       0;
   -2       0      -2       0;
    0       2       0       2;
    0      -2       0      -2;
   -2       0      -2       0;
    2       0       2       0;
    0      -2       0      -2;
    0       2       0       2;
];

X = [
    -2       2       0       0       2      -2       0       0;
     0       0      -2       2       0       0       2      -2;
    -2       2       0       0       2      -2       0       0;
     0       0      -2       2       0       0       2      -2;
];

Y_00 = 27.3137;
Y_02 = -11.3137;
Y = [
  Y_00       0    Y_02       0;
     0    Y_00       0    Y_02;
 -Y_02       0    Y_00       0;
     0   -Y_02       0    Y_00;
];

Z = [
    0      -2;
    4       2;
    2       4;
   -2       0;
];

E = [
    -2       2      -2       2;
     2      -2       2      -2;
];

R = [
    2.8284    -2.8284;
   -2.8284     2.8284;
];

H1 = [
 0;
  -2.797;
  2.3155;
  -2.797;
 0;
  -2.797;
  2.3155;
  -2.797;
];

H2 = [
    -7.6502;
    -17.4987;
    -5.4618;
    2.3063;
];

H3 = [
    -2.797;
     2.797;
];

A = Y - X * M_invers * W;

B = Z;

B_T = - E;

C = R;

F = H2 - X * M_invers * H1;

G = - H3;

A_invers = inv( A );

schur_S = B_T * A_invers * B + C;

schur_f = B_T * A_invers * F - G;

p = schur_S \ schur_f;

u = A_invers * ( F - B * p );