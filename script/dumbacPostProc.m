fileID = fopen('dumbac2.log.bin', 'r', 'ieee-le');
format = 'double';
Data = fread(fileID, Inf, format);
fclose(fileID);
Data = reshape(Data, 10, []).';
Data(isnan(sum(Data, 2)),:) = [];
Data = Data - [Data(1,1) zeros(1,size(Data,2)-1)];
Data = Data.*[1e-9 ones(1,size(Data,2)-1)];
clf;
subplot(3,2,1);
plot(Data(:,1), Data(:,[2 7 8]));
grid on;
subplot(3,2,2);
plot(Data(:,1), Data(:,[3 9 10]));
grid on;
subplot(3,2,3);
plot(Data(:,1), Data(:,[4 5 6]));
grid on;

t = (0:5:floor(Data(end,1))).';

t0 = interp1(Data(:,1), Data(:,2), t);
h0 = interp1(Data(:,1), Data(:,3), t);
f012 = interp1(Data(:,1), Data(:,4), t);
f12 = interp1(Data(:,1), Data(:,5), t);
ac = interp1(Data(:,1), Data(:,6), t);

ac1 = ac.*(f12 + 0.3);
ac2 = ac.*((2-f12) + 0.3);

t1 = interp1(Data(:,1), Data(:,7), t);
t2 = interp1(Data(:,1), Data(:,8), t);
h1 = interp1(Data(:,1), Data(:,9), t);
h2 = interp1(Data(:,1), Data(:,10), t);

ts = 0.1.*t1 + 0.9.*t2;
subplot(3,2,4);
plotyy(t, ts, t, [0.4.*ac, f12]);
grid on;
subplot(3,2,5);
plotyy(t, t2 - t1, t, [f12.*ac./3, f12, 0.4.*f012]);
grid on;
subplot(3,2,6);
plotyy(t, t1, t, t2);
grid on;

% id = iddata(y, x, 1, ...
%    'InputName', {'t0', 'h0', 'f012', 'f12', 'ac'}, ...
%    'OutputName', {'t1', 'h1', 't2', 'h2'});
% idt1 = iddata(t1, [f012, ac1], 5, ...
%     'InputName', {'f012', 'ac1'}, ...
%     'OutputName', {'t1'});
% idt2 = iddata(t2, [f012, ac2], 5, ...
%     'InputName', {'f012', 'ac2'}, ...
%     'OutputName', {'t2'});
% idt1t2 = iddata([t1 t2], [f012, ac1, ac2, f12], 5, ...
%     'InputName', {'f012', 'ac1', 'ac2', 'f12'}, ...
%     'OutputName', {'t1', 't2'});
% x = timeseries(x, t, 'Name', ['t0', 'h0', 'f012', 'f12', 'ac']);
% y = timeseries(y, t, 'Name', ['t1', 'h1', 't2', 'h2']);
