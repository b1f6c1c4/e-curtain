%% The LPV model

Ts = 10;
t1m0s = [-20 40];
Wsuns = [0 1600];
Gds = drss(4,2,8,length(t1m0s),length(Wsuns));
for i = 1:length(t1m0s)
    for j = 1:length(Wsuns)
        Gds(:,:,i,j) = makeACMdl(t1m0s(i), Wsuns(j), Ts);
    end
end
[t1m0sg,Wsunsg] = ndgrid(t1m0s, Wsuns);
Gds.SamplingGrid = struct('t1m0', t1m0sg, 'Wsun', Wsunsg);

Gdsam = mpc(makeACMdl(10, 60, Ts), Ts, 10, 2, struct(...
    'ManipulatedVariables', [0.1 0.1 0.002 0], ...
    'ManipulatedVariablesRate', [0 0 0.02 0.01]));
conE = [
    1 1 0 0
    -1 -1 0 0
    1 -1 0 0
    -1 1 0 0
    1 0 0 0
    -1 0 0 0
    0 1 0 0
    0 -1 0 0
    0 0 1 0
    0 0 -1 0
    0 0 0 1
    0 0 0 -1
    ];
conG = [2; 2; 2; 2; 1.8; 1.8; 1.2; 1.2; 1; 0; 1; 0];
conV = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
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

%%

% Ts = 30;
% 
% m = mpc(Gd, Ts, 5, 2, struct(...
%     'ManipulatedVariables', [0.1 0.1 0.002 0], ...
%     'ManipulatedVariablesRate', [0 0 0.02 0], ...
%     'OutputVariables', [3 1]));

%%

% [configData,stateData,onlineData] = getCodeGenerationData(m);
% 
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
%     onlineData.signals.ref = [0 -2];
%     onlineData.weights.y = [1 3];
%     % Update constraints in online data.
%     % Compute control actions.
%     [u,statedata] = mpcmoveCodeGeneration(configData,stateData,onlineData);
%     % Update and store plant state.
%     ac1q = u(1); ac2q = u(2);
%     if ac1q*ac2q <= 0
%         if abs(ac1q) > ac2q
%             ac2q = 0;
%         else
%             ac1q = 0;
%         end
%     else
%         acq = ac1q + ac2q;
%         if ac1q / ac2q < 0.4/0.6
%             ac1q = acq * 0.4;
%             ac2q = acq * 0.6;
%         elseif ac1q / ac2q > 0.9/0.1
%             ac1q = acq * 0.9;
%             ac2q = acq * 0.1;
%         end
%     end
%     u = [ac1q;ac2q;u(3)];
%     x = Gd.A*x + Gd.B*[u;-10;1;0;3];
%     uCodeGen = [uCodeGen u];
% end
% 
% onlineData.weights.y = [1 1];
% onlineData.limits.umax = [1;1;1];
% 
% func = 'mpcmoveCodeGeneration';
% funcOutput = 'dumbac';
% Cfg = coder.config('lib');
% Cfg.DynamicMemoryAllocation = 'off';
% %codegen('-d', 'libdumbac', '-c', '-config',Cfg,func,'-o',funcOutput,'-args',...
% %    {coder.Constant(configData),stateData,onlineData});

%%

%sim(m, T/Ts, [-1 0], [-10 1 0 0]);

%%

% figure(3);
% subplot(2,1,1);
% plot((0:ct-1).*Ts, yCodeGen.');
% grid on;
% subplot(2,1,2);
% grid on;
% plot((0:ct-1).*Ts, uCodeGen.');
