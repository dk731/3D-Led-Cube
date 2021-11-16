<div id="top"></div>

<div align="center">
    <img src="https://trycubic.com/img/black_circle_logo.svg" alt="Logo" width="15%" height="15%">
    <h1 align="center">3D Led Display</h1>
    <p align="center">
        <a href="https://doc.trycubic.com/"><strong>Explore the docs »</strong></a>
        <br/>
    </p>
</div>

## About The Project

If you every asked yourself, what if monitors could display true 3D visual output without projecting them on 2D plane, this project attempts show how such tool could look like.

<br/>

### Hardware

Following display realization is made up from small (_3 x 3 x 1_ mm) 3 colored led's [SK6812](https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf) which allows controll each pixel independenly and prevent animations from any sort of flickering (no multiplexers are used). Total cube dimensions are _460 x 460 x 460_ with resolution _16 x 16 x 16_ voxels. Dueing assembly about 20k soldering points had to be connected and ~_0.5_ km of thin copper wire was used. Max. refreshrate is ~110 frames per second with 300 W avarage power consumption. Control unit of led's is Raspberry PI being more precies its SMI device that gives ability to produce very prices PWM (_800kHZ_) signal on 16 pins at the same time ([very nive blog about SMI](https://iosoft.blog/2020/07/16/raspberry-pi-smi/)).

### Software

To produce complex output while being able to excecute drawing calls in no time, core of the library was written in _C/C++_ with OpenGL graphical acceleration for even better performance. All speed and feature richness is wrapped to _Python_ modules which gives end coder not only fast but also very easy to use tool which is precompiled and ready to use almost on all major computer systems. As in theory this method seemed to be easy to realise and work, in practise connecting everything together would produce very unstable result with pereodical arduino freezes.

<p align="right">(<a href="#top">back to top</a>)</p>

## Want To Try It Yourself?

As mentioned before, display can be driven by a very simple python scripts so everyone who is interested can try to write his own animation and even run it on real device (to run your code on real cube follow for upcomming events here). More information about writing your own animation in this [doc](https://doc.trycubic.com/)

<br/>

## Upcoming Events

Small competition event is currently scheduled for the _**end of November (2021)**_, more details can be found [here](https:://google.com)

<br/>

## Development Process

Due to general lack of knowlage in the beggining of project many itteration were made to get to the point where it is right now.

<br/>

### Raspberry GPIO's

At first design of whole system could not be easier, it was planned that just Raspberry PI board with its 40 GPIO pins will be more than enough, but almost immediatly after first tests it was obvious that it wont be able to work with required precision. So new approach should be used.

<br/>

### Arduino Towers

After some thinking idea of using Arduino board appeard, as it had allready written driver and was very easy to use. This was planned to work in following way, image would render on Raspberry PI board, then Raspberry board would send all needed data to many arduino boards using I2C which later would independetly send this data to leds. After implmenting all hardware and software it appeared to work very unstable, for some reason arduino's from time to time would just randomly crash and no longer respond.

<img src="https://i.ibb.co/x6Vr3jY/photo-2021-11-15-17-24-06.jpg" width="25%">

<br/>

#### Single ATMega board

After some additional research was found that microchips used in arduino but with faster oscillator would be just enough to make this leds work, custom leds driver was written in pure ASM due timming restrictions (to hit needed speed not even 1 cpu was allowd to be wasted). After redoing almost whoole project 3rd time and running whole system again test result showed huge improvements in stability but one key number was disapointing and that is max refresh rate, as now we could use only one I2C chanell it was throtelling whole system to only ~40FPS.

<img src="https://i.ibb.co/bQP177J/photo-2021-11-15-17-24-06-2.jpg" width="25%">

### Raspberry SMI

As potential max refrash rate could be >100 per second, another solution had to be developed, after long time span of researching different approaches, this [article about Raspberry PI SMI perephiral](https://iosoft.blog/2020/07/16/raspberry-pi-smi/) was discovered. Mentioned SMI device allows to write big amount of data with high precision on 16 GPIO's at the same time directly from Raspberry borad, so everythink had to be re-done 4th time, simple PCB with level shifter and some indecational lights was designed and tested. This appered to work as excpected.

<p align="right">(<a href="#top">back to top</a>)</p>

## Contributions

- Dmitry Kaidalov - help with soldering and general design tips
- Anna Kacane - logo design
- Anastasija Sirotkina - help with soldering

## Contact

demid.kaidalov@gmail.com

## License

Distributed under the MIT License. See `LICENSE.md` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>
