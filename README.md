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

Configure with `-DSEARCH_SLOW_TESTS=ON` to also run the count overflow test, which scans over 2 GB.

## How useful is the list?

RockYou2024 was posted to a cybercrime forum in July 2024 with a claim of about 9.9 billion passwords. Published analyses disagree with that figure as a measure of real passwords, and they differ on how much value the list has.

- [Specops](https://specopssoft.com/blog/rockyou2024-analysis-password-leak/) examined the 1,489,515,500 records added since RockYou2021. It removed 873,375,026 records as common hash types, which left 616,140,474. It called the dataset "mostly garbage data" and said it was too large to be of realistic use in cracking a given hash.
- [Cyberint](https://cyberint.com/?p=33077) reviewed the file and estimated that 1 to 1.5 billion entries are usable. It described credential stuffing with the list as impractical and hash cracking as the more probable use.

These findings have limits. The Specops breakdown covers only the records added since 2021, not the whole file. Cyberint's figure is an estimate without a published breakdown. Neither analysis reports how many real hashes the list cracks, and both come from security vendors.

This project takes no position on the list's value. It's a tool for searching the file, and its results don't show whether an entry was a real password.
