#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import re

def remove_korean_comments(filepath):
    """Replace Korean text with English"""
    try:
        # Try to read with any encoding
        content = None
        for encoding in ['utf-8', 'cp949', 'euc-kr', 'latin1']:
            try:
                with open(filepath, 'r', encoding=encoding) as f:
                    content = f.read()
                break
            except:
                continue
        
        if content is None:
            return False
        
        # Replace Korean strings with English placeholders
        # This is a simple replacement - just remove problematic characters
        content = re.sub(r'[\u3131-\u318e\uac00-\ud7a3]+', 'TEXT', content)
        
        # Write back as UTF-8
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"Cleaned: {filepath}")
        return True
    except Exception as e:
        print(f"Error: {filepath} - {e}")
        return False

def main():
    src_dir = 'src'
    count = 0
    
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                if remove_korean_comments(filepath):
                    count += 1
    
    print(f"\nTotal files cleaned: {count}")

if __name__ == '__main__':
    main()
