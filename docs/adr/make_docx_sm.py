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
    return ('<w:p><w:pPr><w:spacing w:before="20" w:after="20"/></w:pPr></w:p>'
            + tbl +
            '<w:p><w:pPr><w:spacing w:before="20" w:after="20"/></w:pPr></w:p>')

def arrow(txt='↓'):
    return para([run(txt, bold=True, size=24, color='2E75B6')],
                align='center', before=0, after=0)

def vtable(rows, w1=1800, w2=2400, w3=3160, w4=2000):
    border = ('<w:tblBorders>'
              '<w:top w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:left w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:bottom w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:right w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideH w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideV w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '</w:tblBorders>')
    grid = '<w:tblGrid>%s</w:tblGrid>' % ''.join(
        '<w:gridCol w:w="%d"/>' % w for w in (w1, w2, w3, w4))
    trs = ''
    for i, (a, b, c, d) in enumerate(rows):
        fill = 'F2F2F2' if i % 2 == 0 else 'FFFFFF'
        hdr = (i == 0)
        f = 'D9E2F3' if hdr else fill
        cells = ''
        for val, w in zip((a, b, c, d), (w1, w2, w3, w4)):
            cells += ('<w:tc><w:tcPr><w:tcW w:w="%d" w:type="dxa"/>'
                      '<w:shd w:val="clear" w:color="auto" w:fill="%s"/>'
                      '<w:tcMar><w:top w:w="50" w:type="dxa"/><w:left w:w="80" w:type="dxa"/>'
                      '<w:bottom w:w="50" w:type="dxa"/><w:right w:w="80" w:type="dxa"/></w:tcMar>'
                      '</w:tcPr><w:p>%s</w:p></w:tc>') % (
                          w, f, run(val, bold=hdr, size=18))
        trs += '<w:tr>%s</w:tr>' % cells
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="%d" w:type="dxa"/>%s'
           '<w:tblLook w:val="04A0"/></w:tblPr>%s%s</w:tbl>') % (
               w1 + w2 + w3 + w4, border, grid, trs)
    return tbl

def ttable(rows, widths):
    border = ('<w:tblBorders>'
              '<w:top w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:left w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:bottom w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:right w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideH w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '<w:insideV w:val="single" w:sz="4" w:color="CCCCCC"/>'
              '</w:tblBorders>')
    grid = '<w:tblGrid>%s</w:tblGrid>' % ''.join(
        '<w:gridCol w:w="%d"/>' % w for w in widths)
    trs = ''
    for i, row in enumerate(rows):
        fill = 'D9E2F3' if i == 0 else ('F2F2F2' if i % 2 == 0 else 'FFFFFF')
        hdr = (i == 0)
        cells = ''
        for val, w in zip(row, widths):
            cells += ('<w:tc><w:tcPr><w:tcW w:w="%d" w:type="dxa"/>'
                      '<w:shd w:val="clear" w:color="auto" w:fill="%s"/>'
                      '<w:tcMar><w:top w:w="50" w:type="dxa"/><w:left w:w="80" w:type="dxa"/>'
                      '<w:bottom w:w="50" w:type="dxa"/><w:right w:w="80" w:type="dxa"/></w:tcMar>'
                      '</w:tcPr><w:p>%s</w:p></w:tc>') % (
                          w, fill, run(val, bold=hdr, size=18))
        trs += '<w:tr>%s</w:tr>' % cells
    tbl = ('<w:tbl><w:tblPr><w:tblW w:w="%d" w:type="dxa"/>%s'
           '<w:tblLook w:val="04A0"/></w:tblPr>%s%s</w:tbl>') % (
               sum(widths), border, grid, trs)
    return tbl

body = []
body.append(para([run('充电与功率分配控制状态机', bold=True, size=34, color='1F3864')],
                 align='center', before=0, after=60))
body.append(para([run('基于 CanProc / vcu_canbox_cur / CanRecoveryProc 的代码逆向建模',
                      size=22, color='595959')], align='center', before=0, after=160))

# 一 概述
body.append(heading('一、状态机概述'))
body.append(para([run(
    '充电站主控在每个调度周期调用 CanProc()（can.c）。该函数并非线性流程，而是一个由定时器驱动的'
    '周期性控制状态机：在“总线在线监测”守卫下，不断完成“设备数据采集 → 充电使能广播 → 总功率测算 → '
    '功率分配决策 → 逐设备下发”等动作；同时监测子设备的在线/失联状态。CAN 总线离线（BUS-OFF）'
    '恢复作为并发守卫子状态机，离线期间禁止一切下发。该状态机与上述“功率动态分配算法”紧密衔接——'
    '功率分配决策与逐设备下发正是功率分配算法的载体。')]))

