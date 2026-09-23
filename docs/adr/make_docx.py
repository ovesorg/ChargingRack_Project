# -*- coding: utf-8 -*-
import zipfile, os

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

def esc(t):
    return (t.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
             .replace('"', '&quot;'))

def run(text, bold=False, size=22, color=None):
    rpr = '<w:rPr>'
    if bold:
        rpr += '<w:b/>'
    rpr += '<w:sz w:val="%d"/><w:szCs w:val="%d"/>' % (size, size)
    if color:
        rpr += '<w:color w:val="%s"/>' % color
    rpr += '</w:rPr>'
    # support \n inside text -> line breaks
    parts = text.split('\n')
    body = ''
    for i, p in enumerate(parts):
        if i > 0:
            body += '<w:br/>'
        body += '<w:t xml:space="preserve">%s</w:t>' % esc(p)
    return '<w:r>%s%s</w:r>' % (rpr, body)

def para(runs, align=None, before=80, after=80, style=None):
    ppr = '<w:pPr>'
    if style:
        ppr += '<w:pStyle w:val="%s"/>' % style
    ppr += '<w:spacing w:before="%d" w:after="%d"/>' % (before, after)
    if align:
        ppr += '<w:jc w:val="%s"/>' % align
    ppr += '</w:pPr>'
    if isinstance(runs, str):
        runs = [run(runs)]
    return '<w:p>%s%s</w:p>' % (ppr, ''.join(runs))

def heading(text, size=30, color='1F3864'):
    return para([run(text, bold=True, size=size, color=color)],
                before=200, after=120)

def bullet(text):
    ppr = ('<w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>'
           '<w:spacing w:before="40" w:after="40"/></w:pPr>')
    return '<w:p>%s%s</w:p>' % (ppr, run(text))

def number(text):
    ppr = ('<w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="2"/></w:numPr>'
           '<w:spacing w:before="40" w:after="40"/></w:pPr>')
    return '<w:p>%s%s</w:p>' % (ppr, run(text))

def box(text, fill='D5E8F0', width=7000, size=22, bold=True, color=None):
    border = ('<w:tblBorders>'
              '<w:top w:val="single" w:sz="6" w:color="2E75B6"/>'
              '<w:left w:val="single" w:sz="6" w:color="2E75B6"/>'
              '<w:bottom w:val="single" w:sz="6" w:color="2E75B6"/>'
              '<w:right w:val="single" w:sz="6" w:color="2E75B6"/>'
              '<w:insideH w:val="single" w:sz="6" w:color="2E75B6"/>'
              '<w:insideV w:val="single" w:sz="6" w:color="2E75B6"/>'
              '</w:tblBorders>')
    tc = ('<w:tc><w:tcPr>'
          '<w:tcW w:w="%d" w:type="dxa"/>'
          '<w:shd w:val="clear" w:color="auto" w:fill="%s"/>'
          '<w:tcMar><w:top w:w="100" w:type="dxa"/><w:left w:w="150" w:type="dxa"/>'
          '<w:bottom w:w="100" w:type="dxa"/><w:right w:w="150" w:type="dxa"/></w:tcMar>'
          '</w:tcPr>'
          '<w:p><w:pPr><w:jc w:val="center"/></w:pPr>%s</w:p>'
          '</w:tc>') % (width, fill, run(text, bold=bold, size=size, color=color))
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="%d" w:type="dxa"/>%s'
           '<w:tblLook w:val="04A0"/></w:tblPr>'
           '<w:tblGrid><w:gridCol w:w="%d"/></w:tblGrid>'
           '<w:tr>%s</w:tr></w:tbl>') % (width, border, width, tc)
    # spacer before/after table
    return ('<w:p><w:pPr><w:spacing w:before="20" w:after="20"/></w:pPr></w:p>'
            + tbl +
            '<w:p><w:pPr><w:spacing w:before="20" w:after="20"/></w:pPr></w:p>')

def arrow(txt='↓'):
    return para([run(txt, bold=True, size=24, color='2E75B6')],
                align='center', before=0, after=0)

