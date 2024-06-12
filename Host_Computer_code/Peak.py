#!/usr/bin/env python
# coding: utf-8

# In[7]:


import scipy.io
import numpy as np

trans_data = scipy.io.loadmat('/Users/jc/Desktop/trans.mat')
scan_data = scipy.io.loadmat('/Users/jc/Desktop/scan.mat')

# trans = trans_data['trans']
# scan = scan_data['scan']

trans, scan


# In[15]:


# import numpy as np

# class DipLocator:
#     def __init__(self, trans, scan):
#         self.trans = trans
#         self.scan = scan

#     def find_dips(self):
#         dips = {}
#         for i in range(1, len(self.trans) - 1):
#             if self.trans[i] < self.trans[i - 1] and self.trans[i] < self.trans[i + 1]:
#                 peak_left = self.scan[i - 1]
#                 peak = self.scan[i]
#                 peak_right = self.scan[i + 1]
#                 dips[i] = (peak_left, peak, peak_right, self.trans[i])
#         return dips

# trans = trans_data['i'].flatten()
# scan = scan_data['j'].flatten()

# dip_locator = DipLocator(trans, scan)

# dips = dip_locator.find_dips()
# dips


# In[16]:


# # locate by nearest neighbours
# import numpy as np

# class DipLocator:
#     def __init__(self, trans, scan, min_diff=0):
#         self.trans = trans
#         self.scan = scan
#         self.min_diff = min_diff

#     def find_dips(self):
#         dips = {}
#         for i in range(1, len(self.trans) - 1):
#             if self.trans[i] < self.trans[i - 1] and self.trans[i] < self.trans[i + 1]:
#                 if (self.trans[i - 1] - self.trans[i] > self.min_diff and
#                     self.trans[i + 1] - self.trans[i] > self.min_diff):
#                     dips[i] = {
#                         "scan_left": self.scan[i - 1],
#                         "scan_peak": self.scan[i],
#                         "scan_right": self.scan[i + 1],
#                         "trans_value": self.trans[i]
#                     }
#         return dips

# trans = trans_data['i'].flatten()
# scan = scan_data['j'].flatten()

# min_diff = 0.01

# dip_locator = DipLocator(trans, scan, min_diff)

# dips = dip_locator.find_dips()
# dips


# In[19]:


import numpy as np

class DipLocator:
    def __init__(self, trans, scan, min_diff=0, window_size=10):
        self.trans = trans
        self.scan = scan
        self.min_diff = min_diff
        self.window_size = window_size

    def find_dips(self):
        dips = {}
        half_window = self.window_size // 2
        for i in range(half_window, len(self.trans) - half_window):
            if self.trans[i] < min(self.trans[i - half_window:i]) and self.trans[i] < min(self.trans[i + 1:i + 1 + half_window]):
                left_diff = max(self.trans[i - half_window:i]) - self.trans[i]
                right_diff = max(self.trans[i + 1:i + 1 + half_window]) - self.trans[i]
                if left_diff > self.min_diff and right_diff > self.min_diff:
                    left_index = np.argmax(self.trans[i - half_window:i]) + (i - half_window)
                    right_index = np.argmax(self.trans[i + 1:i + 1 + half_window]) + (i + 1)
                    dips[i] = {
                        "scan_left": self.scan[left_index],
                        "scan_peak": self.scan[i],
                        "scan_right": self.scan[right_index],
                        "trans_value": self.trans[i]
                    }
        return dips

trans = trans_data['i'].flatten()
scan = scan_data['j'].flatten()

min_diff = 0.01  
window_size = 30

dip_locator = DipLocator(trans, scan, min_diff, window_size)

dips = dip_locator.find_dips()
dips


# In[ ]:




