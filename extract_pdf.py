import PyPDF2
import sys
sys.stdout.reconfigure(encoding='utf-8')

reader = PyPDF2.PdfReader(r'Uebungen.pdf')
total = len(reader.pages)
print(f'Total pages: {total}')
for i, page in enumerate(reader.pages, start=1):
    text = page.extract_text()
    print(f'--- Page {i} ---')
    print(text)
    print()
