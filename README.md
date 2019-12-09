# Flocking

## Description
This project is a an attempt at creating Reynold's flocking behaviors in unreal. Currently able to spawn ~500 boids at 60fps, and ~1000 boids at 30fps on my laptop. Flocking variables are calculated in parallel threads. The flocking calculations and obstacle avoidance algorithm are the main performance drainers at the moment.

## Screenshots
![alt text](https://i.imgur.com/8Rb9Ll1.png "Flocking1")
![alt text](https://i.imgur.com/HLmYz5a.jpg "Flocking2")

## Goals
* Add flag for simpler obstacle avoidance (reversal)
* Add Octree for spatial optimization
* Move parallel proccessing to the GPU.
