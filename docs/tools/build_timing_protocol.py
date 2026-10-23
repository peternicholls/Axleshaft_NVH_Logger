#!/usr/bin/env python3
"""Render the Markdown protocol with code-native vector illustrations (ReportLab)."""
from pathlib import Path
import re
from html import escape
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, KeepTogether
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.enums import TA_LEFT
from reportlab.graphics.shapes import Drawing, Rect, Circle, Line, String, Polygon

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'docs/experiments/timing-protocol.md'
DEST = ROOT / 'output/pdf/NVH-TIME-01-experiment-protocol.pdf'
BLUE = colors.HexColor('#153E59')
TEAL = colors.HexColor('#087F8C')
PALE = colors.HexColor('#EAF3F6')
INK = colors.HexColor('#182C39')
GREY = colors.HexColor('#526674')
WIDTH = 515


def text(d,x,y,s,size=10,color=INK):
    d.add(String(x,y,s,fontName='Helvetica',fontSize=size,fillColor=color))


def box(d,x,y,w,h,label):
    d.add(Rect(x,y,w,h,rx=5,ry=5,fillColor=PALE,strokeColor=TEAL))
    for i,line in enumerate(label.split('|')):
        text(d,x+9,y+h-18-i*14,line,10)


def arrow(d,x,y,xx,yy,color=TEAL):
    d.add(Line(x,y,xx,yy,strokeColor=color,strokeWidth=1.5))
    import math
    a=math.atan2(yy-y,xx-x)
    d.add(Polygon([xx,yy,xx-7*math.cos(a-.4),yy-7*math.sin(a-.4),
                   xx-7*math.cos(a+.4),yy-7*math.sin(a+.4)],fillColor=color,strokeColor=color))


def rig():
    d=Drawing(WIDTH,245)
    text(d,0,230,'FIGURE 2  /  A mechanically coupled, stationary sensor',11,BLUE)
    box(d,5,130,85,55,'Small motor|fixed mount')
    d.add(Line(90,155,235,155,strokeWidth=6,strokeColor=GREY))
    d.add(Circle(235,155,43,fillColor=colors.HexColor('#394F5B'),strokeColor=BLUE))
    d.add(Rect(229,186,12,11,fillColor=colors.white,strokeColor=colors.white))
    text(d,155,215,'ONE white mark',10)
    arrow(d,209,208,233,196)
    box(d,330,167,175,43,'IR sensor: fixed gap|digital OUT to GPIO26')
    arrow(d,330,184,280,184)
    text(d,286,193,'view',9)
    d.add(Polygon([267,133,290,120,271,112],fillColor=TEAL,strokeColor=TEAL))
    text(d,155,84,'One cam / tab per turn',10)
    arrow(d,247,92,277,119)
    d.add(Line(288,118,340,102,strokeWidth=3,strokeColor=GREY))
    text(d,303,132,'light tap',9)
    box(d,340,55,165,55,'Stationary rigid bracket|GY-521 mounted here')
    arrow(d,339,102,351,102)
    text(d,5,40,'Adjust cam angle relative to mark; measure actual contact timing.',10)
    text(d,5,22,'Motor-on/no-tap is a separate control. Diagram not to scale.',9,GREY)
    return d


def wiring():
    d=Drawing(WIDTH,205)
    text(d,0,192,'FIGURE 3  /  Signal connections (verify actual module labels)',11,BLUE)
    box(d,190,20,130,150,'ESP32|21 SDA / 22 SCL|27 INT|26 optical OUT|32 button|13/18/19/23 SD')
    box(d,0,112,153,55,'GY-521|3V3 / GND / SDA / SCL|INT to GPIO27')
    arrow(d,153,140,190,140)
    box(d,0,32,153,55,'Optical sensor|3V3 / GND / DO|DO must be 3.3 V safe')
    arrow(d,153,58,190,58)
    box(d,355,110,160,55,'SD module|3V3 / GND, 5V open|SPI at 1 MHz')
    arrow(d,320,140,355,140)
    box(d,355,32,160,55,'Button + built-in LED|Button GPIO32 to GND|No external LED needed')
    arrow(d,320,58,355,58)
    text(d,0,4,'All grounds common. Optional instrument pins are described in section 3.',9,GREY)
    return d


