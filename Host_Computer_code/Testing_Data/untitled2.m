figure;
sampling_rate = 500e3; % Sampling rate of 1 GHz
num_samples = length(i); % Determine the number of samples in i
time_vector = (0:num_samples-1) / sampling_rate; % Create time vector in seconds

plot(time_vector, i); % Plot the data with time on the x-axis and i on the y-axis

xlabel('Time (s)'); % Label the x-axis as 'Time' with units seconds
ylabel('Voltage (V)'); % Label the y-axis as 'Voltage' with units volts
title('Waveform Plot'); % Optional: add a title