def vtable(rows, w1=3200, w2=6160):
    border = ('<w:tblBorders>'
              '<w:top w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:left w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:bottom w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:right w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideH w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideV w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '</w:tblBorders>')
    grid = '<w:tblGrid><w:gridCol w:w="%d"/><w:gridCol w:w="%d"/></w:tblGrid>' % (w1, w2)
    trs = ''
    for i, (a, b) in enumerate(rows):
        fill = 'F2F2F2' if i % 2 == 0 else 'FFFFFF'
        c1 = ('<w:tc><w:tcPr><w:tcW w:w="%d" w:type="dxa"/>'
              '<w:shd w:val="clear" w:color="auto" w:fill="%s"/>'
              '<w:tcMar><w:top w:w="60" w:type="dxa"/><w:left w:w="100" w:type="dxa"/>'
              '<w:bottom w:w="60" w:type="dxa"/><w:right w:w="100" w:type="dxa"/></w:tcMar>'
              '</w:tcPr><w:p>%s</w:p></w:tc>') % (w1, fill, run(a, bold=True, size=20))
        c2 = ('<w:tc><w:tcPr><w:tcW w:w="%d" w:type="dxa"/>'
              '<w:tcMar><w:top w:w="60" w:type="dxa"/><w:left w:w="100" w:type="dxa"/>'
              '<w:bottom w:w="60" w:type="dxa"/><w:right w:w="100" w:type="dxa"/></w:tcMar>'
              '</w:tcPr><w:p>%s</w:p></w:tc>') % (w2, run(b, size=20))
        trs += '<w:tr>%s%s</w:tr>' % (c1, c2)
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="%d" w:type="dxa"/>%s'
           '<w:tblLook w:val="04A0"/></w:tblPr>%s%s</w:tbl>') % (w1 + w2, border, grid, trs)
    return tbl

body = []
# Title
body.append(para([run('充电功率动态分配算法说明', bold=True, size=34, color='1F3864')],
                 align='center', before=0, after=60))
body.append(para([run('函数 vcu_canbox_cur() — 按子设备 SOC 大小分配并设置下发功率',
                      size=22, color='595959')], align='center', before=0, after=160))

# 一
body.append(heading('一、算法概述'))
body.append(para([run(
    '在充电站应用中，系统通过 CAN 总线管理多个子设备（最多 CHARGE_NUM 个）。每个子设备上报自身的额定'
    '电压/额定电流（存入 g_BmsSysAP[i]），并实时上报 SOC（存入 g_SlotBmsInfor[i].Soc）。用户通过上位机设置'
    '一个总功率 power，系统同时计算出所有子设备额定功率之和 total_power_all。本算法根据“设置功率”与“全部'
    '负载额定功率之和”的大小关系，决定下发策略：')]))
body.append(bullet('当 power ≥ total_power_all（功率充足）：所有子设备均按额定值（额定电压、额定电流）下发。'))
body.append(bullet('当 power < total_power_all（功率不足）：按 SOC 从大到小排序，SOC 高的子设备优先获得充电功率；'
                   '功率够时按额定下发，剩余功率不足时按“剩余功率 ÷ 额定电压”动态计算电流，依次分配直至功率用尽。'))

# 二
body.append(heading('二、关键变量说明'))
vars_ = [
    ('power', '用户设置的功率（由 get_set_power() 经 MbUint8ToFloat 得到），与 total_power_all 同单位。'),
    ('total_power_all', '全部子设备额定功率之和 = Σ(ap_vol[i]×ap_cur[i]) / 1e7，由 CanProc() 周期性计算。'),
    ('ap_vol[i] / ap_cur[i]', '第 i 个子设备的额定电压 / 额定电流，来自 g_BmsSysAP[i]（TargetAPVoltageH/L、TargetAPCurrentH/L）。'),
    ('power_raw', 'power 换算到与 ap_vol×ap_cur 相同量纲的值：power × 10000000.0f，作为可分配的总功率。'),
    ('left_power', '剩余可分配功率，初始 = power_raw；每分配一个设备后扣减，归零后其余设备电流为 0。'),
    ('rated_power', '第 j 个子设备的额定功率 = ap_vol[j] × ap_cur[j]，仅用于判断剩余功率是否够其满功率运行（非累计消耗）。'),
    ('soc_order[]', '按 g_SlotBmsInfor[i].Soc 从大到小排序后的子设备序号数组，决定分配优先级。'),
    ('out_cur[i]', '最终下发给第 i 个子设备的充电电流（已按功率分配/额定折算）。'),
    ('candi', '下发 CAN ID = 0x1803F000 + 子设备序号 i。'),
]
body.append(vtable(vars_))

# 三
body.append(heading('三、处理流程（文字步骤）'))
steps = [
    '读取设置功率：set_power = get_set_power()，经 MbUint8ToFloat() 得到 float 型 power。',
    '读取额定值：遍历 i = 0…CHARGE_NUM-1，从 g_BmsSysAP[i] 取出 ap_vol[i]、ap_cur[i]，初始化 out_cur[i]=0，并按 i 初始化 soc_order[i]。',
    'SOC 排序：对所有子设备按 g_SlotBmsInfor[i].Soc 从大到小进行简单选择排序，排序结果存入 soc_order[]，使 SOC 大的排在前、优先分配。',
    '量纲换算：power_raw = power × 10000000.0f，使其与 ap_vol×ap_cur 处于同一量纲。',
    '分支判断：若 power ≥ total_power_all，则所有 out_cur[i] = ap_cur[i]（全部按额定下发）；否则进入动态分配。',
    '动态分配（功率不足）：left_power = power_raw；按 soc_order 顺序，对每个设备 j：若额定电压或额定电流为 0，或 left_power≤0 则跳过；若 left_power ≥ 额定功率(ap_vol×ap_cur) 则下发额定电流并扣减；否则下发电流 = left_power / ap_vol[j]，并将 left_power 清零。',
    '逐设备下发：遍历 i = 0…CHARGE_NUM-1，CAN ID = 0x1803F000 + i；电压按额定电压下发（H/L 拆分），电流按 out_cur[i] 下发；仅当 power > 1.0 且 out_cur[i] > 0 时 ChargeEnable = 0x55，否则为 0；调用 CanTransmit() 发送 8 字节。',
]
for s in steps:
    body.append(number(s))

