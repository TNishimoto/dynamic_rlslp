# Dynamic RLSLP

A C++ implementation of dynamic RLSLPs (run-length encoded straight-line programs).

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Table of Contents

* [Quick Start](#quick-start)
* [Overview](#overview)
* [Requirements](#requirements)
* [Installation](#installation)

  * [Download](#download)
  * [Build](#build)
  * [Generated Executables](#generated-executables)
* [Usage](#usage)

  * [Build Data Structure](#build-data-structure)
  * [Print Data Structure](#print-data-structure)
  * [Query](#query)
* [API Documentation](#api-documentation)
* [Dependencies](#dependencies)
* [License](#license)
* [References](#references)

---

## Quick Start

```bash
# 1. Clone the repository and initialize submodules
git clone https://github.com/TNishimoto/dynamic_rlslp.git
cd dynamic_rlslp
git submodule update --init --recursive

# 2. Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# 3. Try an example
cat ../examples/short.txt
./build -i ../examples/short.txt -o short.rr.fa.ds -p restricted_recompression -m fast
cat ../examples/query.tsv
./query -i short.rr.fa.ds -q ../examples/query.tsv -w query.log -o update.ds
cat short.1.txt
cat short.2.txt
cat short.3.txt
```

---

## Overview

This repository provides a dynamic data structure, `DynamicRLSLPString`, for an RLSLP that derives a given string $T[0..n-1]$.

The grammar-based compression algorithm can be selected from the following two options:

* **Restricted recompression**
* **Signature encoding**

Restricted recompression is a randomized construction algorithm that outputs an RLSLP that can be stored in expected $\delta$-optimal space. 
See [the paper](https://doi.org/10.48550/arXiv.2604.24080) for the details of the algorithm.

Signature encoding is a deterministic construction algorithm that outputs an RLSLP that can be stored in compressed space. 
See [the paper](https://doi.org/10.1016/j.dam.2019.01.014) for the details of the algorithm.


The following table summarizes the properties of the RLSLPs constructed by these compression algorithms.

| Value                                    | Restricted recompression                                    | Signature encoding       |
| ---------------------------------------- | ----------------------------------------------------------- | ------------------------ |
| Height `H` of the derivation tree        | Expected $O(\log n)$                                        | At most $4 \log n$       |
| Number `M` of explicit nonterminals      | Expected $O(\delta \log ((n \log \sigma)/(\delta \log n)))$ | $O(z \log n \log^{*} n)$ |
| Number `M'` of non-explicit nonterminals | $O(MH)$                                                     | At most `M`              |

> [!NOTE]
>
> * $H$: The height of the derivation tree corresponding to the RLSLP.
> * $\delta$: Substring complexity.
> * $\sigma$: Alphabet size.
> * $z$: The number of factors in the LZ77 factorization of $T$.
> * An explicit nonterminal is a nonterminal corresponding a leaf or a branching node in the derivation tree.

When constructing an instance of `DynamicRLSLPString`, you can choose either `standard` or `fast` as the data structure mode.

If `fast` mode is selected, `DynamicRLSLPString` stores the following cached information for each explicit nonterminal `X`:

* short prefixes and suffixes of the string represented by `X`
* pointers to certain ancestors `X'` of `X`, together with their relative distances

These caches are used to speed up several operations supported by `DynamicRLSLPString`.

In either mode, the memory usage of `DynamicRLSLPString` is $O(M \log n)$ bits.

An instance of `DynamicRLSLPString` can be built using either `DynamicRLSLPString::offline_build_from_text` or `DynamicRLSLPString::online_build_from_text_file`.

| Operation                     | Time Complexity | Working Space |
| ----------------------------- | --------------- | ------------- |
| `offline_build_from_text`     | $O(nb)$         | $O(n \log n)$ |
| `online_build_from_text_file` | $O(nb)$         | $O(M \log n)$ |

> [!NOTE]
>
> * $b = O(1)$ if the RLSLP is built by restricted recompression. Otherwise, $b = O(\log^{*} n)$.

`DynamicRLSLPString` supports the following operations.

| Operation                | Time Complexity  | Description                                                                                                  |
| ------------------------ | ---------------- | ------------------------------------------------------------------------------------------------------------ |
| `access_char(i)`         | $O(H)$           | Returns $T[i]$.                                                                                              |
| `access_substring(i, d)` | $O(H + d)$       | Returns $T[i..i+d-1]$.                                                                                       |
| `lce(i, j)`              | $O(Hb)$          | Returns the length of the longest common prefix of the two suffixes $T[i..n-1]$ and $T[j..n-1]$.             |
| `reverse_lce(i, j)`      | $O(Hb)$          | Returns the length of the longest common suffix of the two prefixes $T[0..i]$ and $T[0..j]$.                 |
| `lcp(X, X')`             | $O(Hb)$          | Returns the length of the longest common prefix of the two strings derived by the nonterminals `X` and `X'`. |
| `lcs(X, X')`             | $O(Hb)$          | Returns the length of the longest common suffix of the two strings derived by the nonterminals `X` and `X'`. |
| `decompress(ofs)`        | $O(n)$           | Writes $T$ to the given output file stream `ofs`.                                                            |
| `get_all_occurrences(X)` | $O(vocc H)$      | Returns all occurrence positions of the explicit nonterminal `X` in the derivation tree.                     |
| `insert_string(i, P)`    | Expected $O(Hb)$ | Inserts the string $P[0..m-1]$ at position $i$.                                                              |
| `delete_string(i, m)`    | Expected $O(Hb)$ | Deletes $T[i..i+m-1]$ from $T$.                                                                              |

> [!NOTE]
>
> * $vocc$: The number of occurrences of `X` in the derivation tree.
> * The occurrence position of an occurrence of `X` is the position of the character corresponding to the leftmost leaf in the subtree rooted at that occurrence.
> * `get_all_occurrences` runs in $O(vocc H / \log \log n)$ time if `fast` mode is selected.
> * `lcp(X, X')` runs in $O(1)$ time if `lcp(X, X')` is very small and `fast` mode is selected.
> * `lcs(X, X')` runs in $O(1)$ time if `lcs(X, X')` is very small and `fast` mode is selected.

---

## Requirements

* **C++ compiler**: C++17 or later, such as GCC 7+, Clang 5+, or MSVC 2017+
* **CMake**: 3.10 or later
* **Make**: GNU Make

---

## Installation

### Download

```bash
git clone https://github.com/TNishimoto/dynamic_rlslp.git
cd dynamic_rlslp
git submodule update --init --recursive
```

### Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Generated Executables

| Executable | Description                                                      |
| ---------- | ---------------------------------------------------------------- |
| `build`    | Builds a dynamic data structure for an RLSLP from an input text. |
| `print`    | Displays details of a built dynamic data structure.              |
| `query`    | Executes queries from a command file.                            |

---

## Usage

### Build Data Structure

Build a dynamic data structure for an RLSLP representing a given text.

```text
Options:
  -i, --input_file_path     Input text file path (string)
  -o, --output_file_path    Output data structure file path (string [=])
  -p, --parser              Grammar parser: restricted_recompression or signature_encoding (string [=restricted_recompression])
  -s, --seed                Random seed (unsigned long long [=0])
  -f, --offline_build       Use offline build mode (bool [=0])
  -m, --mode                Data structure mode: fast or standard (string [=standard])
  -?, --help                Print this help message
```

> [!NOTE]
> If `--offline_build=0`, `DynamicRLSLPString` is constructed using `dynRLSLP::DynamicRLSLPString::online_build_from_text_file`.  
> If `--offline_build=1`, `DynamicRLSLPString` is constructed using `dynRLSLP::DynamicRLSLPString::offline_build_from_text`.

**Example:**

```bash
./build -i ../examples/short.txt -o short.rr.fa.ds -p restricted_recompression -m fast
```

```text
=============RESULT===============
File : ../examples/short.txt
Output : short.rr.fa.ds
Build mode : Online
  Statistics (DynamicRLSLPString)
    Mode: Fast
    Left Short Strings (Cache): 14 * 8 bytes
    Right Short Strings (Cache): 14 * 8 bytes
    Ancestor Cache List (Cache): 14 * 16 bytes
    Statistics (DynamicGrammarForLayeredRLSLP): 
      Statistics (GrammarForLayeredRLSLP): 
        Compression Algorithm:  Restricted Recompression
        Text Length:    41
        The level of the derivation tree: 70
        Statistics (DictionaryForLayeredRLSLP): 
          Number of nonterminals:               229
            Number of explicit nonterminals:            14
            Number of implicit nonterminals:            215
          Number of null nonterminals:          0
        [END]
        Statistics (RandomBitDictionary): 
          Seed: 0
          Number of short random bytes: 56
          Number of middle random bytes: 104
          Number of long random bytes: 40
        [END]
      [END]
      Statistics (FastParentDictionary): 30 pointers
        Number of lightweight Pointers: 19
        Number of middleweight Pointers: 0
        Number of heavyweight Pointers: 0
        Number of null Pointers: 11
      [END]
    [END]
  [END]
Execution time : 0ms [infchars/ms]
Memory footprint: 1616 KB (1 MB)
==================================
```

```bash
./build -i ../examples/short.txt -o short.se.st.ds -p signature_encoding -m standard -f 1
```

```text
=============RESULT===============
File : ../examples/short.txt
Output : short.se.st.ds
Build mode : Offline
  Statistics (DynamicRLSLPString)
    Mode: Standard
    Left Short Strings (Cache): 0 * 8 bytes
    Right Short Strings (Cache): 0 * 8 bytes
    Ancestor Cache List (Cache): 0 * 16 bytes
    Statistics (DynamicGrammarForLayeredRLSLP): 
      Statistics (GrammarForLayeredRLSLP): 
        Compression Algorithm:  Signature Encoding
        Text Length:    41
        The level of the derivation tree: 8
        Statistics (DictionaryForLayeredRLSLP): 
          Number of nonterminals:               36
            Number of explicit nonterminals:            21
            Number of implicit nonterminals:            15
          Number of null nonterminals:          0
        [END]
        Statistics (RandomBitDictionary): 
          Seed: 0
          Number of short random bytes: 24
          Number of middle random bytes: 40
          Number of long random bytes: 40
        [END]
      [END]
      Statistics (FastParentDictionary): 49 pointers
        Number of lightweight Pointers: 33
        Number of middleweight Pointers: 0
        Number of heavyweight Pointers: 0
        Number of null Pointers: 16
      [END]
    [END]
  [END]
Execution time : 0ms [infchars/ms]
Memory footprint: 1616 KB (1 MB)
==================================
```

---

### Print Data Structure

Display statistics and a memory breakdown for an input dynamic data structure storing an RLSLP.

This command can also output the following files:

* A JSON file representing the input dynamic data structure.
* A JSON file representing the input RLSLP.
* A plain text file representing the derivation tree of the input RLSLP.
* A JSON file representing the canonical RLSLP corresponding to the input RLSLP.

> [!NOTE]
> The canonical RLSLP corresponding to a RLSLP is the RLSLP obtained by removing non-explicit nonterminals from the grammar.

> [!NOTE]
> To clarify the behavior of the construction algorithm, we illustrate the derivation tree of the RLSLP built by the signature encoding as a 4-ary tree. See also Fig. 3 in [the paper](https://doi.org/10.1016/j.dam.2019.01.014).


```text
Options:
  -i, --input_file_path                            File path to a dynamic data structure storing an RLSLP (string)
  -j, --output_dynamic_data_structure_file_path    File path for writing the input dynamic data structure in JSON format (string [=])
  -r, --output_rlslp_file_path                     File path for writing the RLSLP in JSON format (string [=])
  -d, --output_derivation_tree_file_path           File path for writing the derivation tree of the RLSLP (string [=])
  -c, --output_canonical_rlslp_file_path           File path for writing the canonical RLSLP corresponding to the RLSLP in JSON format (string [=])
  -?, --help                                       Print this help message
```

**Example:**

```bash
./print -i short.rr.fa.ds -j short.rr.fa.j.json -r short.rr.fa.r.json -d short.rr.fa.d.log -c short.rr.fa.c.json
```

```text
=============RESULT===============
File : short.rr.fa.ds
Statistics (DynamicRLSLPString)
  Mode: Fast
  Left Short Strings (Cache): 14 * 8 bytes
  Right Short Strings (Cache): 14 * 8 bytes
  Ancestor Cache List (Cache): 14 * 16 bytes
  Statistics (DynamicGrammarForLayeredRLSLP): 
    Statistics (GrammarForLayeredRLSLP): 
      Compression Algorithm:    Restricted Recompression
      Text Length:      41
      The level of the derivation tree: 70
      Statistics (DictionaryForLayeredRLSLP): 
        Number of nonterminals:                 229
          Number of explicit nonterminals:              14
          Number of implicit nonterminals:      215
        Number of null nonterminals:            0
      [END]
      Statistics (RandomBitDictionary): 
        Seed: 16539473643135484331
        Number of short random bytes: 52
        Number of middle random bytes: 104
        Number of long random bytes: 40
      [END]
    [END]
    Statistics (FastParentDictionary): 30 pointers
      Number of lightweight Pointers: 19
      Number of middleweight Pointers: 0
      Number of heavyweight Pointers: 0
      Number of null Pointers: 11
    [END]
  [END]
[END]

Memory Breakdown (DynamicRLSLPString): 2327 bytes
  left_short_string_list: 136 bytes
  right_short_string_list: 136 bytes
  ancestor_cache_list: 248 bytes
  dictionary_mode: 1 bytes
  Memory Breakdown (DynamicGrammarForLayeredRLSLP): 1727 bytes
    Memory Breakdown (GrammarForLayeredRLSLP): 912 bytes
      document_counter: 80 bytes
      grammar_parsing_type: 4 bytes
      Memory Breakdown (DictionaryForLayeredRLSLP): 600 bytes
        explicit_nonterminal_rule_list_: 360 bytes
        explicit_nonterminal_level_list_: 52 bytes
        explicit_nonterminal_length_list_: 136 bytes
        relative_max_level_list_: 52 bytes
      [END]
      Memory Breakdown (RandomBitDictionary): 228 bytes
        short_random_bits_: 52 bytes
        middle_random_bits_: 104 bytes
        long_random_bits_: 40 bytes, counts: 0
        Others: 32 bytes
      [END]
    [END]
    unused_nonterminals: 24 bytes
    character_nonterminal_item_map: 216 bytes
    character_id_map: 176 bytes
    Memory Breakdown (FastParentDictionary): 399 bytes
      primaryParents: 136 bytes
      sub_pointer: 136 bytes
      sub_pointer_status_: 38 bytes
      Memory Breakdown (SubParentVector): 80 bytes, count: 2
      Memory Breakdown (FewParentDictionary): 0 bytes, count: 0
      Memory Breakdown (ManyParentDictionary): 0 bytes, count: 0
    [END]
  [END]
[END]
==================================
```

The output files are as follows:

* [`short.rr.fa.j.json`](examples/log/short.rr.fa.j.json)
* [`short.rr.fa.r.json`](examples/log/short.rr.fa.r.json)
* [`short.rr.fa.d.log`](examples/log/short.rr.fa.d.log)
* [`short.rr.fa.c.json`](examples/log/short.rr.fa.c.json)

---

### Query

Execute queries on a dynamic RLSLP using a TSV query file.

```text
usage: ./query --input_index_path=string --input_query_path=string --output_log_path=string [options] ...

options:
  -i, --input_index_path              Input index file path (.ds) (string)
  -q, --input_query_path              Input query file path in TSV format (string)
  -w, --output_log_path               Output log file path (string)
  -o, --output_index_path             Output path for saving the updated index (optional) (string [=])
  -t, --alternative_tab_key           Alternative tab key for the query file (optional) (string [=])
  -n, --alternative_line_break_key    Alternative line break key for the query file (optional) (string [=])
  -?, --help                          Print this help message
```

**Supported Queries:**

| Query        | Param 1 | Param 2 | Description                                                                                      |
| ------------ | ------- | ------- | ------------------------------------------------------------------------------------------------ |
| `ACCESS`     | `i`     | `m`     | Returns $T[i..i+m-1]$.                                                                           |
| `DECOMPRESS` | `F`     | `-`     | Writes the string derived by the RLSLP to the file path `F`.                                     |
| `LCE`        | `i`     | `j`     | Returns the length of the longest common prefix of the two suffixes $T[i..n-1]$ and $T[j..n-1]$. |
| `INSERT`     | `i`     | `P`     | Updates the input data structure by inserting string `P` into $T$ at position `i`.               |
| `DELETE`     | `i`     | `m`     | Updates the input data structure by deleting substring $T[i..i+m-1]$ from $T$.                   |

**Example:**

```bash
cat ../examples/query.tsv
./query -i short.rr.fa.ds -q ../examples/query.tsv -w query.log -o update.ds
cat short.1.txt
cat short.2.txt
cat short.3.txt
```

```text
ACCESS  5       15
LCE     5       10
DECOMPRESS      short.1.txt
DELETE  5       30
DECOMPRESS      short.2.txt
INSERT  5       _aaa
DECOMPRESS      short.3.txt
```

```text
=============RESULT===============
Index File:                 short.rr.fa.ds
Query File:                 ../examples/query.tsv
Log File:                   query.log
Output Index File:          update.ds
Alternative Tab Key:        
Alternative Line Break Key: 
==================================
```

```text
cacao_cacao_cacao_cacao_cacao_cacao_cacao
```

```text
cacao_cacao
```

```text
cacao_aaa_cacao
```

---

## API Documentation

[Doxygen Documentation](https://TNishimoto.github.io/dynamic_rlslp/index.html) *(in preparation)*

---

## Dependencies

This project uses the following library:

* [STool](https://github.com/TNishimoto/stool) — string utilities (MIT License)
* [cmdline](https://github.com/tanakh/cmdline) - a simple command line parser (BSD 3-Clause License)

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

If you use this library, please cite the following paper:

```bibtex
@article{DBLP:journals/corr/abs-2604-24080,
  author       = {Takaaki Nishimoto and
                  Yasuo Tabei},
  title        = {Dynamic Grammar-Compressed Self-Index in {\(\delta\)}-Optimal Space},
  journal      = {CoRR},
  volume       = {abs/2604.24080},
  year         = {2026},
  url          = {https://doi.org/10.48550/arXiv.2604.24080}
}
```

---

## References

* T. Nishimoto and Y. Tabei, “Dynamic Grammar-Compressed Self-Index in $\delta$-Optimal Space”, [arXiv:2604.24080](https://doi.org/10.48550/arXiv.2604.24080), 2026.
* T. Nishimoto et al., "Dynamic index and LZ factorization in compressed space", [Discret. Appl. Math.](https://doi.org/10.1016/j.dam.2019.01.014), 2020.  
* T. Kociumaka et al., "Toward a Definitive Compressibility Measure for Repetitive Sequences" [IEEE Trans. Inf. Theory](https://doi.org/10.1109/TIT.2022.3224382), 2023.  
