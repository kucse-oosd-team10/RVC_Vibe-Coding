#!/usr/bin/env python3
"""Convert LCOV coverage report to SonarCloud Generic Test Coverage XML format."""

import sys
import xml.etree.ElementTree as ET

def convert(lcov_path, output_path):
    root = ET.Element("coverage", version="1")
    current_file = None
    file_elem = None

    with open(lcov_path, "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("SF:"):
                filepath = line[3:]
                file_elem = ET.SubElement(root, "file", path=filepath)
            elif line.startswith("DA:"):
                parts = line[3:].split(",")
                if len(parts) >= 2 and file_elem is not None:
                    line_num = parts[0]
                    hits = parts[1]
                    covered = "true" if int(hits) > 0 else "false"
                    ET.SubElement(
                        file_elem,
                        "lineToCover",
                        lineNumber=line_num,
                        covered=covered,
                    )
            elif line == "end_of_record":
                file_elem = None

    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ")
    tree.write(output_path, xml_declaration=True, encoding="UTF-8")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input.lcov> <output.xml>")
        sys.exit(1)
    convert(sys.argv[1], sys.argv[2])
    print(f"Converted {sys.argv[1]} -> {sys.argv[2]}")