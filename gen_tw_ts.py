from zhconv import convert

with open('translations/zh_CN.ts', 'r', encoding = 'utf-8') as fi, \
     open('translations/zh_TW.ts', 'w', encoding = 'utf-8') as fo:
    fo.write(convert(fi.read(), 'zh-tw'))
