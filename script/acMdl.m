%% The LPV matrixes

kt = 1250/(2100*8e-6/1e-3*0.5);

% [ts1 ts2 tac td1 td2] -> d/dt [ts1;ts2;tac;td1;td2]
A0 = [
    0   0     1e-4  0       0
    0   -4e-3 0     0       0
    0   0     -1e-3 0       0
    0   0     0     -0.4e-3 0
    0   0     0     0       -7e-3
    ];
% [Wac W1+W2+W01+W012 W2-W1-W01 Wx] -> d/dt [ts1;ts2;tac;td1;td2]
B3 = [
    0    0.14e-6 0       0
    0    0       0       2e-6
    4e-6 0       0       0
    0    0       0.38e-6 0
    0    0       0       -10e-6
    ];
% [Wac W1 W2 W01 W012 Wx] -> [Wac W1+W2+W01+W012 W2-W1-W01 Wx]
B2 = [
    1  0  0  0  0  0
    0  +1 +1 +1 +1 0
    0  -1 +1 -1 0  0
    0  0  0  0  0  +1
    ];
% [ac1 ac2 ak1 ak2 f012 cur t0d w0 w1 w2] -> [Wac W1 W2 W01 W012 Wx]
%   1    1  tac tac t1m0       Wsun-50  1   1    0   0
B1s = [
    2100 2100 0  0  0          0        0   0    0   0
    200  0    kt 0  0          0        0   0    200 0
    0    200  0  kt 0          0        0   1100 0   300
    0    0    0  0  -350       1        0   0    0   0
    0    0    0  0  0          0        155 0    0   0
    0    0    0  0  -140       0        0   0    0   0
    ];
Blpv = [
    0 0 0 1
    0 0 0 1
    0 0 1 0
    0 0 1 0
    1 0 0 0
    0 1 0 0
    0 0 0 1
    0 0 0 1
    0 0 0 1
    0 0 0 1
    ];
% [ts td tac] -> [Wac W1 W2 W01 W012 Wx]
BA = [
    0 0 0
    0 0 0
    0 0 0
    0 0 0
    -155 0 0
    0 0 0
    ];
% [ts1 ts2 tac td1 td2] -> [ts td tac]
C2 = [
    1 1 0 0 0
    0 0 0 1 1
    0 0 1 0 0
    ];
% [t1 t2 tac] -> [ts td tac]
C1 = [
    0.25 0.75 0
    -1 1 0
    0 0 1
    ];
% [ac1 ac2 ak1 ak2 f012 cur t0d w0 w1 w2] -> [t1 t2]
D0 = [
    0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0
    ];

fmakeACMdl = @(t1m0, Wsun, tac, Ts) makeACMdl(t1m0, Wsun, tac, Ts, ...
    A0, B3, B2, B1s, Blpv, BA, C2, C1, D0);

%% The LPV model

Ts = 10;
t1m0s = [-20 40];
Wsuns = [0 1600];
tacs = [-16.8 16.8];
Gds = drss(5,3,10,length(t1m0s),length(Wsuns),length(tacs));
for i = 1:length(t1m0s)
    for j = 1:length(Wsuns)
        for k = 1:length(tacs)
            Gds(:,:,i,j,k) = fmakeACMdl(t1m0s(i), Wsuns(j), tacs(k), Ts);
        end
    end
end
[t1m0sg,Wsunsg,tacsg] = ndgrid(t1m0s, Wsuns, tacs);
Gds.SamplingGrid = struct('t1m0', t1m0sg, 'Wsun', Wsunsg, 'tac', tacsg);

Gdsam = mpc(fmakeACMdl(10, 60, 0, Ts), Ts, 10, 2, struct(...
    'ManipulatedVariables',     [0.15 0.15 0.001 0.001 0.002 0    ], ...
    'ManipulatedVariablesRate', [0    0    0     0     0.015 0.012]));
conE = [
    -1 0  0  0  0  0
    +1 0  0  0  0  0
    0  -1 0  0  0  0
    0  +1 0  0  0  0
    -1 -1 0  0  0  0
    +1 +1 0  0  0  0
    0  0  -1 0  0  0
    0  0  +1 0  0  0
    0  0  0  -1 0  0
    0  0  0  +1 0  0
    0  0  +1 +1 0  0
    0  0  0  0  -1 0
    0  0  0  0  +1 0
    0  0  0  0  0  -1
    0  0  0  0  0  +1
    ];
conG = [
    2; 2
    2; 2
    2; 2
    -(0.4-0.08); 0.9-0.08
    -(0.1-0.08); 0.6-0.08
    1
    0; 1
    0; 1
    ];
conV = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
setconstraint(Gdsam, conE, [], conG, conV);

%% The t->t filter

Fs = 5;  % Sampling Frequency
N     = 30;   % Order
Fpass = 0.2;  % Passband Frequency
Fstop = 0.3;  % Stopband Frequency
Wpass = 1;    % Passband Weight
Wstop = 10;   % Stopband Weight
dens  = 20;   % Density Factor
% Calculate the coefficients using the FIRPM function.
b  = firpm(N, [0 Fpass Fstop Fs/2]/(Fs/2), [1 1 0 0], [Wpass Wstop], ...
    {dens});
Hd = dsp.FIRFilter('Numerator', b);
t12filt = Hd.Numerator ./ sum(Hd.Numerator);

%% The t0->t0d filter

Fs = 1/60/10;     % Every 10 minutes
N  = 34;          % 2.77hr delay
Fc = 0.5/60/60;   % Cutoff Frequency
TM = 'Bandwidth'; % Transition Mode
BW = Fc*2;        % Bandwidth
DT = 'Normal';    % Design Type
win = flattopwin(N+1);
b  = firrcos(N, Fc/(Fs/2), BW/(Fs/2), 2, TM, DT, [], win);
Hd = dsp.FIRFilter('Numerator', b);
t0filt = Hd.Numerator ./ sum(Hd.Numerator);
[fA,fB,fC,fD] = ss(Hd);
t0filts = ss(fA, fB, fC, fD, 1/Fs);
