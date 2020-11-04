function [Gd]=makeACMdl(t1m0, Wsun, Ts)

% [ts1 ts2 td1 td2] -> d/dt [ts1;ts2;td1;td2]
A0 = [
    0 0 0 0
    0 -4e-3 0 0
    0 0 -0.4e-3 0
    0 0 0 -7e-3
    ];
% [W1+W2+W01+W012 W2-W1-W01 Wx] -> d/dt [ts1;ts2;td1;td2]
B3 = [
    0.14e-6 0 0
    0 0 2e-6
    0 0.38e-6 0
    0 0 -10e-6
    ];
% [W1 W2 W01 W012 Wx] -> [W1+W2+W01+W012 W2-W1-W01 Wx]
B2 = [
    +1 +1 +1 +1 0
    -1 +1 -1 0  0
    0  0  0  0  +1
    ];
% [heat ac1 ac2 f012 cur t0d w0 w1 w2] -> [W1 W2 W01 W012 Wx]
B1 = [
    666  2500 0    0          0          0   0    200 0
    0    0    2500 0          0          0   1100 0 300
    0    0    0  -350*t1m0    Wsun-50    0   0    0 0
    0    0    0    0          0          155 0    0 0
    500  0    0  -140*t1m0    0          0   0    0 0
    ];
% [ts td] -> [W1 W2 W01 W012 Wx]
BA = [
    0 0
    0 0
    0 0
    -155 0
    0 0
    ];
% [ts1 ts2 td1 td2] -> [ts td]
C2 = [
    1 1 0 0
    0 0 1 1
    ];
% [t1 t2] -> [ts td]
C1 = [
    0.25 0.75
    -1 1
    ];
% [heat ac1 ac2 f012 cur t0d w0 w1 w2] -> [t1 t2]
D0 = [
    0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0
    ];
% [ac acd f012 cur t0d w0 w1 w2] -> [ac1 ac2 f012 cur t0d w0 w1 w2]
% X = [
%     0.5 -0.5 0 0 0 0 0 0
%     0.5 0.5 0 0 0 0 0 0
%     0 0 1 0 0 0 0 0
%     0 0 0 1 0 0 0 0
%     0 0 0 0 1 0 0 0
%     0 0 0 0 0 1 0 0
%     0 0 0 0 0 0 1 0
%     0 0 0 0 0 0 0 1
%     ];

A = A0 + B3*B2*BA*C2;
B = B3*B2*B1; % *X
C = C1\C2;
D = D0; % *X

G = ss(A, B, C, D, ...
    'InputName', {'heat', 'ac1', 'ac2', 'f012', 'cur', 't0d', 'w0', 'w1', 'w2'}, ...
    'OutputName', {'t1', 't2'}, ...
    'InputDelay', [30, 60, 60, 0, 180, 0, 0, 0, 0], ...
    'OutputDelay', [0, 120]);

Gd = c2d(G, Ts);
Gd = setmpcsignals(Gd, 'MV', [1 2 3 4 5], 'MD', [6 7 8 9]);
Gd = absorbDelay(Gd);

end
