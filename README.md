# Dynamic r-index

A C++ implementation of the **dynamic r-index** and **dynamic FM-index** — space-efficient data structures supporting pattern matching queries and dynamic text updates.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Table of Contents

- [Quick Start](#quick-start)
- [Overview](#overview)
  - [Dynamic r-index](#dynamic-r-index-1)
  - [Dynamic FM-index](#dynamic-fm-index)
  - [Comparison](#comparison-between-dynamic-r-index-and-dynamic-fm-index)
- [Requirements](#requirements)
- [Installation](#installation)
  - [Download](#download)
  - [Install SDSL](#install-sdsl)
  - [Build](#build)
- [Usage](#usage)
  - [Common Tools](#common-tools)
  - [Dynamic r-index](#dynamic-r-index-usage)
  - [Dynamic FM-index](#dynamic-fm-index-usage)
- [API Documentation](#api-documentation)
- [Dependencies](#dependencies)
- [License](#license)
- [References](#references)

---

## Quick Start

```bash
# 1. Install SDSL library (if not already installed)
git clone https://github.com/simongog/sdsl-lite.git
cd sdsl-lite
./install.sh ~/local
cd ..

# 2. Clone and setup
git clone https://github.com/TNishimoto/dynamic_r_index.git
cd dynamic_r_index
git submodule update --init --recursive

# 3. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSDSL_LIBRARY_DIR=~/local/lib \
         -DSDSL_INCLUDE_DIR=~/local/include
make

# 4. Try it out
./build_r_index -i ../examples/ab.txt -o ab.dri
./query -i ab.dri -q ../examples/command.tsv -w result.log
cat result.log
```

> [!NOTE]
> If SDSL is already installed elsewhere, skip step 1 and adjust the paths in step 3.
> See [Installation](#installation) for alternative SDSL locations (e.g., Homebrew).

---

## Overview

### Dynamic r-index

The [dynamic r-index](https://arxiv.org/abs/2504.19482) is a dynamic version of [the r-index](https://dl.acm.org/doi/10.1145/3375890), supporting count and locate queries on an input string $T$ while allowing insertions and deletions.
It can be stored in $O(r \log n)$ bytes, where $r$ is the number of runs in the BWT.

For more details on the r-index, see [the r-index repository](https://github.com/nicolaprezza/r-index).

| Operation               | Time Complexity                          | Description                                    |
| ----------------------- | ---------------------------------------- | ---------------------------------------------- |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                     | Build index from BWT $L[0..n-1]$               |
| insert_string($i$, $P$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Insert string $P[0..m-1]$ at position $i$      |
| delete_string($i$, $m$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Delete $m$ characters starting at position $i$|
| count_query($P$)        | $O(m \log σ \log n)$                     | Count occurrences of pattern $P$               |
| locate_query($P$)       | $O((m + occ) \log σ \log n)$             | Find all positions of pattern $P$              |
| backward_search($P$)    | $O(m \log σ \log n)$                     | Return the SA-interval of $P$                  |

> [!NOTE]  
> - $σ$: alphabet size of $T$
> - $L_{avg}$: average LCP value in the LCP array of $T$
> - Worst-case time for insert/delete: $O((m + L_{max}) \log σ \log n)$

### Dynamic FM-index

The [dynamic FM-index](https://www.sciencedirect.com/science/article/pii/S1570866709000343), proposed by Salson et al., is a dynamic version of [the FM-index](https://en.wikipedia.org/wiki/FM-index).
It can be stored in $O(n \log σ + (n/s) \log n)$ bytes for a tunable parameter $1 \leq s \leq n$.

For more details, see [the original implementation](https://framagit.org/mikaels/dfmi).

| Operation               | Time Complexity                          | Description                                    |
| ----------------------- | ---------------------------------------- | ---------------------------------------------- |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                     | Build index from BWT $L[0..n-1]$               |
| insert_string($i$, $P$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Insert string $P[0..m-1]$ at position $i$      |
| delete_string($i$, $m$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Delete $m$ characters starting at position $i$|
| count_query($P$)        | $O(m \log σ \log n)$                     | Count occurrences of pattern $P$               |
| locate_query($P$)       | $O((m + s \cdot occ) \log σ \log n)$     | Find all positions of pattern $P$              |
| backward_search($P$)    | $O(m \log σ \log n)$                     | Return the SA-interval of $P$                  |

### Comparison between dynamic r-index and dynamic FM-index

| Feature          | Dynamic r-index                      | Dynamic FM-index                         |
| ---------------- | ------------------------------------ | ---------------------------------------- |
| **Space**        | $O(r \log n)$                        | $O(n \log σ + (n/s) \log n)$             |
| **Locate query** | $O((m + occ) \log σ \log n)$         | $O((m + s \cdot occ) \log σ \log n)$     |
| **Best for**     | Highly repetitive texts              | General texts                            |
| **Trade-off**    | Smaller for repetitive data          | Adjustable via parameter $s$             |

**Recommendation:**
- Use **r-index** for highly repetitive texts (e.g., genome collections, versioned documents)
- Use **FM-index** for general-purpose indexing with tunable space/time trade-off

> [!IMPORTANT]
> These time complexities are slightly larger than described in the original papers due to the use of [B-trees](https://github.com/TNishimoto/b_tree_plus_alpha) for performance.

---

## Requirements

- **C++ Compiler**: C++17 or later (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake**: 3.10 or later
- **Make**: GNU Make
- **SDSL Library**: [sdsl-lite](https://github.com/simongog/sdsl-lite)

---

## Installation

### Download

```bash
git clone https://github.com/TNishimoto/dynamic_r_index.git
cd dynamic_r_index
git submodule update --init --recursive
```

### Install SDSL

If you haven't installed SDSL yet:

```bash
git clone https://github.com/simongog/sdsl-lite.git
cd sdsl-lite
./install.sh ~/local
cd ..
```

This installs the library to `~/local/lib` and headers to `~/local/include`.

### Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSDSL_LIBRARY_DIR=~/local/lib \
         -DSDSL_INCLUDE_DIR=~/local/include
make
```

> [!TIP]
> **Alternative SDSL locations:**
> - Homebrew (Apple Silicon): `-DSDSL_LIBRARY_DIR=/opt/homebrew/lib -DSDSL_INCLUDE_DIR=/opt/homebrew/include`
> - Homebrew (Intel Mac): `-DSDSL_LIBRARY_DIR=/usr/local/lib -DSDSL_INCLUDE_DIR=/usr/local/include`

**Generated executables:**

| Executable        | Description                                      |
| ----------------- | ------------------------------------------------ |
| `build_bwt`       | Compute BWT from a text file                     |
| `build_r_index`   | Build dynamic r-index from text or BWT           |
| `build_fm_index`  | Build dynamic FM-index from text or BWT          |
| `print_index`     | Display index information and extract text/BWT   |
| `query`           | Execute queries from a command file              |

---

## Usage

### Common Tools

#### build

Build the dynamic RLSLP representing a given text.

```
options:
  -i, --input_file_path     Input text file path (string)
  -o, --output_file_path    Output data structure file path (string [=])
  -p, --parser              Grammar_parser(restricted_recompression or signature_encoding) (string [=restricted_recompression])
  -s, --seed                Seed (unsigned long long [=0])
  -f, --offline_build       Offline build mode (bool [=0])
  -m, --mode                If fast mode is selected, the built data structure incldues cache for fast LCE and parent queries (fast or standard) (string [=standard])
  -?, --help   
```

> [!NOTE]  
> If --offline_build=0, then construct the DynamicRLSLPString by dynRLSLP::DynamicRLSLPString::offline_build_from_text function. 
> Otherwise, construct the DynamicRLSLPString by dynRLSLP::DynamicRLSLPString::online_build_from_text_file function. 

**Example:**

```bash
./build -i ../examples/short.txt -o short.rr.fa.ds -p restricted_recompression -m fast
```

```
=============RESULT===============
File : ../examples/short.txt
Output : short.rr.fa.ds
Processing Mode : Online
  Statistics (DynamicRLSLPString)
    Mode: Fast
    Left Short Strings (Cache): 21 * 8 bytes
    Right Short Strings (Cache): 21 * 8 bytes
    Ancestor Cache List (Cache): 21 * 16 bytes
    Statistics (DynamicGrammarForLayeredRLSLP): 
      Statistics (GrammarForLayeredRLSLP): 
        Compression Algorithm:  Restricted Recompression
        Text Length:    35
        The level of the derivation tree: 58
        Statistics (DictionaryForLayeredRLSLP): 
          Number of nonterminals:               233
            Number of explicit nonterminals:            21
            Number of implicit nonterminals:            212
          Number of null nonterminals:          0
        [END]
        Statistics (RandomBitDictionary): 
          Seed: 0
          Number of short random bytes: 88
          Number of middle random bytes: 40
          Number of long random bytes: 40
        [END]
      [END]
      Statistics (FastParentDictionary): 45 pointers
        Number of lightweight Pointers: 30
        Number of middleweight Pointers: 0
        Number of heavyweight Pointers: 0
        Number of null Pointers: 15
      [END]
    [END]
  [END]
Excecution time : 0ms [infchars/ms]
Memory footprint: 1616 KB (1 MB)
==================================
```

```bash
./build -i ../examples/short.txt -o short.se.st.ds -p signature_encoding -m standard -f 1
```

```
=============RESULT===============
File : ../examples/short.txt
Output : short.se.st.ds
Processing Mode : Offline
  Statistics (DynamicRLSLPString)
    Mode: Standard
    Left Short Strings (Cache): 0 * 8 bytes
    Right Short Strings (Cache): 0 * 8 bytes
    Ancestor Cache List (Cache): 0 * 16 bytes
    Statistics (DynamicGrammarForLayeredRLSLP): 
      Statistics (GrammarForLayeredRLSLP): 
        Compression Algorithm:  Signature Encoding
        Text Length:    35
        The level of the derivation tree: 8
        Statistics (DictionaryForLayeredRLSLP): 
          Number of nonterminals:               46
            Number of explicit nonterminals:            26
            Number of implicit nonterminals:            20
          Number of null nonterminals:          0
        [END]
        Statistics (RandomBitDictionary): 
          Seed: 0
          Number of short random bytes: 24
          Number of middle random bytes: 40
          Number of long random bytes: 40
        [END]
      [END]
      Statistics (FastParentDictionary): 58 pointers
        Number of lightweight Pointers: 40
        Number of middleweight Pointers: 0
        Number of heavyweight Pointers: 0
        Number of null Pointers: 18
      [END]
    [END]
  [END]
Excecution time : 0ms [infchars/ms]
Memory footprint: 1616 KB (1 MB)
==================================
```


---

### Dynamic r-index Usage

#### build_r_index

Builds the dynamic r-index from a text file or BWT.

```
Options:
  -i, --input_file_path           Input file path (text or BWT)
  -o, --output_file_path          Output index file path (.dri)
  -c, --null_terminated_string    End-of-text character (default: \0)
  -u, --is_bwt                    Set to 1 if input is BWT
  -?, --help                      Show help
```

**Example:**

```bash
# From text
./build_r_index -i ../examples/ab.txt -o ab.dri

# From BWT
./build_bwt -i ../examples/ab.txt -o ab.bwt -c "$"
./build_r_index -i ab.bwt -o ab.dri -u 1
```

#### print_index (r-index)

Displays r-index information and optionally extracts the text/BWT.

```
Options:
  -i, --input_file_path     Input index file path (.dri)
  -o, --output_text_path    Output text file path (optional)
  -b, --output_bwt_path     Output BWT file path (optional)
  -?, --help                Show help
```

**Example:**

```bash
./print_index -i ab.dri
```

```
Statistics(DynamicRIndex):
  Text length:              26
  Text:                     aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:            5
  Alphabet:                 [$, A, B, a, b]
  BWT:                      baaaBBBAAAaaaaaaaaAA$Aaaaa
  The number of runs:       9
```

#### query (r-index)

Executes queries on a dynamic r-index from a TSV command file.

```
Options:
  -i, --input_index_path    Input index file path (.dri)
  -q, --command_file        Command file path (TSV format)
  -w, --log_file            Output log file path
  -o, --output_index_path   Save updated index (optional)
  -?, --help                Show help
```

**Supported Commands:**

| Command      | Param 1 | Param 2 | Description                                |
| ------------ | ------- | ------- | ------------------------------------------ |
| `COUNT`      | P       | -       | Count occurrences of pattern P             |
| `LOCATE`     | P       | -       | Find all positions of pattern P            |
| `LOCATE_SUM` | P       | -       | Sum of all occurrence positions            |
| `INSERT`     | i       | P       | Insert string P at position i              |
| `DELETE`     | i       | m       | Delete m characters starting at position i |
| `PRINT`      | -       | -       | Print current text and BWT                 |

**Example command file** (`command.tsv`):

```
PRINT
COUNT ABA
LOCATE ABA
INSERT 3 bbbb
PRINT
DELETE 3 5
PRINT
```

**Example:**

```bash
./build_r_index -i ../examples/ab.txt -o ab.dri
./query -i ab.dri -q ../examples/command.tsv -w result.log
cat result.log
```

**Sample output** (`result.log`):

```
0	PRINT	Text:	aaaaABAaaaaABAaaaABAaaaab$	BWT:	baaaBBBAAAaaaaaaaaAA$Aaaaa
1	COUNT	The number of occurrences of the given pattern: 	3	Time (microseconds): 	51
2	LOCATE	The occurrences of the given pattern: 	[11, 4, 17]	Time (microseconds): 	57
3	INSERT	Reorder count: 	3	Time (microseconds): 	25
4	PRINT	Text:	aaabbbbaABAaaaaABAaaaABAaaaab$	BWT:	baaaBBBAAAabaaaaAAAa$aaaaabbba
...
```

---

### Dynamic FM-index Usage

#### build_fm_index

Builds the dynamic FM-index from a text file or BWT.

```
Options:
  -i, --input_file_path           Input file path (text or BWT)
  -o, --output_file_path          Output index file path (.dfmi)
  -c, --null_terminated_string    End-of-text character (default: \0)
  -u, --is_bwt                    Set to 1 if input is BWT
  -s, --sampling_interval         SA sampling interval (default: 32)
  -?, --help                      Show help
```

**Example:**

```bash
# Default sampling interval (32)
./build_fm_index -i ../examples/ab.txt -o ab.dfmi

# Custom sampling interval
./build_fm_index -i ../examples/ab.txt -o ab.dfmi -s 8
```

#### print_index (FM-index)

Displays FM-index information and optionally extracts the text/BWT.

```bash
./print_index -i ab.dfmi
```

```
Statistics(DynamicFMIndex):
  Text length:                    26
  Text:                           aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:                  5
  Alphabet:                       [$, A, B, a, b]
  BWT:                            baaaBBBAAAaaaaaaaaAA$Aaaaa
  Sampling interval:              8
  The number of sampled values:   5
```

#### query (FM-index)

Executes queries on a dynamic FM-index from a TSV command file.

The command format is identical to the r-index version.

**Example:**

```bash
./build_fm_index -i ../examples/ab.txt -o ab.dfmi -s 8
./query -i ab.dfmi -q ../examples/command.tsv -w result.log
cat result.log
```

> [!NOTE]
> The `query` command automatically detects whether the input is a dynamic r-index (.dri) or dynamic FM-index (.dfmi).

---

## API Documentation

[Doxygen Documentation](https://TNishimoto.github.io/dynamic_r_index/index.html) *(in preparation)*

---

## Dependencies

This project uses the following libraries:

- [SDSL](https://github.com/simongog/sdsl-lite) — Succinct Data Structure Library
- [STool](https://github.com/TNishimoto/stool) — String utilities
- [B-tree_plus_alpha](https://github.com/TNishimoto/b_tree_plus_alpha) — B-tree implementation

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.  
If you use the library, please cite the following paper:

```
@misc{nishimoto2025dynamicrindexupdatableselfindex,
      title={Dynamic r-index: An Updatable Self-Index in LCP-bounded Time}, 
      author={Takaaki Nishimoto and Yasuo Tabei},
      year={2025},
      eprint={2504.19482},
      archivePrefix={arXiv},
      primaryClass={cs.DS},
      url={https://arxiv.org/abs/2504.19482}, 
}
```

---

## References

- T. Nishimoto, "Dynamic r-index: An Updatable Self-Index in LCP-bounded Time," [arXiv:2504.19482](https://arxiv.org/abs/2504.19482), 2025.
- T. Gagie, G. Navarro, N. Prezza, "Fully Functional Suffix Trees and Optimal Text Searching in BWT-Runs Bounded Space," [ACM DL](https://dl.acm.org/doi/10.1145/3375890), 2020.
- M. Salson et al., "Dynamic extended suffix arrays," [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S1570866709000343), 2010.
