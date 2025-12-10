#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import re

# 클래스명 매핑
class_mappings = {
    'RemoteLCD': 'LocalLCD',
    'REMOTE_LCD': 'LOCAL_LCD',
    'RemoteCANCom': 'CanComm',
    'REMOTE_CAN_COM': 'CAN_COMM',
    'RemoteESPNow': 'RecvESPNow',
    'REMOTE_ESP_NOW': 'RECV_ESP_NOW',
    'RemoteLED': 'LocalLED',
    'REMOTE_LED': 'LOCAL_LED'
}

def rename_in_file(filepath):
    """파일 내용에서 클래스명을 변경"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        modified = False
        for old_name, new_name in class_mappings.items():
            if old_name in content:
                content = content.replace(old_name, new_name)
                modified = True
        
        if modified:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Updated: {filepath}")
            return True
        return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    src_dir = 'src'
    count = 0
    
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                if rename_in_file(filepath):
                    count += 1
    
    print(f"\nTotal files updated: {count}")

if __name__ == '__main__':
    main()
