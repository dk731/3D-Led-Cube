<div id="top"></div>

<div align="center">
    <img src="https://cube.qwe.me/img/black_circle_logo.svg" alt="Logo" width="15%" height="15%">
    <h1 align="center">3D Led Display</h1>
    <p align="center">
        <a href="https://docs.cube.qwe.me/"><strong>Explore the docs »</strong></a>
        <br/>
    </p>
</div>

## About The Project

If you ever asked yourself, what if monitors could display actual 3D visual output without projecting them on a 2D plane? This project attempts to show how such a device could look like.

<br/>

### Hardware

Following display realization is made up of small (_3 x 3 x 1_ mm) 3 colored led's [SK6812](https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf), which allows control of each pixel independently and prevent animations from any flickering (no multiplexers are used). Total cube dimensions are _460 x 460 x 460_ with resolution _16 x 16 x 16_ voxels. About 20k soldering points had to be connected during assembly, and ~_0.5_ km of thin copper wire was used. MAX refresh rate is ~110 frames per second with 300 W average power consumption. The Control unit of LEDs is Raspberry PI being more precise its SMI device that gives the ability to produce a very precise PWM (_800kHZ_) signal on 16 pins at the same time ([very nice blog about SMI](https://iosoft.blog/2020/07/16/raspberry-pi-smi/)).

### Software

To produce complex output while executing drawing calls in no time, the library's core is written in _C/C++_ with OpenGL graphical acceleration for even better performance. All speed and feature richness are wrapped to _Python_ modules, which gives an end coder a fast and easy-to-use tool that is precompiled and ready to use almost on all major computer systems. As in theory, this method seemed to be easy to implement and work. In practice connecting everything would produce a volatile result with periodical Arduino freezes.

<p align="right">(<a href="#top">back to top</a>)</p>

## Want To Try It Yourself?

As mentioned before, the display can be driven by a straightforward Python script so everyone interested can try to write his animation and even run it on an actual device (to run your code on real cube follow for upcoming events here). More information about writing your animation in this [doc](https://docs.cube.qwe.me/)

<br/>

## Upcoming Events

A small competition event is currently scheduled for the _**3rd-5th December**_. More details can be found on our [Facebook page](https://www.facebook.com/events/1019178698930725/?ref=newsfeed) or [here](https://trycubic.devpost.com/)

<br/>

## Development Process

Due to a general lack of knowledge at the beginning of the project, many iterations were made to get to where it is right now.

<br/>

### Raspberry GPIO's

The first design of the whole system could not be more straightforward, it was planned that just Raspberry PI board with its 40 GPIO pins would be more than enough, but almost immediately after the first tests, it was apparent that it wouldn't be able to work with the required precision. So a new approach should be used.

<br/>

### Arduino Towers

After some thinking, using the Arduino board appeared to solve the problem, as it had a ready driver and was very easy to use. This was planned to work in the following way. The image would render on the Raspberry PI board. Raspberry board would send all needed data to many Arduino boards using I2C, which later would independently send this data to LEDs. After implementing all hardware and Software, it appeared to work very unstably. For some reason, Arduino's, from time to time, would randomly crash and no longer respond.

<img src="https://i.ibb.co/x6Vr3jY/photo-2021-11-15-17-24-06.jpg" width="25%">

<br/>

#### Single ATMega board

After some additional research, microchips used in Arduino but with faster oscillators would be just enough to make these LEDs work. Custom LEDs driver was written in pure ASM due to timing restrictions (to hit needed speed, not even one CPU was allowed to be wasted). After redoing almost the whole project 3rd time and running the entire system again, the test result showed considerable stability improvements. Still, one key number was disappointing, and that is the max refresh rate, as now we could use only one I2C channel. It was throttling the whole system to only ~40FPS.

<img src="https://i.ibb.co/bQP177J/photo-2021-11-15-17-24-06-2.jpg" width="25%">

### Raspberry SMI

As the potential max refresh rate could be >100 per second, another solution had to be developed. After a long period of researching different approaches, this [article about Raspberry PI SMI perephiral](https://iosoft.blog/2020/07/16/raspberry-pi-smi/) was discovered. Mentioned SMI device allows writing a significant amount of data with high precision on 16 GPIO's at the same time directly from Raspberry board. So everything had to be re-done 4th time. A simple PCB with a level shifter and some indicator lights was designed and tested. This appeared to work as expected.

<img src="https://i.ibb.co/bJzXKJt/photo-2021-11-19-16-39-26.jpg" width="25%">

<p align="right">(<a href="#top">back to top</a>)</p>

## Contributions

- Dmitry Kaidalov - help with soldering, general design tips, and Hackathon Prize Sponsor
- Anna Kacane - logo design
- Anastasija Sirotkina - help with soldering
- Tom Kaidalov - help with resin works and Hackaton PC station provide

## Additional Tools

For the project were also created additional web pages with docs, emulator, and submission platform:

- [Cube Doc](https://github.com/dk731/cube-doc) - documention page (_[docs.cube.qwe.me](https://docs.cube.qwe.me/)_)
- [Cube Emulator](https://github.com/dk731/cube-emulator) - emulator page (_[sim.cube.qwe.me](https://sim.cube.qwe.me/)_)
- [Code Submition](https://github.com/dk731/cube-submit) - submition platform (_[cube.qwe.me](https://cube.qwe.me/)_)

## Contact

demid.kaidalov@gmail.com

## License

Distributed under the MIT License. See `LICENSE.md` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>
