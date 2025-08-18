# PROTOTYPE
 ### Unsual behavior Reasons in Results
 **Why Does It Print Even After Timeout?**
 ```Type something (time out 3s). Crtl+D to exit.
hello[timeout]

you typed : hello
[timeout]
tanish[timeout]
shivhare[timeout]

you typed : tanishshivhare
[timeout]
EOF or error;exiting...```

Here’s the key point: Terminals use line-buffered input in canonical mode by default.
That means:

1) While you type, your characters are not sent to the program immediately.
They sit inside the kernel’s terminal line buffer (you can think of it as: the OS “keeps” your keystrokes).

2) Input is only actually sent to the program when you press Enter (newline), or when a special key like EOF (Ctrl+D) is used.

3) Because of this,If select() times out before you press Enter, it sees no data ready → prints [Time out].
But the partially typed stuff is still waiting in the terminal buffer from the kernel’s side.
In the next iteration, if you finally press Enter, the kernel flushes that whole line to the program → select() now instantly reports “ready” → read() gets the entire line, even though you started typing it during the timeout period.
