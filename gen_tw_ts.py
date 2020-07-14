from zhconv import convert

with open('zh_CN.ts', 'r', encoding = 'utf-8') as fi, \
     open('zh_TW.ts', 'w', encoding = 'utf-8', newline = '\n') as fo:
    fo.write(convert(fi.read(), 'zh-tw').replace('簡體中文', '简体中文'))
