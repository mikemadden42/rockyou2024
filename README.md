# rockyou2024
Quickly search the rockyou2024.txt file, which is ~ 150 GB.

> **This project is for research and educational purposes only.**
> 
This is a C++ port based on this original C project:
- https://github.com/elithaxxor/RockYou2024_Reader

The b3sum for the rockyou2024.txt file can be found here:
- rockyou2024.zip.b3sum

NOTE: Make sure you have the full zip and txt file available before you run the `ln` command or run the tool. The files are very large so they are not contained in the git repository.

```bash
ls -lh rockyou2024.txt rockyou2024.zip | sd 'mjm' 'hulk'
-rw-rw-r-- 1 hulk hulk 146G Jun 25 19:20 rockyou2024.txt
-rw-r--r-- 1 hulk hulk  46G Jul  6 17:56 rockyou2024.zip
```

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
