#!/usr/bin/env python3
"""
Extract valid MVP WebAssembly modules from .wast test files.

This script uses wast2json to decompose .wast files into individual modules,
filters out assert_invalid and assert_malformed test cases, and extracts
only valid (module ...) definitions into a corpus directory.

Usage:
    python scripts/build-spec-corpus.py [--dry-run] [--input INPUT] [--output OUTPUT]

Example:
    python scripts/build-spec-corpus.py --dry-run  # Preview without execution
    python scripts/build-spec-corpus.py            # Extract modules to default output
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import re
import shutil
import subprocess
import sys
import tempfile
from collections import Counter
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Optional


@dataclass
class ProcessStats:
    """Statistics for corpus building process."""

    total_wast_files: int = 0
    processed_wast_files: int = 0
    failed_wast_files: int = 0
    total_modules: int = 0
    valid_modules: int = 0
    invalid_modules: int = 0
    malformed_modules: int = 0
    extracted_wasm_files: int = 0


def setup_logging(verbose: bool = False) -> None:
    """Configure logging with appropriate verbosity."""
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(format="%(levelname)s: %(message)s", level=level)


def find_tool(tool_name: str) -> Optional[Path]:
    """
    Locate a tool in system PATH.

    Args:
        tool_name: Name of the tool executable

    Returns:
        Path to the tool if found, None otherwise
    """
    result = shutil.which(tool_name)
    return Path(result) if result else None


def validate_tools() -> bool:
    """
    Validate that required WABT tools are available.

    Returns:
        True if all tools are available, False otherwise
    """
    required_tools = ["wast2json", "wasm2wat"]
    missing = []

    for tool in required_tools:
        if not find_tool(tool):
            missing.append(tool)
            logging.error(f"Required tool not found: {tool}")

    if missing:
        logging.error("Please install WABT tools: https://github.com/WebAssembly/wabt")
        return False

    return True


def extract_instructions(wasm_path: Path) -> list[str]:
    """
    Extract instruction list from a wasm file using wasm-objdump.

    Args:
        wasm_path: Path to .wasm file

    Returns:
        List of instruction names
    """
    try:
        result = subprocess.run(
            ["wasm-objdump", "-d", str(wasm_path)],
            capture_output=True,
            text=True,
            check=True,
        )
        instructions = []
        for line in result.stdout.splitlines():
            if "|" in line:
                parts = line.split("|")
                if len(parts) >= 2:
                    instr_line = parts[1].strip()
                    if instr_line:
                        # Extract first word (instruction name)
                        instr = instr_line.split()[0]
                        instructions.append(instr)
        return instructions
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        logging.warning(f"Failed to extract instructions from {wasm_path}: {e}")
        return []


def generate_manifest(output_dir: Path, stats: ProcessStats) -> None:
    """
    Generate manifest.json with instruction coverage statistics.

    Args:
        output_dir: Directory containing .wasm files
        stats: ProcessStats from the extraction process
    """
    wasm_files = sorted(output_dir.glob("*.wasm"))

    if not wasm_files:
        logging.warning("No .wasm files found, skipping manifest generation")
        return

    all_instructions: list[str] = []
    file_entries: list[dict] = []

    for wasm_file in wasm_files:
        instructions = extract_instructions(wasm_file)
        all_instructions.extend(instructions)

        # Parse source from filename (pattern: <source>.<index>.wasm)
        name_parts = wasm_file.stem.rsplit(".", 1)
        source = (
            f"{name_parts[0]}.wast" if len(name_parts) > 1 else f"{wasm_file.stem}.wast"
        )

        file_entries.append(
            {
                "name": wasm_file.name,
                "source": source,
                "instruction_count": len(instructions),
            }
        )

    # Count instruction frequency
    instr_counts = Counter(all_instructions)

    manifest = {
        "corpus": "WebAssembly MVP Specification Test Corpus",
        "generated": date.today().isoformat(),
        "total_modules": len(wasm_files),
        "total_instructions": len(all_instructions),
        "unique_instructions": len(instr_counts),
        "instruction_coverage": dict(instr_counts),
        "top_instructions": [
            {"instruction": instr, "count": count}
            for instr, count in instr_counts.most_common(20)
        ],
        "files": file_entries,
    }

    manifest_path = output_dir / "manifest.json"
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)

    logging.info(f"Generated manifest: {manifest_path}")
    logging.info(
        f"  Modules: {len(wasm_files)}, Instructions: {len(all_instructions)}, Unique: {len(instr_counts)}"
    )


def read_wast_file_list(input_file: Path) -> list[Path]:
    """
    Read list of .wast files from input file.

    Args:
        input_file: Path to file containing .wast file paths (one per line)

    Returns:
        List of absolute paths to .wast files

    Raises:
        FileNotFoundError: If input file doesn't exist
        ValueError: If no valid .wast files found
    """
    if not input_file.exists():
        raise FileNotFoundError(f"Input file not found: {input_file}")

    wast_files = []
    repo_root = input_file.resolve().parents[1]  # scripts/ -> repo root

    with open(input_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            # Convert relative path to absolute
            wast_path = repo_root / line
            if not wast_path.exists():
                logging.warning(f"File not found (skipping): {line}")
                continue

            wast_files.append(wast_path)

    if not wast_files:
        raise ValueError(f"No valid .wast files found in {input_file}")

    logging.info(f"Found {len(wast_files)} .wast files to process")
    return wast_files


def extract_modules_from_wast(
    wast_file: Path, output_dir: Path, stats: ProcessStats, dry_run: bool = False
) -> bool:
    """
    Extract valid modules from a .wast file using wast2json.

    Args:
        wast_file: Path to .wast file to process
        output_dir: Directory to write extracted .wasm files
        stats: ProcessStats object to update
        dry_run: If True, only preview without writing files

    Returns:
        True if processing succeeded, False otherwise
    """
    wast_name = wast_file.stem
    logging.info(f"Processing: {wast_file.name}")

    # Create temporary directory for wast2json output
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)
        json_file = temp_path / f"{wast_name}.json"

        # Run wast2json to decompose .wast file
        try:
            result = subprocess.run(
                ["wast2json", str(wast_file), "-o", str(json_file)],
                capture_output=True,
                text=True,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            logging.error(
                f"  wast2json failed for {wast_file.name}: {e.stderr.strip()}"
            )
            stats.failed_wast_files += 1
            return False
        except FileNotFoundError:
            logging.error("  wast2json not found in PATH")
            return False

        # Parse JSON output
        try:
            with open(json_file, "r") as f:
                data = json.load(f)
        except json.JSONDecodeError as e:
            logging.error(f"  Failed to parse JSON from {json_file}: {e}")
            stats.failed_wast_files += 1
            return False
        except FileNotFoundError:
            logging.error(f"  wast2json did not produce output file: {json_file}")
            stats.failed_wast_files += 1
            return False

        # Process commands from JSON
        commands = data.get("commands", [])
        valid_count = 0
        invalid_count = 0
        malformed_count = 0

        for cmd in commands:
            cmd_type = cmd.get("type", "")
            stats.total_modules += 1

            if cmd_type == "module":
                # Valid module - extract it
                module_filename = cmd.get("filename")
                if not module_filename:
                    logging.warning(f"  Module command missing filename field")
                    continue

                source_wasm = temp_path / module_filename
                if not source_wasm.exists():
                    logging.warning(f"  Module file not found: {module_filename}")
                    continue

                # Generate output filename: <wast_name>.<module_index>.wasm
                dest_wasm = output_dir / f"{wast_name}.{valid_count}.wasm"
                dest_wat = output_dir / f"{wast_name}.{valid_count}.wat"

                if dry_run:
                    logging.info(f"  [DRY-RUN] Would extract: {dest_wasm.name}")
                else:
                    # Copy .wasm file
                    shutil.copy2(source_wasm, dest_wasm)

                    # Generate .wat file using wasm2wat
                    try:
                        subprocess.run(
                            ["wasm2wat", str(dest_wasm), "-o", str(dest_wat)],
                            capture_output=True,
                            text=True,
                            check=True,
                        )
                        logging.debug(f"  Generated: {dest_wat.name}")
                    except subprocess.CalledProcessError as e:
                        logging.warning(
                            f"  wasm2wat failed for {dest_wasm.name}: {e.stderr.strip()}"
                        )
                        # Keep the .wasm file even if .wat generation fails

                valid_count += 1
                stats.valid_modules += 1
                stats.extracted_wasm_files += 1

            elif cmd_type == "assert_invalid":
                invalid_count += 1
                stats.invalid_modules += 1
                logging.debug(f"  Skipping assert_invalid module")

            elif cmd_type == "assert_malformed":
                malformed_count += 1
                stats.malformed_modules += 1
                logging.debug(f"  Skipping assert_malformed module")

        logging.info(
            f"  Extracted {valid_count} valid modules (skipped {invalid_count} invalid, {malformed_count} malformed)"
        )
        stats.processed_wast_files += 1
        return True


def main() -> int:
    """
    Main entry point for the corpus builder.

    Returns:
        Exit code (0 for success, 1 for failure)
    """
    parser = argparse.ArgumentParser(
        description="Extract valid MVP WebAssembly modules from .wast test files",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --dry-run              Preview extraction without creating files
  %(prog)s                        Extract modules to default output directory
  %(prog)s --output corpus/       Extract modules to custom directory
  %(prog)s --input custom.txt     Use custom .wast file list
        """,
    )

    parser.add_argument(
        "--input",
        type=Path,
        default=Path("scripts/mvp-wast-files.txt"),
        help="Input file containing list of .wast files (default: scripts/mvp-wast-files.txt)",
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path("test/spec-corpus/raw"),
        help="Output directory for extracted .wasm files (default: test/spec-corpus/raw)",
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview extraction without creating files",
    )

    parser.add_argument("--verbose", action="store_true", help="Enable verbose logging")

    args = parser.parse_args()

    # Setup logging
    setup_logging(args.verbose)

    # Validate tools
    if not validate_tools():
        return 1

    # Read input file list
    try:
        wast_files = read_wast_file_list(args.input)
    except (FileNotFoundError, ValueError) as e:
        logging.error(str(e))
        return 1

    # Create output directory
    if not args.dry_run:
        args.output.mkdir(parents=True, exist_ok=True)
        logging.info(f"Output directory: {args.output}")
    else:
        logging.info("[DRY-RUN MODE] No files will be created")

    # Process each .wast file
    stats = ProcessStats(total_wast_files=len(wast_files))

    for wast_file in wast_files:
        extract_modules_from_wast(wast_file, args.output, stats, args.dry_run)

    # Print summary statistics
    print("\n" + "=" * 60)
    print("CORPUS BUILD SUMMARY")
    print("=" * 60)
    print(f"Total .wast files:       {stats.total_wast_files}")
    print(f"Processed successfully:  {stats.processed_wast_files}")
    print(f"Failed:                  {stats.failed_wast_files}")
    print(f"\nTotal modules found:     {stats.total_modules}")
    print(f"Valid modules:           {stats.valid_modules}")
    print(f"Invalid modules:         {stats.invalid_modules} (skipped)")
    print(f"Malformed modules:       {stats.malformed_modules} (skipped)")
    print(f"\nExtracted .wasm files:   {stats.extracted_wasm_files}")
    print("=" * 60)

    # Generate manifest.json
    if not args.dry_run:
        generate_manifest(args.output, stats)

    if args.dry_run:
        print("\n[DRY-RUN] Run without --dry-run to actually extract files")

    # Return success if all files processed
    if stats.failed_wast_files == 0:
        logging.info("Corpus build completed successfully")
        return 0
    else:
        logging.warning(
            f"Corpus build completed with {stats.failed_wast_files} failures"
        )
        return 1


if __name__ == "__main__":
    sys.exit(main())
