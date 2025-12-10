#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys

def fix_file_encoding(filepath):
    """파일을 다양한 인코딩으로 읽어서 UTF-8로 저장"""
    encodings = ['cp949', 'euc-kr', 'latin1', 'iso-8859-1']
    
    for encoding in encodings:
        try:
            with open(filepath, 'r', encoding=encoding) as f:
                content = f.read()
            
            # UTF-8로 저장
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            
            print(f"Fixed {filepath} using {encoding}")
            return True
        except:
            continue
    
    print(f"Failed to fix {filepath}")
    return False

def main():
    src_dir = 'src'
    count = 0
    
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                if fix_file_encoding(filepath):
                    count += 1
    
    print(f"\nTotal files fixed: {count}")

if __name__ == '__main__':
    main()
