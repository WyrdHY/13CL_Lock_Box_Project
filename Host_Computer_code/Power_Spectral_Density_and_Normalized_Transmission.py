#!/usr/bin/env python
# coding: utf-8

# In[ ]:


import scipy.io
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import welch

# Load the .mat file
mat_data = scipy.io.loadmat(r"C:\Users\cl22N\Downloads\for_alex\05-28-2024\t2.mat")

# Extract the data associated with key 'i'
data = mat_data['i']

# Convert the data to a pandas DataFrame
df = pd.DataFrame(data)

# Define the normalization value
normalization_value = 1.6

# Normalize the data by the given value
normalized_data = (1 - df.iloc[0] / normalization_value) * 100

# Calculate the time interval between each data point
sampling_rate = 15390  # data points per second
time_interval = 1 / sampling_rate

# Create the time array
time = np.arange(0, len(normalized_data) * time_interval, time_interval)

# Plot the normalized data with time on the x-axis
plt.figure(figsize=(14, 7),dpi = 300)
plt.plot(time, normalized_data, label='Normalized Data')
plt.xlabel('Time (s)')
plt.ylabel('Percentage of Light Coupling to Cavity')
plt.title('Normalized Transmission Cavity')
plt.legend()
plt.grid(True)
plt.show()

# Remove the DC component by subtracting the mean of the normalized data
normalized_data_detrended = normalized_data - np.mean(normalized_data)

# Calculate the Power Spectral Density (PSD) using Welch's method
frequencies, psd = welch(normalized_data_detrended, fs=sampling_rate, nperseg=1024)

# Plot the PSD
plt.figure(figsize=(14, 7))
plt.semilogy(frequencies, psd, label='PSD (Detrended)')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Power Spectral Density (units^2/Hz)')
plt.title('Power Spectral Density of Detrended Normalized Transmission Cavity Data')
plt.legend()
plt.grid(True)
plt.show()