# 四 流程图
body.append(heading('四、算法流程图'))
body.append(box('开始', fill='E2EFDA'))
body.append(arrow())
body.append(box('读取设置功率 power\n(get_set_power → MbUint8ToFloat)'))
body.append(arrow())
body.append(box('读取各子设备额定值\nap_vol[i], ap_cur[i] ← g_BmsSysAP[i]'))
body.append(arrow())
body.append(box('按 SOC 从大到小排序\n生成 soc_order[]（SOC 大者优先）'))
body.append(arrow())
body.append(box('power_raw = power × 1e7'))
body.append(arrow())
body.append(box('power ≥ total_power_all ?', fill='FFF2CC', bold=True))
body.append(arrow('是（功率充足）'))
body.append(box('所有子设备 out_cur[i] = ap_cur[i]\n（按额定值全量下发）', fill='E2EFDA'))
body.append(arrow('否（功率不足）'))
body.append(box('left_power = power_raw\n依 soc_order 顺序遍历每个设备 j'))
body.append(arrow())
body.append(box('ap_vol[j]=0 或 ap_cur[j]=0\n或 left_power≤0 ?', fill='FFF2CC'))
body.append(arrow('是'))
body.append(box('跳过该设备（out_cur[j] = 0）', fill='F2F2F2'))
body.append(arrow('否'))
body.append(box('left_power ≥ ap_vol[j]×ap_cur[j] ?', fill='FFF2CC'))
body.append(arrow('是'))
body.append(box('out_cur[j] = ap_cur[j]\n（满额定下发），left_power −= 额定功率'))
body.append(arrow('否'))
body.append(box('out_cur[j] = left_power / ap_vol[j]\n（剩余功率÷额定电压），left_power = 0'))
body.append(arrow())
body.append(box('遍历所有子设备，逐一下发：\ncandi = 0x1803F000 + i\n电压=额定，电流=out_cur[i]\nChargeEnable = (power>1.0 且 out_cur>0)?0x55:0', fill='DDEBF7'))
body.append(arrow())
body.append(box('结束', fill='E2EFDA'))

# 五
body.append(heading('五、分配示例'))
body.append(para([run(
    '假设 3 个子设备额定功率分别为 1000 / 1500 / 2000（同一量纲），设置总功率 power_raw = 2000，'
    '并设按 SOC 排序后顺序为设备 A(1000)、设备 B(1500)、设备 C(2000)：')]))
body.append(bullet('设备 A：2000 ≥ 1000 → 下发额定电流，剩余 left_power = 1000。'))
body.append(bullet('设备 B：1000 < 1500 → 电流 = 1000 / 额定电压，剩余功率清零。'))
body.append(bullet('设备 C：left_power 已为 0 → out_cur = 0，停止充电。'))

document = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<w:document xmlns:w="%s"><w:body>%s'
    '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
    '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
    'w:header="720" w:footer="720" w:gutter="0"/></w:sectPr>'
    '</w:body></w:document>'
) % (W, ''.join(body))

numbering = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<w:numbering xmlns:w="%s">'
    '<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0">'
    '<w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="•"/>'
    '<w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl></w:abstractNum>'
    '<w:abstractNum w:abstractNumId="1"><w:lvl w:ilvl="0">'
    '<w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%%1."/>'
    '<w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl></w:abstractNum>'
    '<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>'
    '<w:num w:numId="2"><w:abstractNumId w:val="1"/></w:num>'
    '</w:numbering>'
) % W

content_types = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
    '<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>'
    '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>'
    '<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>'
    '</Types>'
)

rels = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
    '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>'
    '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>'
    '</Relationships>'
)

doc_rels = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>'
    '</Relationships>'
)

core = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" '
    'xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" '
    'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
    '<dc:title>充电功率动态分配算法说明</dc:title><dc:creator>CodeBuddy</dc:creator></cp:coreProperties>'
)
app = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties">'
    '<Application>CodeBuddy</Application></Properties>'
)

path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '充电功率动态分配算法.docx')
with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', content_types)
    z.writestr('_rels/.rels', rels)
    z.writestr('word/document.xml', document)
    z.writestr('word/_rels/document.xml.rels', doc_rels)
    z.writestr('word/numbering.xml', numbering)
    z.writestr('docProps/core.xml', core)
    z.writestr('docProps/app.xml', app)

print('OK ->', path)
