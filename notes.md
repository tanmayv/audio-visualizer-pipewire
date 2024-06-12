# Audio Visualizer in CPP

- Get the system audio stream and microphone audio stream.
- I will be using pipewire for as this is what is use to manage audio and video in my arch linux machine.
- Get into details of sinks/sources/nodes.

- Raw Buffer -> amplitude at particular point of time. The value on this buffer would depend on the source's number of channels and sample rate.
- Once we have the combined audio wave. We are going to split that into individual sine/cos components using FFT.


