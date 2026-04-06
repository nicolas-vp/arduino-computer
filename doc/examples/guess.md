# Угадай число

На устройстве загораются индикаторы, пользователю необходимо угадать, что это за число 0-15.


```bas
print "Угадай число"
n=int(rnd*16)
b3=int(n/8)
n8=n-b3*8
b2=int(n8/4)
m4=n8-b2*4
b1=int(m4/2)
b0=m4-b1*2
pimode 13,1
pin 13,b0
pinmode 12,1
pin 12,b1
pinmode 11,1
pin 11,b2
pinmode 10,1
pin 10,b3
input a
if a=n then beep
if a=n then print "Угадал! Молодец!"
if a<>n then print "Не угадал:";n
for t=10 to 13
pin t,0
next t
```