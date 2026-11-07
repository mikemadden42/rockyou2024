# rockyou2024
Quickly search the rockyou2024.txt file, which is ~ 150 GB.

> **This project is for research and educational purposes only.**
> 
This is a C++ port based on this original C project:
- https://github.com/elithaxxor/RockYou2024_Reader

The b3sum for the rockyou2024.zip file can be found here:
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

You can also pass the filename as an argument. The result goes to stdout, while the prompts, progress, and timing go to stderr, so a script can capture just the result:

```bash
printf '%s\n' 'Hunter02' | ./search input.txt 2>/dev/null
Occurrences of keyword "Hunter02": 316
```

Add `--exact` to count only lines that are exactly the keyword, which answers whether a specific password is in the list. Lines may end in `\n` or `\r\n`:

```bash
printf '%s\n' 'Hunter02' | ./search --exact input.txt 2>/dev/null
```

The keyword is always read from stdin, so it doesn't end up in your shell history or `ps` output.

The time above was measured before the read loop was rewritten. Your time will depend mostly on your disk.

The search matches substrings, so `Hunter02` also counts `xHunter02!`. The result is the number of occurrences, not the number of matching lines, and matches are case-sensitive.

Run the tests from the `build` directory:

```bash
ctest --output-on-failure
```
