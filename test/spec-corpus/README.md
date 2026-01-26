# WebAssembly MVP Specification Test Corpus

## 1. Overview
- **Purpose**: A semantically trusted subset of WebAssembly modules extracted from the official specification test suite, specifically curated for JIT/AOT instruction merging and optimization analysis.
- **Source**: [WebAssembly official specification test suite](https://github.com/WebAssembly/testsuite).
- **Scope**: Strictly WebAssembly MVP (2017) specification. All post-MVP features (SIMD, Bulk Memory, Reference Types, etc.) are excluded.
- **Size**: 798 modules extracted from 61 MVP-compatible `.wast` source files.

## 2. Directory Structure

| Category | Modules | Description | Key Instructions |
|----------|---------|-------------|------------------|
| 01-integer-arithmetic | 0 | i32/i64 arithmetic | i32.add, i32.mul, i64.div_s, i64.xor, ... |
| 02-float-arithmetic | 113 | f32/f64 arithmetic | f32.add, f64.mul, f32.sqrt, f64.abs, ... |
| 03-control-flow | 7 | Control flow | block, loop, if, br, br_table, return, ... |
| 04-memory-ops | 32 | Memory access | i32.load, f64.store, memory.size, memory.grow, ... |
| 05-function-calls | 4 | Function calls | call, call_indirect |
| 06-variables | 3 | Local/Global variables | local.get, local.set, local.tee, global.get, ... |
| 07-conversions | 0 | Type conversions | i32.wrap_i64, f32.convert_i32_s, f64.promote_f32, ... |
| 08-comparisons | 639 | Comparisons & Catch-all | i32.eq, f64.lt, drop, select, i32.const, ... |

## 3. File Organization
Each category directory follows a consistent structure:
```text
test/spec-corpus/
├── 01-integer-arithmetic/
│   ├── manifest.json          # Category metadata and instruction statistics
│   ├── *.wasm                 # WebAssembly binary modules
│   └── *.wat                  # WebAssembly text format (for human readability)
├── ...
└── manifest.json              # Top-level summary of the entire corpus
```
- **Naming convention**: `<source>.<index>.{wasm,wat}` (e.g., `f32.0.wasm`, `f32.0.wat`).
- Every `.wasm` binary has a corresponding `.wat` text file for inspection.

## 4. MVP Instruction Coverage
- **Reference**: `scripts/mvp-instruction-whitelist.txt` (172 base instructions).
- **Excluded Features**:
  - SIMD (v128.*, relaxed_*)
  - Sign-extension (i32.extend8_s, i64.extend32_s, etc.)
  - Bulk memory (memory.copy, memory.fill, table.init, etc.)
  - Reference types (ref.null, ref.func, extern.ref, etc.)
  - Multi-value, tail-call, exception-handling, threads/atomics.
- **Verification**: All modules have been verified to pass a post-MVP contamination check using `wasm-objdump`.

## 5. Manifests
- **Per-category manifests (`*/manifest.json`)**:
  - Full file list with source mapping.
  - Instruction coverage statistics (frequency of each instruction).
  - Top 10 most-used instructions in the category.
- **Top-level manifest (`manifest.json`)**:
  - Aggregated statistics across all 8 categories.
  - Total module count: 798.
  - Summary of instruction types and distribution.

## 6. Usage Examples

```bash
# List all modules in a category
ls test/spec-corpus/02-float-arithmetic/*.wasm

# Analyze instructions in a module
wasm-objdump -d test/spec-corpus/02-float-arithmetic/f32.0.wasm

# Run a module using uwvm
uwvm --run test/spec-corpus/02-float-arithmetic/f32.0.wasm

# Validate a module
wasm-validate test/spec-corpus/02-float-arithmetic/f32.0.wasm

# View manifest for a category (requires jq)
cat test/spec-corpus/02-float-arithmetic/manifest.json | jq

# Get aggregated instruction coverage stats
cat test/spec-corpus/manifest.json | jq '.categories'
```

## 7. Known Limitations
- **Empty categories**: `01-integer-arithmetic` and `07-conversions` are currently empty. This is because the initial classification was based strictly on source filename patterns (e.g., `f32.wast` → `02-float-arithmetic`).
- **Catch-all category**: `08-comparisons` contains 639/798 (80%) of the modules. This includes 402 modules from `const.wast` which only contain `const` and `control` instructions.
- **Classification quality**: The current classification is simplistic. Modules are assigned to categories based on their source file's primary theme rather than a deep analysis of their instruction content.
- **Future improvement**: It is recommended to reclassify the modules using a content-based analysis (see `.sisyphus/notepads/spec-wasm-corpus/classification-algorithm.md` for a proposed specification).

## 8. Adding New Modules
1. Place a new `.wast` file in `test/third-parties/wabt/third_party/testsuite/`.
2. Verify it only uses MVP instructions (no post-MVP features).
3. Add the file path to `scripts/mvp-wast-files.txt`.
4. Run the extraction script:
   ```bash
   python3 scripts/build-spec-corpus.py
   ```
5. Regenerate manifests (currently a manual process using internal scripts).
6. Verify the integrity:
   ```bash
   wasm-validate test/spec-corpus/**/*.wasm
   # Check for post-MVP contamination
   find test/spec-corpus -name "*.wasm" -exec sh -c 'wasm-objdump -d "$1" 2>/dev/null | grep -qE "v128|extend8_s|memory.copy|ref.null" && echo "CONTAMINATED: $1"' _ {} \;
   ```

## 9. Integrity Verification
The following commands can be used to verify the state of the corpus:

```bash
# Verify directory structure
ls -F test/spec-corpus/

# Verify all modules are valid according to the spec
find test/spec-corpus -name "*.wasm" -exec wasm-validate {} \;

# Verify no post-MVP contamination
find test/spec-corpus -name "*.wasm" -exec sh -c 'wasm-objdump -d "$1" 2>/dev/null | grep -qE "extend8_s|extend16_s|memory.copy|v128|ref.null" && echo "CONTAMINATED: $1"' _ {} \;

# Count total modules
find test/spec-corpus -name "*.wasm" | wc -l  # Expected: 798

# Verify manifests exist for all categories
ls test/spec-corpus/*/manifest.json | wc -l   # Expected: 8
```

## 10. References
- **Source repository**: [WebAssembly/testsuite](https://github.com/WebAssembly/testsuite)
- **MVP Specification**: [WebAssembly Core Spec 1.0](https://webassembly.github.io/spec/core/)
- **Instruction whitelist**: `scripts/mvp-instruction-whitelist.txt`
- **Extraction script**: `scripts/build-spec-corpus.py`
- **Feature documentation**: `documents/features.md`
