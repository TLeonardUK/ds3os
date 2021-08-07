import os, psutil
from aes import AES

# Pin the Python process to the last CPU to measure performance
# Note: this code works for psutil 1.2.x, not 2.x!
cpu_count = psutil.cpu_count()
p = psutil.Process(os.getpid())
proc_list = p.cpu_affinity()
p.cpu_affinity([proc_list[-1]])

# Perform several encryption / decryption operations

random_iv = bytearray(os.urandom(16))
random_key = bytearray(os.urandom(16))

data = bytearray(list(range(256)))
data1 = data[:151]
data2 = data[151:]

# Note: __PROFILE_AES__ must be defined when building the native
# module in order for the print statements below to work
aes_ctr = AES(mode='ctr', key=random_key, iv=random_iv)
result = aes_ctr.encrypt(data1)
if result:
  print('Encrypted data1 in: %5d cycles' % result)
result = aes_ctr.encrypt(data2)
if result:
  print('Encrypted data2 in: %5d cycles' % result)

data_new = data1 + data2
aes_ctr = AES(mode='ctr', key=random_key, iv=random_iv)
result = aes_ctr.decrypt(data_new)
if result:
  print('Decrypted data in:  %5d cycles' % result)

assert data == data_new, "The demo has failed."