# 二 状态定义
body.append(heading('二、状态定义'))
states = [
    ('S0 初始化', '上电/复位后调用 CanRamInit() 清零各 BMS/AP/电量结构体与定时器，进入运行。', 'can.c CanRamInit / 启动'),
    ('S1 总线监测', 'CanRecoveryProc()：检测 BUS-OFF 与漏应答(MISS_ACK)，维护 BUSOFF_NONE/SLOW/QUICK 与 MISS_ACK_* 子状态，离线时 tx_disable=TRUE。', 'can.c:81-182'),
    ('S2 数据采集', '解析 BMS/AP 报文，刷新 g_SlotBmsInfor[i].Soc、ChargeCurrent 与 g_BmsSysAP[i] 额定值；统计在线数 bat_act_number 与最大 SOC 设备 max_soc_numb。', 'can.c:617-634'),
    ('S3 使能广播', '每 10s(t200ms≥10000) 调 vcu_canbox_mode() 向 0x1806E640 广播：PaygGetPayState() 决定 TargetChargeVoltageH(0xff=暂停)，power>1.0 置 ChargeEnable=0x55。', 'can.c:702-706 / 507-542'),
    ('S4 总功率测算', '每 5s(t500ms≥5000) 累加 Σ(ap_vol[i]×ap_cur[i])，得到 total_power_all = 总和/1e7。', 'can.c:710-723'),
    ('S5 分配决策', '读取设置功率 power，与 total_power_all 比较：≥ 则全额定；< 则按 SOC 从大到小动态分配剩余功率。', 'can.c vcu_canbox_cur:552'),
    ('S6 逐设备下发', '遍历 i=0…CHARGE_NUM-1，candi=0x1803F000+i；电压=额定、电流=分配/额定，ChargeEnable=(power>1.0 且电流>0)?0x55:0，CanTransmit 发送 8 字节。', 'can.c vcu_canbox_cur:629-644'),
    ('S7 失联处理', 'g_BmsLostTimeCount[i]≥6 的子设备判定离线，清零其 SN 与 g_SlotBmsInfor[i]，退出本轮分配。', 'can.c:1393- '),
]
body.append(ttable(
    [['状态', '说明', '代码位置']] +
    [[a, b, c] for a, b, c in states],
    widths=[1500, 6000, 1860]))

# 三 状态转移
body.append(heading('三、状态转移表'))
body.append(ttable([
    ['当前状态', '触发条件', '动作 / 事件', '下一状态'],
    ['S0 初始化', '上电 / 复位', 'CanRamInit 清结构体与定时器', 'S1 总线监测'],
    ['S1 总线监测', '每个 CanProc 周期', '检测 BUS-OFF/漏应答；离线则 tx_disable', 'S2 数据采集'],
    ['S1 总线监测', '检测到 BUS-OFF 错误', '进入 BUSOFF_SLOW/QUICK 子状态', 'S1(离线,禁止下发)'],
    ['S1 总线监测', '超时恢复且无错误', 'busoff_state=BUSOFF_NONE, tx_disable=FALSE', 'S2 数据采集'],
    ['S2 数据采集', '每周期', '刷新 SOC/电流/额定值、在线数、最大SOC', 'S3/S4/S5/S6(按定时器)'],
    ['S2 数据采集', '某设备 LostTime≥6', '清零该设备数据', 'S7 失联处理 → S2'],
    ['S3 使能广播', 't200ms≥10000(10s)', 'vcu_canbox_mode → 0x1806E640', 'S2 数据采集'],
    ['S4 总功率测算', 't500ms≥5000(5s)', '算 total_power_all', 'S5 分配决策'],
    ['S5 分配决策', 'power ≥ total_power_all', '全部 out_cur=额定', 'S6 逐设备下发'],
    ['S5 分配决策', 'power < total_power_all', '按 SOC 排序动态分配剩余功率', 'S6 逐设备下发'],
    ['S6 逐设备下发', '完成遍历发送', 'CanTransmit 各子设备', 'S2 数据采集'],
], widths=[1700, 2200, 3460, 2000]))

