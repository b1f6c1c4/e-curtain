clf;
human = @(x) 60+170.*(erf((x-10)./8)+1);
ts = 0.25.*t1 + 0.75.*t2;
td = t2 - t1;
h = (t < 3.54e4 | t >= 6e4) + ...
    (t < 3.9e4 | t >= 6e4) + ...
    (t < 4.4e4 | t >= 6.28e4);
Wh2 = 1100+human(38-t2).*(h + 3.*(t >= 6.7e4 & t < 6.9e4));
Wh1 = human(34-t1).*(3-h);
acc = ac;
klb = 0.4;
kub = 0.9;
Wac1 = 2500.*acc.*(klb+(kub-klb).*f12./2);
Wac2 = 2500.*acc.*(1-(klb+(kub-klb).*f12./2));
W1 = Wh1 + Wac1;
W2 = Wh2 + Wac2;
W01 = -350.*(f012).*(t1-t0);

t0d = filter(t0filt, 1, [repmat(t0(1),length(t0filt),1);t0]);
t0d(1:length(t0filt)) = [];
Ws0 = -155.*(ts-t0d);

W012 = Ws0; % + 200.*exp((t - 4.56e4)./5e3-6);
Wx = -150.*(f12.*(1-f012))-140.*(t1-t0).*f012;

tfxxs = ss([0 0; 0 -4e-3], [0.14e-6 0; 0 2e-6], [1 1], [0 0]);
tsE = lsim(c2d(tfxxs, 5), [W1+W2+W01+W012,Wx], t, [ts(1), 0]);
subplot(2,1,1);
plotyy(t, [ts, tsE], t, [W1+W2+W01+W012,Wx]);
grid on;

tfxxd = ss([-0.4e-3 0; 0 -7e-3], [0.38e-6 0; 0 -10e-6], [1 1], [0 0]);
tdE = lsim(c2d(tfxxd, 5), [W2-W1-W01,Wx], t, [td(1), 0]);
subplot(2,1,2);
plotyy(t, [td, tdE], t, [W2-W1-W01,Wx]);
grid on;

idts = iddata(ts, [W1+W2+W01,W012,Wx], 5, ...
    'InputName', {'Ws', 'W012', 'Wx'}, ...
    'OutputName', {'ts'});
idtd = iddata(t2-t1, W2-W1-W01, 5, ...
    'InputName', {'Wd'}, ...
    'OutputName', {'td'});
