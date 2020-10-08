function [Gd]=makeACMdl(t1m0, Wsun, tac, Ts, A0, B3, B2, B1s, Blpv, BA, C2, C1, D0)

B1 = B1s .* ([t1m0 Wsun tac 1] * Blpv.');

A = A0 + B3*B2*BA*C2;
B = B3*B2*B1;
C = C1\C2;
D = D0;

G = ss(A, B, C, D, ...
    'InputName', {'ac1', 'ac2', 'ak1', 'ak2', 'f012', 'cur', 't0d', 'w0', 'w1', 'w2'}, ...
    'OutputName', {'t1', 't2', 'tac'});

Gd = c2d(G, Ts);
Gd = setmpcsignals(Gd, 'MV', [1 2 3 4 5 6], 'MD', [7 8 9 10], 'MO', [1 2], 'UO', 3);

end
