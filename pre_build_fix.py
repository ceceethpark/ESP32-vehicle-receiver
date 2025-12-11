Import("env")
import sys
import io

# UTF-8 인코딩으로 stdout/stderr 래핑
if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
if sys.stderr.encoding != 'utf-8':
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

print("Encoding fix applied: stdout={}, stderr={}".format(sys.stdout.encoding, sys.stderr.encoding))