def timeline():
    d=Drawing(WIDTH,212)
    text(d,0,199,'FIGURE 1  /  Different events: do not force them to line up',11,BLUE)
    labels=['Physical mark (reference)','Optical electrical OUT','Candidate ISR timestamp','Qualified event processing','Mechanical first contact','IMU onset / peak','SD write completion']
    xs=[177,198,216,265,240,292,453]
    for i,(label,x) in enumerate(zip(labels,xs)):
        y=173-i*23
        text(d,0,y,label,9)
        d.add(Line(171,y,505,y,strokeColor=colors.HexColor('#CCD8DD')))
        d.add(Line(x,y-5,x,y+6,strokeColor=TEAL,strokeWidth=2))
        if i==5:
            d.add(Line(x+35,y-5,x+35,y+6,strokeColor=BLUE,strokeWidth=2))
            text(d,x-6,y+9,'onset',8)
            text(d,x+28,y+9,'peak',8)
    arrow(d,176,10,498,10)
    text(d,179,0,'Time ->  schematic positions only; contact can precede OUT.',8,GREY)
    return d


def flow():
    d=Drawing(WIDTH,88)
    text(d,0,73,'FIGURE 4  /  Freeze the rules before collecting acceptance data',11,BLUE)
    names=['Pilot / references','Freeze targets','Capture repeats','Validate bytes','Report claim']
    for i,name in enumerate(names):
        x=i*104
        box(d,x,16,96,38,name)
        if i<4: arrow(d,x+96,35,x+103,35)
    text(d,0,0,'Missing reference or undefined tolerance -> UNVERIFIED, not assumed PASS.',9,GREY)
    return d


styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name='BodyNVH',fontName='Helvetica',fontSize=10,leading=13.5,spaceAfter=7,textColor=INK))
styles.add(ParagraphStyle(name='TableNVH',fontName='Helvetica',fontSize=8.6,leading=11.5,textColor=INK))
styles.add(ParagraphStyle(name='TitleNVH',fontName='Helvetica-Bold',fontSize=24,leading=28,spaceAfter=12,textColor=BLUE))
styles.add(ParagraphStyle(name='SectionNVH',fontName='Helvetica-Bold',fontSize=14,leading=18,spaceBefore=15,spaceAfter=8,textColor=BLUE,keepWithNext=True))


def markup(s):
    s=escape(s)
    s=re.sub(r'\[([^]]+)\]\(([^)]+)\)',lambda m: f'<link href="{m[2]}" color="#087F8C">{m[1]}</link>' if m[2].startswith('http') else m[1],s)
    return re.sub(r'`([^`]+)`',r'<font name="Courier">\1</font>',s)


def para(s,style='BodyNVH'):
    return Paragraph(markup(s),styles[style])


def build():
    DEST.parent.mkdir(parents=True,exist_ok=True)
    lines=SOURCE.read_text().splitlines(); story=[]; i=0
    while i<len(lines):
        line=lines[i].strip()
        if not line: i+=1; continue
        if line.startswith('# '): story.append(para(line[2:],'TitleNVH')); i+=1; continue
        if line.startswith('## '):
            title=line[3:]; story.append(para(title,'SectionNVH'))
            if title.startswith('2.'): story.append(timeline()); story.append(Spacer(1,8))
            if title.startswith('3.'): story.append(rig()); story.append(wiring()); story.append(Spacer(1,8))
            if title.startswith('5.'): story.append(flow()); story.append(Spacer(1,8))
            i+=1; continue
        if line.startswith('|'):
            rows=[]
            while i<len(lines) and lines[i].strip().startswith('|'):
                row=[c.strip() for c in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r':?-+:?',c) for c in row): rows.append([para(c,'TableNVH') for c in row])
                i+=1
            widths=[140,187,188] if len(rows[0])==3 else [205,310]
            table=Table(rows,colWidths=widths,repeatRows=1,hAlign='LEFT')
            table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),PALE),('VALIGN',(0,0),(-1,-1),'TOP'),('GRID',(0,0),(-1,-1),.4,colors.HexColor('#CBD7DD')),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6)]))
            story.extend([table,Spacer(1,10)]); continue
        # Markdown paragraphs are separated by blank lines; numbered/bullet steps
        # each remain independent for good page splitting and visible step numbers.
        parts=[line]; i+=1
        while i<len(lines) and lines[i].strip() and not lines[i].startswith(('#','|','- ')) and not re.match(r'^\d+\. ',lines[i]):
            parts.append(lines[i].strip()); i+=1
        story.append(para(' '.join(parts)))
    def page(c,doc):
        c.setStrokeColor(TEAL); c.line(40,39,555,39)
        c.setFont('Helvetica',8); c.setFillColor(GREY)
        c.drawString(40,26,'NVH-TIME-01  |  Rev 1  |  0.4.7  |  Procedure prepared - not executed')
        c.drawRightString(555,26,str(doc.page))
    doc=SimpleDocTemplate(str(DEST),pagesize=(595,842),leftMargin=40,rightMargin=40,topMargin=38,bottomMargin=53,title='NVH-TIME-01: Timing experiment protocol',author='Polestar NVH Logger project')
    doc.build(story,onFirstPage=page,onLaterPages=page)
    print(DEST)

if __name__=='__main__': build()
