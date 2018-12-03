# Philosophers problem in C++ with PThreads

https://en.wikipedia.org/wiki/Dining_philosophers_problem

My program allows you to input as the first parameter the number of times to eat.
Also as the second parameter you can enter how many philosophers are at the table.

Odd numbered philosophers first pick up the chopstick to the left
and even philosophers first pick up the chopstick to the right.
This makes sure that two philosophers sitting next to each other aren't holding just one chopstick.
I used mutex locking to make sure that a thread successfully labels a chopstick as picked up
so another thread can't pick it up, too.

Philosophers can be in 4 different states: thinking, hungry, eating, and full.