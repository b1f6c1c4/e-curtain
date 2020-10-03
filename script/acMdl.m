%% The continuous-time model

% Assumptions:
W01ass = 10; % t1-t0 = 10

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
    1 1 1 1 0
    -1 1 -1 0 0
    0 0 0 0 1
    ];
% [ac1 ac2 f012 t0d] -> [Wac1 Wac2 W01 W012 Wx]
B1 = [
    2500 0 0 0
    0 2500 0 0
    0 0 -350*W01ass 0
    0 0 0 155
    0 0 -1000 0
    ];
% [ts td] -> [Wac1 Wac2 W01 W012 Wx]
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
% [ac1 ac2 f012 t0d] -> [t1 t2]
D0 = [
    0 0 0 0
    0 0 0 0
    ];
% [ac acd f012 t0d] -> [ac1 ac2 f012 t0d]
X = [
    0.5 -0.5 0 0
    0.5 0.5 0 0
    0 0 1 0
    0 0 0 1
    ];

A = A0 + B3*B2*BA*C2;
B = B3*B2*B1; % *X
C = C1\C2;
D = D0; % *X

G = ss(A, B, C, D, ...
    'InputName', {'ac1', 'ac2', 'f012', 't0d'}, 'OutputName', {'t1', 't2'});

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

%%

Ts = 30;

Gd = c2d(G, Ts);
Gd = setmpcsignals(Gd, 'MV', [1 2 3], 'MD', 4);
m = mpc(Gd, Ts, 5, [], struct(...
    'ManipulatedVariables', [0.1 0.1 0], ...
    'ManipulatedVariablesRate', [0 0 0.02], ...
    'OutputVariables', [1 100]));
conE = [
    1 1 0
    -1 -1 0
    1 -1 0
    -1 1 0
    0 0 1
    0 0 -1
    ];
conG = [2; 2; 2; 2; 1; 0];
conV = [0; 0; 0; 0; 0; 0];
setconstraint(m, conE, [], conG, conV);

%%

[configData,stateData,onlineData] = getCodeGenerationData(m);

% yCodeGen = [];
% uCodeGen = [];
% x = [0;0;0;0];
% T = 1000;
%     
% for ct = 1:floor(T/Ts)
%     % Update and store plant output.
%     y = Gd.C*x;
%     yCodeGen = [yCodeGen y];
%     % Update measured output in online data.
%     onlineData.signals.ym = y;
%     onlineData.signals.md = 0;
%     % Update reference in online data.
%     onlineData.signals.ref = [-1 0];
%     %onlineData.weights.y = [0.1 2];
%     % Update constraints in online data.
%     % Compute control actions.
%     [u,statedata] = mpcmoveCodeGeneration(configData,stateData,onlineData);
%     % Update and store plant state.
%     x = Gd.A*x + Gd.B*[u;0];
%     uCodeGen = [uCodeGen u];
% end

onlineData.weights.y = [1 1];
onlineData.limits.umax = [1;1;1];

func = 'mpcmoveCodeGeneration';
funcOutput = 'dumbac';
Cfg = coder.config('lib');
Cfg.DynamicMemoryAllocation = 'off';
codegen('-d', 'libdumbac', '-c', '-config',Cfg,func,'-o',funcOutput,'-args',...
    {coder.Constant(configData),stateData,onlineData});

%%

sim(m, T/Ts, [-1 0], 0);

%%

figure(3);
subplot(2,1,1);
plot(yCodeGen.');
grid on;
subplot(2,1,2);
grid on;
plot(uCodeGen.');
