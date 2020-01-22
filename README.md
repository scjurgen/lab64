

###Requirements:

- C++ compiler 
- Cmake
- ImageMagick
- nlohmann json (json-devel on some linux distros)

This works nicely on mac (use homebrew for the missing stuff, you will not regret it) and linux. 

Sorry, no Windows solution.


###Build

create the build folder run cmake and make

```bash
mkdir bild
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
###Run

Smallsize example
```bash
./LabNew -j ./configs/blue-velvet.json -w 800 -h 600 -o bright_n --oversample 1 -r 2
```
Retina display with oversampling

```bash
./LabNew -j ./configs/dark-age.json -w 5120 -h 2880 -o bright_n --oversample 4 -r 2
```


