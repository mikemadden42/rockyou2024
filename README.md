# rockyou2024
Quickly search the rockyou2024.txt file, which is ~ 150 GB.

> **This project is for research and educational purposes only.**
> 
This is a C++ port based on this original C project:
- https://github.com/elithaxxor/RockYou2024_Reader

The b3sum for the rockyou2024.txt file can be found here:
- rockyou2024.zip.b3sum

Build & run the project with these steps.  You will need a C++ compiler, cmake, & ninja.

```bash
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja

ln -s ../rockyou2024.txt input.txt

./search
Enter the keyword to search: Hunter02
Enter the filename to search in: input.txt
Occurrences of keyword "Hunter02": 316
Search complete. Time taken: 517.751 seconds
```
