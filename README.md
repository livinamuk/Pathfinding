# A* Pathfinding

![Image](https://www.principiaprogrammatica.com/dump/astar.jpg)

This is a sandbox I wrote to implement pathfinding in for another project. It uses
the A* algorithm plus a string pulling post processing phase. It's not the shortest possible pulled string but I'll be recalcuating paths most frames and that'll pull it tight over time, plus it's fast.

The codebase is a little convoluted because it's built atop Hell Engine with everything stripped out besides the functionality to render 2D UI, but if you're curious how it works, all the relevant code is in Core/Pathfinding.h/.cpp.


```
CONTROLS:
N: New map
L: Load map
S: Save map
1: Place start
2: Place destination
Left Mouse: Add wall
Right mouse: Remove wall
Space: Find path
D: Toggle slow mode
W: Smooth path (hold)
A: Smooth path (press)
G: fullscreen
```