%% The LPV model

Ts = 10;
t1m0s = [-20 40];
Wsuns = [0 1600];
Gds = drss(61,2,9,length(t1m0s),length(Wsuns));
for i = 1:length(t1m0s)
    for j = 1:length(Wsuns)
        Gds(:,:,i,j) = makeACMdl(t1m0s(i), Wsuns(j), Ts);
    end
end
[t1m0sg,Wsunsg] = ndgrid(t1m0s, Wsuns);
Gds.SamplingGrid = struct('t1m0', t1m0sg, 'Wsun', Wsunsg);

Gdsam = mpc(makeACMdl(0, 0, Ts), Ts, 25, 2, struct(...
    'ManipulatedVariables', [0.2 0.3 0.2 0.02 0], ...
    'ManipulatedVariablesRate', [0 0 0.1 0.3 0.2]));
conE = [
    1 0 0 0 0
    -1 0 0 0 0
    0 1 1 0 0
    0 -1 -1 0 0
    0 1 -1 0 0
    0 -1 1 0 0
    0 1 0 0 0
    0 -1 0 0 0
    0 0 1 0 0
    0 0 -1 0 0
    0 0 0 -1 0
    0 0 0 1 0
    0 0 0 0 -1
    0 0 0 0 1
    ];
conG = [3; 0; 2; 2; 2; 2; 1.8; 1.8; 1.2; 1.2; 0; 1; 0; 1];
conV = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
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
