import struct
from Crypto.Cipher import AES
import os
_root=os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..'))
d=open(os.path.join(_root,'iso_extract','default.xex'),'rb').read()
hs=struct.unpack('>I',d[8:12])[0]; so=struct.unpack('>I',d[16:20])[0]
RK=bytes([0x20,0xB1,0x85,0xA5,0x9D,0x28,0xFD,0xC3,0x40,0x58,0x3F,0xBB,0x08,0x96,0xBF,0x91])
key=AES.new(RK,AES.MODE_CBC,bytes(16)).decrypt(d[so+0x150:so+0x160])
body=d[hs:]; body=body[:len(body)//16*16]
dec=AES.new(key,AES.MODE_CBC,bytes(16)).decrypt(body)
cnt=struct.unpack('>I',d[20:24])[0]
for i in range(cnt):
    k,v=struct.unpack('>II',d[24+8*i:32+8*i])
    if k==0x3FF: ff=v
info,enc,comp=struct.unpack('>IHH',d[ff:ff+8])
n=info//8-1; out=bytearray(); p=0
for i in range(n):
    ds,zs=struct.unpack('>II',d[ff+8+8*i:ff+16+8*i])
    out+=dec[p:p+ds]; p+=ds; out+=bytes(zs)
open(os.path.join(_root,'analysis','image.bin'),'wb').write(out)
print('image bytes',hex(len(out)),'blocks',n,'MZ?',out[:2])
