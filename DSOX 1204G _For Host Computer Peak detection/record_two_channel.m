myScope = oscilloscope();
availableResources = resources(myScope);
myScope.Resource = 'USB0::0x2A8D::0x0396::CN61297440::0::INSTR';
connect(myScope);

[i, j] = readWaveform(myScope); % channel 2, 3, 4 are recorded

subplot(2, 1, 1); 
plot(i);
xlabel('Samples');
ylabel('Voltage');
title('Channel i, SPAD');

subplot(2, 1, 2); 
plot(j);
xlabel('Samples');
ylabel('Voltage');
title('Channel j, Gate Control VOltate');


baseFolder = 'C:\\Users\\Dirk\\Documents\\oscilloscope_Measurement';

dateFolder = datestr(now, 'mm-dd-yyyy');

fullPath = fullfile(baseFolder, dateFolder);

if ~exist(fullPath, 'dir')
    mkdir(fullPath);
end

timestamp = datestr(now, 'yyyymmdd_HHMMSS');


filenamei = sprintf('%s\\chi_%s.mat', fullPath, timestamp);
filenamej = sprintf('%s\\chj_%s.mat', fullPath, timestamp);

save(filenamei, 'i');
save(filenamej, 'j');