# 四 状态转移图
body.append(heading('四、状态转移图（主控制状态机）'))
body.append(box('S0 初始化\nCanRamInit()', fill='E2EFDA'))
body.append(arrow())
body.append(box('S1 总线监测\nCanRecoveryProc()\nBUSOFF_NONE / SLOW / QUICK\nMISS_ACK_NONE / TIMEOUT / RECOVERY'))
body.append(arrow())
body.append(box('S2 数据采集\n刷新 Soc / 电流 / 额定值\n统计在线数 & 最大SOC'))
body.append(arrow('每周期'))
body.append(box('定时器分支判断', fill='FFF2CC'))
body.append(arrow('t200ms≥10s (10s)'))
body.append(box('S3 使能广播\nvcu_canbox_mode → 0x1806E640\nPaygGetPayState 门控 / power>1.0 使能', fill='DDEBF7'))
body.append(arrow('t500ms≥5s (5s)'))
body.append(box('S4 总功率测算\ntotal_power_all = Σ(ap×cur)/1e7'))
body.append(arrow())
body.append(box('S5 分配决策\npower ≥ total_power_all ?', fill='FFF2CC'))
body.append(arrow('是（充足）'))
body.append(box('全额定下发\nout_cur[i] = ap_cur[i]', fill='E2EFDA'))
body.append(arrow('否（不足）'))
body.append(box('按 SOC 从大到小\n动态分配剩余功率\n电流 = 剩余 / 额定电压'))
body.append(arrow())
body.append(box('S6 逐设备下发\ncandi = 0x1803F000 + i\n电压=额定, 电流=分配\nChargeEnable=(power>1.0 && cur>0)?0x55:0', fill='DDEBF7'))
body.append(arrow('回到 S2'))

body.append(heading('四-B、CAN 总线离线恢复子状态机（S1 守卫）', size=26))
body.append(box('BUSOFF_NONE\n(总线正常, tx_disable=FALSE)', fill='E2EFDA'))
body.append(arrow('检测到 BUS-OFF 错误 (CAN_ERR_BOERR)'))
body.append(box('BUSOFF_QUICK\n(counter<5, 快恢复 100ms)', fill='FFF2CC'))
body.append(arrow('busoff_counter≥5'))
body.append(box('BUSOFF_SLOW\n(慢恢复 1000ms)', fill='FFF2CC'))
body.append(arrow())
body.append(box('超时后重新初始化 CAN\n清 BOERR, tx_disable=FALSE\nbusoff_state=NONE', fill='E2EFDA'))
body.append(arrow('恢复'))
body.append(box('MISS_ACK_NONE → MISS_ACK_TIMEOUT\n→ MISS_ACK_RECOVERY\n(漏应答时停发 TX_TIMEOUT, 再恢复)', fill='F2F2F2'))

# 五 关键说明
body.append(heading('五、关键说明'))
body.append(number('守卫关系：S1 总线监测是全局守卫。只有 busoff_state==BUSOFF_NONE 且 tx_disable==FALSE 时，S3/S6 的下发才真正有效；离线期间所有 CanTransmit 被抑制。'))
body.append(number('并发而非互斥：S2 每周期都执行；S3（10s）与 S4/S5/S6（5s）按各自定时器触发，互相穿插，并非严格串行排队。'))
body.append(number('支付门控：S3 使能广播受 PaygGetPayState() 约束——未付费时 TargetChargeVoltageH 置 0xFF（暂停），设备据此停止充电。'))
body.append(number('功率门控：power>1.0 才允许 ChargeEnable=0x55；分配结果为 0 电流的设备同样被置为不使能，自然退出充电。'))
body.append(number('失联自愈：S7 仅清零离线设备数据并使其退出本轮分配，不影响其他在线设备的功率分配与下发。'))
body.append(number('与功率分配算法的衔接：S5/S6 正是《充电功率动态分配算法说明》中流程图的具体实现，决策与下发逻辑完全一致。'))

# 六 与算法文档对照
body.append(heading('六、与《充电功率动态分配算法说明》的对应关系'))
body.append(ttable([
    ['算法文档步骤', '本状态机环节', '代码'],
    ['读取设置功率 / 量纲换算', 'S5 分配决策', 'vcu_canbox_cur:564-596'],
    ['SOC 排序（大者优先）', 'S2 数据采集 + S5', 'g_bat_soc / soc_order'],
    ['power≥total 全额定', 'S5→S6 充足分支', 'vcu_canbox_cur:598-601'],
    ['功率不足动态分配', 'S5→S6 不足分支', 'vcu_canbox_cur:603-627'],
    ['逐设备下发电压/电流/使能', 'S6 逐设备下发', 'vcu_canbox_cur:629-644'],
], widths=[3400, 3200, 2760]))

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
    '<dc:title>充电与功率分配控制状态机</dc:title><dc:creator>CodeBuddy</dc:creator></cp:coreProperties>'
)
app = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties">'
    '<Application>CodeBuddy</Application></Properties>'
)

path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                    '充电与功率分配控制状态机.docx')
with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', content_types)
    z.writestr('_rels/.rels', rels)
    z.writestr('word/document.xml', document)
    z.writestr('word/_rels/document.xml.rels', doc_rels)
    z.writestr('word/numbering.xml', numbering)
    z.writestr('docProps/core.xml', core)
    z.writestr('docProps/app.xml', app)

print('OK ->', path)
