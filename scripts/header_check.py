#!/usr/bin/env python3
import sys
import os

MODULE_NAME = "HEADER_CHECK"

def check_header(filepath):
    filename = os.path.basename(filepath)
    guard_macro = f"BLUEOS_{filename.upper().replace('.', '_')}_"
    
    with open(filepath, 'r') as f:
        lines = [line.strip() for line in f.readlines() if line.strip()]
        
    if not lines:
        return True
        
    # Verify presence of inclusion guards inside top lines
    if not lines[0].startswith("#ifndef") or guard_macro not in lines[0]:
        print(f"<3>[  {MODULE_NAME}  ] Error: Header guard #ifndef missing or mismatched in {filename}")
        return False
    if not lines[1].startswith("#define") or guard_macro not in lines[1]:
        print(f"<3>[  {MODULE_NAME}  ] Error: Header guard #define missing or mismatched in {filename}")
        return False
        
    print(f"<6>[  {MODULE_NAME}  ] Header file {filename} passed encapsulation checks.")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(1)
    success = check_header(sys.argv[1])
    sys.exit(0 if success else 1)